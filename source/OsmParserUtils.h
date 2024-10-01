#pragma once

#include "LatLong.h"

#include <string>
#include <vector>

struct OsmComponent {};

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