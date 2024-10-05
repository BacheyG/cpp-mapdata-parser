
#include "OsmParserUtils.h"

#include "FTileMapData.h"
#include "ShapeUtils.h"
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
		if (!ShapeUtils::CalculateShapeOrientation(fLine)) {
			REVERSE(fLine->coordinates);
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
				}
				else {
					for (OsmNode* node : segment.first->nodes) {
						FCoordinate fCoordinate;
						PopulateCoordinate(fCoordinate, node->coordinate, lowerCorner, upperCorner);
						ADD(fLine->coordinates, fCoordinate);
					}
				}
				if (!ShapeUtils::CalculateShapeOrientation(fLine)) {
					REVERSE(fLine->coordinates);
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

	struct DirectionCache {
		std::vector<OsmWay*> next;
		uint64_t currentId = 0;

		OsmWay* Get() {
			if (currentId < next.size()) {
				return next[currentId];
			}
			return nullptr;
		}
	};

	void OsmRelation::PrecomputeMultigonRelations() {
		if (isMultigon) {
			// Index to map node IDs to ways for faster searching
			std::unordered_map<uint64_t, DirectionCache> outerStartNodeMap;
			std::unordered_map<uint64_t, DirectionCache> outerEndNodeMap;

			// First, fill in the maps with outer ways
			for (auto& currentMember : relations) {
				if (currentMember.second == "outer") {
					OsmWay* outerWay = dynamic_cast<OsmWay*>(currentMember.first);
					if (outerWay != nullptr) {
						auto sameStartExists = outerStartNodeMap.find(outerWay->GetStartNodeId());
						auto sameEndExists = outerEndNodeMap.find(outerWay->GetEndNodeId());

						if (sameStartExists == outerStartNodeMap.end()) {
							outerStartNodeMap[outerWay->GetStartNodeId()] = DirectionCache();
						}
						if (sameEndExists == outerEndNodeMap.end()) {
							outerEndNodeMap[outerWay->GetEndNodeId()] = DirectionCache();
						}
						outerStartNodeMap[outerWay->GetStartNodeId()].next.push_back(outerWay);
						outerEndNodeMap[outerWay->GetEndNodeId()].next.push_back(outerWay);
					}
				}
			}

			// Use an iterator to find the first outer way
			auto startIt = outerStartNodeMap.begin();
			if (startIt == outerStartNodeMap.end()) return; // No outer segments found

			OsmWay* start = startIt->second.Get();
			startIt->second.currentId++;

			auto asd = outerEndNodeMap.find(start->GetEndNodeId());
			asd->second.currentId++;
			OsmWay* current = start;
			bool isCurrentReversed = false; // Assume first item is not reversed
			while (true) {
				multigonCache.outerSegments.push_back(std::make_pair(current, isCurrentReversed));
				uint64_t nextStartNodeId = isCurrentReversed ? current->GetStartNodeId() : current->GetEndNodeId();
				auto newStart = outerStartNodeMap.find(nextStartNodeId);
				if (newStart == outerStartNodeMap.end()) {
					newStart = outerEndNodeMap.find(nextStartNodeId);
					if (newStart == outerEndNodeMap.end()) {
						break; // We finished early?
					}
					isCurrentReversed = true;
				}
				else {
					isCurrentReversed = false;
				}
				current = newStart->second.Get();
				newStart->second.currentId++;
				if (current == nullptr || current == start) {
					break; // Completed a full loop
				}
				uint64_t nextEndNodeId = isCurrentReversed ? current->GetStartNodeId() : current->GetEndNodeId();
				auto newEnd = isCurrentReversed ? outerStartNodeMap.find(nextEndNodeId) : outerEndNodeMap.find(nextEndNodeId);
				newEnd->second.currentId++;
				UE_LOG(LogTemp, Log, TEXT("START NODE: %lu END NODE: %lu"), nextStartNodeId, nextEndNodeId);
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