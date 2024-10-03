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
			TryParseValueFor<FMapLayer>("water", parsedMapData->water, foundKey, inputData, i, ParseLayer);
			TryParseValueFor<FMapLayer>("buildings", parsedMapData->buildings, foundKey, inputData, i, ParseLayer);
		}
	}

	return true;
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

	// ID to LatLong coordinates
	typedef std::unordered_map<uint64_t, OsmNode> NodeCache;
	typedef std::unordered_map<uint64_t, OsmWay> WayCache;
	typedef std::unordered_map<uint64_t, OsmRelation> RelationCache;

	NodeCache nodes;
	WayCache ways;
	RelationCache relations;

	tinyxml2::XMLElement* root = doc.RootElement();

	// Cache all nodes
	for (tinyxml2::XMLElement* xmlNodeElement = root->FirstChildElement("node"); xmlNodeElement != nullptr; xmlNodeElement = xmlNodeElement->NextSiblingElement("node")) {
		const char* nodeId = xmlNodeElement->Attribute("id");
		const char* lat = xmlNodeElement->Attribute("lat");
		const char* lon = xmlNodeElement->Attribute("lon");
		OsmNode currentNode = OsmNode(LatLong(std::stod(lat), std::stod(lon)));
		currentNode.AddTags(xmlNodeElement);
		nodes[std::stoull(nodeId)] = currentNode;
	}

	// Cache all ways
	for (tinyxml2::XMLElement* xmlWayElement = root->FirstChildElement("way"); xmlWayElement != nullptr; xmlWayElement = xmlWayElement->NextSiblingElement("way")) {
		const char* wayId = xmlWayElement->Attribute("id");
		std::vector<OsmNode*> nodeReferences;
		// For each 'nd' (node reference) element inside this way
		for (tinyxml2::XMLElement* nd = xmlWayElement->FirstChildElement("nd"); nd != nullptr; nd = nd->NextSiblingElement("nd")) {
			const char* ref = nd->Attribute("ref");  // 'ref' is the node ID
			uint64_t nodeId = std::stoull(ref);

			// Check if the node exists in the cached nodes
			if (nodes.find(nodeId) != nodes.end()) {
				nodeReferences.push_back(&(nodes[nodeId]));  // Add the LatLong of the node to the vector
			}
		}

		OsmWay currentWay = OsmWay(nodeReferences);
		currentWay.AddTags(xmlWayElement);
		ways[std::stoull(wayId)] = currentWay;
	}

	// Cache all relations
	for (tinyxml2::XMLElement* xmlRelationElement = root->FirstChildElement("relation"); xmlRelationElement != nullptr; xmlRelationElement = xmlRelationElement->NextSiblingElement("relation")) {
		const char* relationId = xmlRelationElement->Attribute("id");
		OsmRelation currentRelation;
		// For each 'member' (node reference) element inside this way
		for (tinyxml2::XMLElement* member = xmlRelationElement->FirstChildElement("member"); member != nullptr; member = member->NextSiblingElement("member")) {
			const char* type = member->Attribute("type");
			const char* ref = member->Attribute("ref");
			const char* role = member->Attribute("role");
			uint64_t memberId = std::stoull(ref);

			// Check if the node exists in the cached nodes
			if (strcmp(type, "node") == 0 && nodes.find(memberId) != nodes.end()) {
				currentRelation.relations.push_back(std::make_pair(&nodes[memberId], role));
			}
			else if (strcmp(type, "way") == 0 && ways.find(memberId) != ways.end()) {
				currentRelation.relations.push_back(std::make_pair(&ways[memberId], role));
			}
			else if (strcmp(type, "relation") == 0 && relations.find(memberId) != relations.end()) {
				currentRelation.relations.push_back(std::make_pair(&relations[memberId], role));
			}
		}

		currentRelation.AddTags(xmlRelationElement);
		relations[std::stoull(relationId)] = currentRelation;
	}

	FMapLayer buildingLayer = FMapLayer();
	FMapLayer waterLayer = FMapLayer();

	for (const auto& osmWay : ways) {
		if (osmWay.second.IsBuilding()) {
			FFeature* building = new FFeature();
			ADD(buildingLayer.features, building);
			building->geometry = FFeatureGeometry();
			ADD(building->geometry.shapes, FShape());
			FShape& currentShape = building->geometry.shapes[SIZE(building->geometry.shapes) - 1];
			for (const auto& node : osmWay.second.nodes) {
				FCoordinate coordinate;
				coordinate.latitude = node->coordinate.latitude;
				coordinate.longitude = node->coordinate.longitude;
				coordinate.localPosition.X = GetRangeMappedValue(coordinate.longitude,
					tileCornerLow.longitude,
					tileCornerHigh.longitude);
				coordinate.localPosition.Y = GetRangeMappedValue(coordinate.latitude,
					tileCornerHigh.latitude,
					tileCornerLow.latitude);
				ADD(currentShape.coordinates, coordinate);
			}
		}
	}
	parsedMapData->buildings = buildingLayer;

	return true;
}
