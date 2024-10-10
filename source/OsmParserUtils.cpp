
#include "OsmParserUtils.h"

#include "FTileMapData.h"
#include "ShapeUtils.h"
#include "type_defines.h"
#include <unordered_set>

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

	bool OsmComponent::IsLandUse() const {
		return tags.find("landuse") != tags.end();
	}

	static void PopulateCoordinate(FCoordinate* coordinate, LatLong input, LatLong lowerCorner, LatLong upperCorner) {
		coordinate->globalPosition = input;
		coordinate->localPosition.X = GetRangeMappedValue(coordinate->globalPosition.longitude,
			lowerCorner.longitude,
			upperCorner.longitude);
		coordinate->localPosition.Y = GetRangeMappedValue(coordinate->globalPosition.latitude,
			upperCorner.latitude,
			lowerCorner.latitude);
	}

	FMapGeometry* OsmNode::CreateGeometry(LatLong lowerCorner, LatLong upperCorner) const {
		FCoordinate* fCoordinate = new FCoordinate();
		PopulateCoordinate(fCoordinate, coordinate, lowerCorner, upperCorner);
		return fCoordinate;
	}

	uint64_t OsmWay::GetStartNodeId() const {
		return nodes[0]->id;
	}

	uint64_t OsmWay::GetEndNodeId() const {
		return nodes[SIZE(nodes) - 1]->id;
	}

	FMapGeometry* OsmWay::CreateGeometry(LatLong lowerCorner, LatLong upperCorner) const {
		FLine* fLine = new FLine();
		for (const auto& node : this->nodes) {
			FCoordinate* fCoordinate = new FCoordinate();
			PopulateCoordinate(fCoordinate, node->coordinate, lowerCorner, upperCorner);
			ADD(fLine->coordinates, fCoordinate);
		}
		fLine->isClockwise = ShapeUtils::CalculateShapeOrientation(fLine);
		return fLine;
	}

	FMapGeometry* OsmRelation::CreateGeometry(LatLong lowerCorner, LatLong upperCorner) const {
		if (isMultigon) {
			FPolygon* polygon = new FPolygon();
			FLine* fLine = new FLine();
			uint64_t lastAddedNodeId = 0;

			for (int i = 0; i < SIZE(this->multigonCache.outerSegments); ++i) {
				auto& segment = this->multigonCache.outerSegments[i];
				if (segment.second) { // Is reversed?
					for (auto nodeIterator = segment.first->nodes.rbegin(); nodeIterator != segment.first->nodes.rend(); ++nodeIterator) {
						if ((*nodeIterator)->id != lastAddedNodeId) {
							FCoordinate* fCoordinate = new FCoordinate();
							PopulateCoordinate(fCoordinate, (*nodeIterator)->coordinate, lowerCorner, upperCorner);
							ADD(fLine->coordinates, fCoordinate);
							lastAddedNodeId = (*nodeIterator)->id;
						}
					}
				}
				else {
					for (OsmNode* node : segment.first->nodes) {
						if (node->id != lastAddedNodeId) {
							FCoordinate* fCoordinate = new FCoordinate();
							PopulateCoordinate(fCoordinate, node->coordinate, lowerCorner, upperCorner);
							ADD(fLine->coordinates, fCoordinate);
							lastAddedNodeId = node->id;
						}
					}
				}
				fLine->isClockwise = ShapeUtils::CalculateShapeOrientation(fLine);
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
		ADD(relations, std::make_pair(component, role));
	}

	void OsmRelation::PrecomputeMultigonRelations() {
		if (isMultigon) {
			// Index to map node IDs to ways for faster searching
			std::unordered_set<OsmWay*> segmentCache;

			// First, fill in the cache set of all possible outer ways
			for (auto& currentMember : relations) {
				if (currentMember.second == "outer") {
					OsmWay* outerWay = dynamic_cast<OsmWay*>(currentMember.first);
					if (outerWay != nullptr) {
						segmentCache.insert(outerWay);
					}
				}
			}
			// Begin the algorithm with an outer way
			const auto& startIterator = segmentCache.begin();
			if (startIterator == segmentCache.end()) return; // No outer segments found
			OsmWay* startWay = *startIterator;
			OsmWay* currentWay = startWay;
			bool isReversed = false;
			ADD(multigonCache.outerSegments, std::make_pair(currentWay, isReversed));
			segmentCache.erase(startIterator);
			LOG_F("[BEGIN MATCHING] START NODE: %lu END NODE: %lu", currentWay->GetStartNodeId(), currentWay->GetEndNodeId());
			
			// In my solution, we keep iterating in the cache looking for a next segment until we create a closed loop or we
			// are out of way segments.
			// We also take into account if the segment is reversed or not.
			int32_t maxLoop = 5000;
			while (maxLoop-- > 0) {
				uint64_t currentIdToMatch = isReversed ? currentWay->GetStartNodeId() : currentWay->GetEndNodeId();

				// We got back matching on the start way
				if (currentIdToMatch == startWay->GetStartNodeId()) {
					LOG("[END MATCHING] Returned to start.");
					break;
				}
				for (auto cacheIterator = segmentCache.begin(); cacheIterator != segmentCache.end(); ++cacheIterator) {
					if ((*cacheIterator)->GetStartNodeId() == currentIdToMatch) {
						isReversed = false;
						currentWay = (*cacheIterator);
						ADD(multigonCache.outerSegments, std::make_pair(currentWay, isReversed));
						segmentCache.erase(cacheIterator++);
						LOG_F("START NODE: %lu END NODE: %lu", currentWay->GetStartNodeId(), currentWay->GetEndNodeId());

						break;
					}
					else if ((*cacheIterator)->GetEndNodeId() == currentIdToMatch) {
						isReversed = true;
						currentWay = (*cacheIterator);
						ADD(multigonCache.outerSegments, std::make_pair(currentWay, isReversed));
						segmentCache.erase(cacheIterator++);
						LOG_F("START NODE: %lu END NODE: %lu REVERSED", currentWay->GetStartNodeId(), currentWay->GetEndNodeId());

						break;
					}
				}

				if (segmentCache.empty()) {
					LOG("[END MATCHING] Cache is now empty.");
					break;
				}
			}

			// TODO take care of outers still remaining in the cache set. They are likely separate islands, handle them as well.
			// TODO take care of the inners (later), should be easy, because they don't seem to be separated to segments.
		}
	}
}