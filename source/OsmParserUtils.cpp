
#include "OsmParserUtils.h"

#include "FTileMapData.h"
#include "type_defines.h"

namespace Osm {

	void OsmComponent::AddTags(tinyxml2::XMLElement* source) {
		for (tinyxml2::XMLElement* tag = source->FirstChildElement("tag"); tag != nullptr; tag = tag->NextSiblingElement("tag")) {
			const char* key = tag->Attribute("k");
			const char* value = tag->Attribute("v");

			if(key != nullptr && value != nullptr) {
				tags[key] = value;
			}
		}
	}

	bool OsmComponent::IsBuilding() const {
		return tags.find("building") != tags.end();
	}

	static void PopulateCoordinate(FCoordinate& coordinate, LatLong input, LatLong lowerCorner, LatLong upperCorner) {
		coordinate.latitude = input.latitude;
		coordinate.longitude = input.longitude;
		coordinate.localPosition.X = GetRangeMappedValue(coordinate.longitude,
			lowerCorner.longitude,
			upperCorner.longitude);
		coordinate.localPosition.Y = GetRangeMappedValue(coordinate.latitude,
			upperCorner.latitude,
			lowerCorner.latitude);
	}

	void OsmNode::AddGeometry(FFeatureGeometry& geometry, LatLong lowerCorner, LatLong upperCorner) const {
			FCoordinate fCoordinate;
			PopulateCoordinate(fCoordinate, coordinate, lowerCorner, upperCorner);
			ADD(geometry.shapes, FShape());
			FShape& currentShape = geometry.shapes[SIZE(geometry.shapes) - 1];
			ADD(currentShape.coordinates, fCoordinate);
	}

	void OsmWay::AddGeometry(FFeatureGeometry& geometry, LatLong lowerCorner, LatLong upperCorner) const {
		ADD(geometry.shapes, FShape());
		FShape& currentShape = geometry.shapes[SIZE(geometry.shapes) - 1];
		for (const auto& node : this->nodes) {
			FCoordinate fCoordinate;
			PopulateCoordinate(fCoordinate, node->coordinate, lowerCorner, upperCorner);
			ADD(currentShape.coordinates, fCoordinate);
		}
	}

	void OsmRelation::AddGeometry(FFeatureGeometry& geometry, LatLong lowerCorner, LatLong upperCorner) const {
		for (const auto& component : this->relations) {
			component.first->AddGeometry(geometry, lowerCorner, upperCorner);
		}
	}

	void OsmRelation::AddRelation(OsmComponent* component, const std::string role) {
		// Add to the front
		if(role == "outer") 
		{
			relations.insert(relations.begin(), std::make_pair(component, role));
		}
		else 
		{
			relations.push_back(std::make_pair(component, role));
		}
	}
}