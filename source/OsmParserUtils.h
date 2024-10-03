#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "LatLong.h"
#include "tinyxml2.h"

namespace Osm {

struct OsmComponent {
	std::unordered_map<std::string, std::string> tags;
	void AddTags(tinyxml2::XMLElement* source);
	bool IsBuilding() const;
};

struct OsmNode : public OsmComponent {
	OsmNode() {}
	OsmNode(const LatLong& c) { coordinate = c; }
	LatLong coordinate;
};

struct OsmWay : public OsmComponent {
	OsmWay() {}
	OsmWay(const std::vector<OsmNode*>& n) { nodes = n; }
	std::vector<OsmNode*> nodes;
};

struct OsmRelation : public OsmComponent {
	std::vector<std::pair<OsmComponent*, std::string>> relations;
};

}