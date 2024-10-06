#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "LatLong.h"
#include "tinyxml2.h"
#include "type_defines.h"

#include <string>

struct FMapGeometry;

namespace Osm {

struct OsmWay;
struct MultigonCache {
	MultigonCache() {}
	ARRAY<std::pair<OsmWay*, bool>> outerSegments; // Holds the IDs of outer ways
	ARRAY<OsmWay*> innerSegments; // Holds the IDs of inner ways
};

struct OsmComponent {
	std::unordered_map<std::string, std::string> tags;
	void AddTags(tinyxml2::XMLElement* source);
	bool IsBuilding() const;
	uint64_t id = 0;
	virtual FMapGeometry* CreateGeometry(LatLong lowerCorner, LatLong upperCorner) const = 0;
};

struct OsmNode : public OsmComponent {
	OsmNode() {}
	OsmNode(const LatLong& c) { coordinate = c; }
	LatLong coordinate;
	FMapGeometry* CreateGeometry(LatLong lowerCorner, LatLong upperCorner) const override;
};

struct OsmWay : public OsmComponent {
	OsmWay() {}
	OsmWay(const ARRAY<OsmNode*>& n) { nodes = n; }
	uint64_t GetStartNodeId() const;
	uint64_t GetEndNodeId() const;
	ARRAY<OsmNode*> nodes;
	FMapGeometry* CreateGeometry(LatLong lowerCorner, LatLong upperCorner) const override;
};

struct OsmRelation : public OsmComponent {
	ARRAY<std::pair<OsmComponent*, std::string>> relations;
	FMapGeometry* CreateGeometry(LatLong lowerCorner, LatLong upperCorner) const override;
	void AddRelation(OsmComponent* component, const std::string role);
	void PrecomputeMultigonRelations();
	bool isMultigon = false;
	MultigonCache multigonCache;
};

typedef std::unordered_map<uint64_t, OsmComponent*> OsmItemsCache;

struct OsmCache {
	OsmItemsCache nodes;
	OsmItemsCache ways;
	OsmItemsCache relations;

	OsmItemsCache::iterator current;

	OsmItemsCache::iterator First();
	OsmItemsCache::iterator Last();
	OsmItemsCache::iterator GetNext();
};
}