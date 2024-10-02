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

bool MapDataUtils::ProcessMapDataFromGeoJson(const STRING& mapDataJson, FTileMapData* parsedMapData, int32_t tileX, int32_t tileY)
{
	if (EMPTY(mapDataJson)) {
		return false;
	}
	LatLong tileCornerLow = TileUtils::TileToLatLong(tileX, tileY, 16);
	LatLong tileCornerHigh = TileUtils::TileToLatLong(tileX + 1, tileY + 1, 16);

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

bool MapDataUtils::ProcessMapDataFromOsm(const STRING& mapDataOsm, FTileMapData* parsedMapData, int32_t tileX, int32_t tileY) 
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.Parse(CSTRINGOF(mapDataOsm));

	if (error != tinyxml2::XML_SUCCESS) {
		return false;
	}
	// ID to LatLong coordinates
	typedef std::unordered_map<uint64_t, OsmNode> NodeCache;
	typedef std::unordered_map<uint64_t, OsmWay> WayCache;
	typedef std::unordered_map<uint64_t, OsmRelation> RelationCache;

	NodeCache nodes;
	WayCache ways;
	RelationCache relations;

	tinyxml2::XMLElement* root = doc.RootElement();

	// Cache all nodes
	for (tinyxml2::XMLElement* node = root->FirstChildElement("node"); node != nullptr; node = node->NextSiblingElement("node")) {
		const char* nodeId = node->Attribute("id");
		const char* lat = node->Attribute("lat");
		const char* lon = node->Attribute("lon");
		nodes[std::stoull(nodeId)] = OsmNode(LatLong(std::stod(lat), std::stod(lon)));
	}

	// Cache all ways
	for (tinyxml2::XMLElement* way = root->FirstChildElement("way"); way != nullptr; way = way->NextSiblingElement("way")) {
		const char* wayId = way->Attribute("id");
		std::vector<OsmNode*> nodeReferences;
		// For each 'nd' (node reference) element inside this way
		for (tinyxml2::XMLElement* nd = way->FirstChildElement("nd"); nd != nullptr; nd = nd->NextSiblingElement("nd")) {
			const char* ref = nd->Attribute("ref");  // 'ref' is the node ID
			uint64_t nodeId = std::stoull(ref);

			// Check if the node exists in the cached nodes
			if (nodes.find(nodeId) != nodes.end()) {
				nodeReferences.push_back(&(nodes[nodeId]));  // Add the LatLong of the node to the vector
			}
		}

		ways[std::stoull(wayId)] = OsmWay(nodeReferences);
	}

	// Cache all relations
	for (tinyxml2::XMLElement* xmlRelationNode = root->FirstChildElement("relation"); xmlRelationNode != nullptr; xmlRelationNode = xmlRelationNode->NextSiblingElement("relation")) {
		const char* relationId = xmlRelationNode->Attribute("id");
		OsmRelation currentRelation;
		// For each 'member' (node reference) element inside this way
		for (tinyxml2::XMLElement* member = xmlRelationNode->FirstChildElement("member"); member != nullptr; member = member->NextSiblingElement("member")) {
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

		relations[std::stoull(relationId)] = currentRelation;
	}


	return true;
}
