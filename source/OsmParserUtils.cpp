
#include "OsmParserUtils.h"

#include "FTileMapData.h"
#include "type_defines.h"

namespace Osm {

	void OsmComponent::AddTags(tinyxml2::XMLElement* source) {
		for (tinyxml2::XMLElement* tag = source->FirstChildElement("tag"); tag != nullptr; tag = tag->NextSiblingElement("tag")) {
			const char* key = tag->Attribute("k");
			const char* value = tag->Attribute("v");

			if (key != nullptr && value != nullptr) {
				tags[key] = value;
			}
		}
	}

	bool OsmComponent::IsBuilding() const {
		return tags.find("building") != tags.end() || tags.find("building:part") != tags.end();
	}

	static void PopulateCoordinate(FCoordinate& coordinate, LatLong input, LatLong lowerCorner, LatLong upperCorner) {
		coordinate.latitudeLongitude = input;
		coordinate.localPosition.X = GetRangeMappedValue(coordinate.latitudeLongitude.longitude,
			lowerCorner.longitude,
			upperCorner.longitude);
		coordinate.localPosition.Y = GetRangeMappedValue(coordinate.latitudeLongitude.latitude,
			upperCorner.latitude,
			lowerCorner.latitude);
	}

	FMapGeometry* OsmNode::CreateGeometry(LatLong lowerCorner, LatLong upperCorner) const {
		FCoordinate* fCoordinate = new FCoordinate();
		PopulateCoordinate(*fCoordinate, coordinate, lowerCorner, upperCorner);
		return fCoordinate;
	}

	uint64_t OsmWay::GetStartNodeId() const {
		return nodes.front()->id;
	}

	uint64_t OsmWay::GetEndNodeId() const {
		return nodes.back()->id;
	}

	FMapGeometry* OsmWay::CreateGeometry(LatLong lowerCorner, LatLong upperCorner) const {
		FLine* fLine = new FLine();
		for (const auto& node : this->nodes) {
			FCoordinate fCoordinate;
			PopulateCoordinate(fCoordinate, node->coordinate, lowerCorner, upperCorner);
			ADD(fLine->coordinates, fCoordinate);
		}
		return fLine;
	}

	FMapGeometry* OsmRelation::CreateGeometry(LatLong lowerCorner, LatLong upperCorner) const {
		if (isMultigon) {
			FPolygon* polygon = new FPolygon();
			FLine* fLine = new FLine();
			for (const auto& segment : this->multigonCache.outerSegments) {
				if (segment.second) { // Is reversed?
					for (auto nodeIterator = segment.first->nodes.rbegin(); nodeIterator != segment.first->nodes.rend(); ++nodeIterator) {
						FCoordinate fCoordinate;
						PopulateCoordinate(fCoordinate, (*nodeIterator)->coordinate, lowerCorner, upperCorner);
						ADD(fLine->coordinates, fCoordinate);
					}
				} else {
					for (OsmNode* node : segment.first->nodes) {
						FCoordinate fCoordinate;
						PopulateCoordinate(fCoordinate, node->coordinate, lowerCorner, upperCorner);
						ADD(fLine->coordinates, fCoordinate);
					}
				}
			}
			polygon->outerShape = fLine;
			return polygon;
		}
		else {
			FCompositeGeometry* compositeGeometry = new FCompositeGeometry();
			for (const auto& component : this->relations) {
				FMapGeometry* child = component.first->CreateGeometry(lowerCorner, upperCorner);
				ADD(compositeGeometry->geometries, child);
			}
			return compositeGeometry;
		}
	}

	void OsmRelation::AddRelation(OsmComponent* component, const std::string role) {
		// Add to the front
		if (role == "outer" || role == "inner") {
			isMultigon = true;
		}
		relations.push_back(std::make_pair(component, role));
	}

	void OsmRelation::PrecomputeMultigonRelations() {
		if (isMultigon) {
			// Index to map node IDs to ways for faster searching
			std::unordered_map<int, OsmWay*> outerStartNodeMap;

			// First, fill in the maps with outer ways
			for (auto& currentMember : relations) {
				if (currentMember.second == "outer") {
					OsmWay* outerWay = dynamic_cast<OsmWay*>(currentMember.first);
					if (outerWay != nullptr) {
						outerStartNodeMap[outerWay->GetStartNodeId()] = outerWay;
					}
				}
			}

			// Use an iterator to find the first outer way
			auto startIt = outerStartNodeMap.begin();
			if (startIt == outerStartNodeMap.end()) return; // No outer segments found

			OsmWay* start = startIt->second;
			int startNodeId = start->GetStartNodeId(); // Store start node ID for comparison
			OsmWay* current = start;
			bool isReversed = false;
			while (true) {
				multigonCache.outerSegments.push_back(std::make_pair(current, isReversed));
				auto nextItem = outerStartNodeMap.find(current->GetEndNodeId());
				if (nextItem == outerStartNodeMap.end()) {
					break; // We finished early?
				}
				current = nextItem->second;
				if (current == start) {
					break; // We went around, good
				}
			}
			// TODO take care of excluded outers
			// TODO take care of the inners (later)
		}
	}

	OsmItemsCache::iterator OsmCache::First() {
		current = relations.begin();
		return current;
	}

	OsmItemsCache::iterator OsmCache::Last() {
		return nodes.end();
	}

	OsmItemsCache::iterator OsmCache::GetNext() {
		++current;
		if (current == relations.end()) {
			current = ways.begin();
		}
		if (current == ways.end()) {
			current = nodes.begin();
		}
		return current;
	}
}