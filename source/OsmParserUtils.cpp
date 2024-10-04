
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

	FGeometry* OsmNode::CreateGeometry(LatLong lowerCorner, LatLong upperCorner) const {
		FCoordinate* fCoordinate = new FCoordinate();
		PopulateCoordinate(*fCoordinate, coordinate, lowerCorner, upperCorner);
		return fCoordinate;
	}

	FGeometry* OsmWay::CreateGeometry(LatLong lowerCorner, LatLong upperCorner) const {
		FLine* fLine = new FLine();
		for (const auto& node : this->nodes) {
			FCoordinate fCoordinate;
			PopulateCoordinate(fCoordinate, node->coordinate, lowerCorner, upperCorner);
			ADD(fLine->coordinates, fCoordinate);
		}
		return fLine;
	}

	FGeometry* OsmRelation::CreateGeometry(LatLong lowerCorner, LatLong upperCorner) const {
		FCompositeGeometry* compositeGeometry = new FCompositeGeometry();
		for (const auto& component : this->relations) {
			FGeometry* child = component.first->CreateGeometry(lowerCorner, upperCorner);
			ADD(compositeGeometry->geometries, child);
		}
		return compositeGeometry;
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