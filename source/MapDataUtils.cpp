// Fill out your copyright notice in the Description page of Project Settings.

#include "MapDataUtils.h"
#include "JsonParserUtils.h"
#include "OsmParserUtils.h"
#include "TileUtils.h"
#include "tinyxml2.h"

#include <algorithm>
#include <ctype.h>
#include <type_traits>
#include <unordered_map>
#include <string>

bool MapDataUtils::ProcessMapDataFromGeoJson(const STRING& mapDataJson, FTileMapData* parsedMapData, int32_t tileX, int32_t tileY, int32_t zoom)
{
	if (EMPTY(mapDataJson)) {
		return false;
	}
	LatLong tileCornerLow = TileUtils::TileToLatLong(tileX, tileY, zoom);
	LatLong tileCornerHigh = TileUtils::TileToLatLong(tileX + 1, tileY + 1, zoom);

	TileData inputData(tileCornerLow, tileCornerHigh, mapDataJson);
	JsonParserState parser;
	for (int32_t i = 0; i < LENGTH(mapDataJson); ++i) {
		parser.UpdateState(mapDataJson[i]);
		if (parser.GetBracketDepth() == 1 && mapDataJson[i] == '"') {
			STRING foundKey;
			ParseString(inputData, i, foundKey);
			parser.FlipIsInString();
			// TODO: Parse JSON items.
		}
	}

	return true;
}

static void ParseOneItem(FTileMapData* parsedMapData, Osm::OsmComponent* component, LatLong tileCornerLow, LatLong tileCornerHigh) 
{
	static const int kDefaultLevels = 1;
	static const int kDefaultHeightPerLevel = 30;

	if (component->IsLandUse()) {
		FLanduseData* fLanduse = new FLanduseData();
		ADD(parsedMapData->landuse, fLanduse);
		LanduseKind landuseKind = LanduseKind::Unknown;
		const char* landuseStr = component->tags.find("landuse") != component->tags.end() ?
								 component->tags.find("landuse")->second.c_str() : "";
		if (strcmp(landuseStr, "residential") == 0)
		{
			landuseKind = LanduseKind::Residental;
		}
		if (strcmp(landuseStr, "commercial") == 0)
		{
			landuseKind = LanduseKind::Commercial;
		}
		if (strcmp(landuseStr, "industrial") == 0)
		{
			landuseKind = LanduseKind::Industrial;
		}
		if (strcmp(landuseStr, "retail") == 0)
		{
			landuseKind = LanduseKind::Retail;
		}
		if (strcmp(landuseStr, "military") == 0)
		{
			landuseKind = LanduseKind::Military;
		}
		fLanduse->kind = landuseKind;
		fLanduse->geometry = component->CreateGeometry(tileCornerLow, tileCornerHigh);
	}
	if (component->IsBuilding()) 
	{
		FBuildingData* fBuilding = new FBuildingData();
		ADD(parsedMapData->buildings, fBuilding);
		fBuilding->id = component->id;
		auto minHeightTag = component->tags.find("min-height");
		auto heightTag = component->tags.find("height");
		auto levelTag = component->tags.find("levels");
		uint32_t levelValue = (minHeightTag != component->tags.end()) ? std::stoi(heightTag->second) : 0;
		float heightValue = (heightTag != component->tags.end()) ? std::stoi(heightTag->second) : 0;
		uint32_t minHeightValue = (minHeightTag != component->tags.end()) ? std::stoi(minHeightTag->second) : 0;
		if (levelValue == 0 && heightValue == 0)
		{
			levelValue = kDefaultLevels;
			heightValue = kDefaultHeightPerLevel * levelValue;
		} 
		else if (levelValue == 0) 
		{
			levelValue = heightValue / kDefaultHeightPerLevel;
		}
		else if (heightValue == 0) 
		{
			heightValue = levelValue * kDefaultHeightPerLevel;
		}
		fBuilding->height = heightValue;
		fBuilding->levels = levelValue;
		fBuilding->geometry = component->CreateGeometry(tileCornerLow, tileCornerHigh);
	}
}

bool MapDataUtils::ProcessMapDataFromOsm(const STRING& mapDataOsm, FTileMapData* parsedMapData, int32_t tileX, int32_t tileY, int32_t zoom)
{
	using namespace Osm;
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.Parse(CSTRINGOF(mapDataOsm));

	if (error != tinyxml2::XML_SUCCESS) {
		return false;
	}

	if (parsedMapData == nullptr) {
		return false;
	}

	LatLong tileCornerLow = TileUtils::TileToLatLong(tileX, tileY, zoom);
	LatLong tileCornerHigh = TileUtils::TileToLatLong(tileX + 1, tileY + 1, zoom);


	OsmCache osmCache;

	tinyxml2::XMLElement* root = doc.RootElement();

	// Cache all nodes
	for (tinyxml2::XMLElement* xmlNodeElement = root->FirstChildElement("node"); xmlNodeElement != nullptr; xmlNodeElement = xmlNodeElement->NextSiblingElement("node")) {
		const char* nodeIdStr = xmlNodeElement->Attribute("id");
		const char* lat = xmlNodeElement->Attribute("lat");
		const char* lon = xmlNodeElement->Attribute("lon");
		uint64_t nodeId = std::stoull(nodeIdStr);
		OsmNode* currentNode = new OsmNode(LatLong(std::stod(lat), std::stod(lon)));
		currentNode->AddTags(xmlNodeElement);
		currentNode->id = nodeId;
		osmCache.nodes[nodeId] = currentNode;
	}

	// Cache all ways
	for (tinyxml2::XMLElement* xmlWayElement = root->FirstChildElement("way"); xmlWayElement != nullptr; xmlWayElement = xmlWayElement->NextSiblingElement("way")) {
		const char* wayIdString = xmlWayElement->Attribute("id");
		uint64_t wayId = std::stoull(wayIdString);
		ARRAY<OsmNode*> nodeReferences;
		// For each 'nd' (node reference) element inside this way
		for (tinyxml2::XMLElement* nd = xmlWayElement->FirstChildElement("nd"); nd != nullptr; nd = nd->NextSiblingElement("nd")) {
			const char* ref = nd->Attribute("ref");  // 'ref' is the node ID
			uint64_t nodeId = std::stoull(ref);

			// Check if the node exists in the cached nodes
			if (osmCache.nodes.find(nodeId) != osmCache.nodes.end()) {
				ADD(nodeReferences, dynamic_cast<OsmNode*>(osmCache.nodes[nodeId]));  // Add the LatLong of the node to the vector
			}
		}

		OsmWay* currentWay = new OsmWay(nodeReferences);
		currentWay->AddTags(xmlWayElement);
		currentWay->id = wayId;
		osmCache.ways[wayId] = currentWay;
	}

	// Cache all relations
	for (tinyxml2::XMLElement* xmlRelationElement = root->FirstChildElement("relation"); xmlRelationElement != nullptr; xmlRelationElement = xmlRelationElement->NextSiblingElement("relation")) {
		const char* relationIdString = xmlRelationElement->Attribute("id");
		uint64_t relationId = std::stoull(relationIdString);
		OsmRelation* currentRelation = new OsmRelation;
		currentRelation->id = relationId;
		// For each 'member' (node reference) element inside this way
		for (tinyxml2::XMLElement* member = xmlRelationElement->FirstChildElement("member"); member != nullptr; member = member->NextSiblingElement("member")) {
			const char* type = member->Attribute("type");
			const char* ref = member->Attribute("ref");
			const char* role = member->Attribute("role");
			uint64_t memberId = std::stoull(ref);

			// Check if the node exists in the cached nodes
			if (strcmp(type, "node") == 0 && osmCache.nodes.find(memberId) != osmCache.nodes.end()) {
				currentRelation->AddRelation(osmCache.nodes[memberId], role);
			}
			else if (strcmp(type, "way") == 0 && osmCache.ways.find(memberId) != osmCache.ways.end()) {
				currentRelation->AddRelation(osmCache.ways[memberId], role);
			}
			else if (strcmp(type, "relation") == 0 && osmCache.relations.find(memberId) != osmCache.relations.end()) {
				currentRelation->AddRelation(osmCache.relations[memberId], role);
			}
		}
		currentRelation->AddTags(xmlRelationElement);
		currentRelation->PrecomputeMultigonRelations();
		osmCache.relations[relationId] = currentRelation;
	}

	//parsedMapData->buildings = FMapLayer();

	// Parse everything
	for (const auto& osmItem : osmCache.relations) 
	{
		ParseOneItem(parsedMapData, osmItem.second, tileCornerLow, tileCornerHigh);
	}

	for (const auto& osmItem : osmCache.ways)
	{
		ParseOneItem(parsedMapData, osmItem.second, tileCornerLow, tileCornerHigh);
	}

	for (const auto& osmItem : osmCache.nodes)
	{
		ParseOneItem(parsedMapData, osmItem.second, tileCornerLow, tileCornerHigh);
	}

	for (FBuildingData* building : parsedMapData->buildings) {
		for (FLanduseData* landuse : parsedMapData->landuse) {
			if (ShapeUtils::IsPointInShape(landuse->geometry->GetMainSegment(), building->geometry->GetMainSegment()->coordinates[0]->localPosition))
			{
				building->belongingLanduse = landuse;
				break;
			}
		}
	}

	return true;
}
