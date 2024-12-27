// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LatLong.h"
#include "type_defines.h"

struct TileData {
public:
	TileData(LatLong lower, LatLong upper, const STRING& mapDataJson) : lowerCorner(lower), upperCorner(upper), mapDataJson(mapDataJson) {}
	LatLong lowerCorner;
	LatLong upperCorner;
	const STRING& mapDataJson;
};

/*enum EGeometryType {
	Node,
	Line,
	Polygon,
	Composite
};*/

struct FLine;

struct FMapGeometry {
public:
	//EGeometryType type;
	virtual FLine* GetMainSegment() = 0;
	virtual ARRAY<FLine*> GetHoleSegments() = 0;
	virtual int GetComponentCount() { return 1; }
	virtual ~FMapGeometry() = default;
};

struct FCoordinate : public FMapGeometry {
public:
	FCoordinate() : globalPosition(0, 0) {}
	FLine* GetMainSegment() override { return nullptr; } // Invalid, a single node may not have segment
	ARRAY<FLine*> GetHoleSegments() override { return ARRAY<FLine*>(); } // Invalid, a single node may not have holes
	LatLong globalPosition;
	VECTOR2D localPosition;
};

struct FLine : public FMapGeometry {
public:
	ARRAY<FCoordinate*> coordinates;
	bool isClosed; //for closed ways, e.g. simple buildings or areas
	bool isClockwise;
	FCoordinate* GetCoordinate(int i) { return isClockwise ? coordinates[i] : coordinates[SIZE(coordinates) - 1 - i]; }
	FLine* GetMainSegment() override { return this; } // This shape is the outer segment itself
	ARRAY<FLine*> GetHoleSegments() override { return ARRAY<FLine*>(); } // A FLine will not have any holes
};

struct FPolygon : public FMapGeometry {
public:
	FLine* outerShape;
	ARRAY<FLine*> innerShapes;

	FLine* GetMainSegment() override { return outerShape; }
	ARRAY<FLine*> GetHoleSegments() override { return innerShapes; }
};

struct FCompositeGeometry : public FMapGeometry {
public:
	ARRAY<FMapGeometry*> geometries;

	FMapGeometry* GetActiveGeometry() {
		return geometries[CurrentIndex];
	}
	FLine* GetMainSegment() override { return GetActiveGeometry()->GetMainSegment(); }
	ARRAY<FLine*> GetHoleSegments() override { return GetActiveGeometry()->GetHoleSegments(); }
	int GetComponentCount() { return SIZE(geometries); }
	void SetGeometryIndex(int index) { CurrentIndex = index; }
private:
	int CurrentIndex = 0;
};

struct FMapElement {
public:
	STRING name;
	int64_t id;
	int area;
	FMapGeometry* geometry;
};

enum class PathType {
	Unknown,           // For any unrecognized path types
	Motorway,          // highway=motorway
	Trunk,             // highway=trunk
	Primary,           // highway=primary
	Secondary,         // highway=secondary
	Tertiary,          // highway=tertiary
	Unclassified,      // highway=unclassified
	Residential,       // highway=residential
	Service,           // highway=service
	LivingStreet,      // highway=living_street
	Pedestrian,        // highway=pedestrian
	Footway,           // highway=footway
	Cycleway,          // highway=cycleway
	Path,              // highway=path
	Track,             // highway=track (usually off-road tracks)
	Bridleway,         // highway=bridleway (for horse riding)
	Steps,             // highway=steps (for staircases)
	Road,              // highway=road (generic road with unspecified type)
};

enum class PathSurfaceMaterial {
	Unknown,           // For any unrecognized surface materials
	Asphalt,           // surface=asphalt
	Concrete,          // surface=concrete
	Paved,             // surface=paved
	Unpaved,           // surface=unpaved
	Gravel,            // surface=gravel
	Dirt,              // surface=dirt
	Sand,              // surface=sand
	Grass,             // surface=grass
	Mud,               // surface=mud
	Cobblestone,       // surface=cobblestone
	Pebblestone,       // surface=pebblestone
	Sett,              // surface=sett (a form of paving stone)
	Wood,              // surface=wood (used for wooden walkways or bridges)
	Metal,             // surface=metal (metal surfaces, e.g. bridges)
	Snow,              // surface=snow
	Ice,               // surface=ice
	Compacted,         // surface=compacted (compacted gravel or soil)
	FineGravel,        // surface=fine_gravel
	Ground             // surface=ground (natural ground surface)
};

struct FPathData : public FMapElement {
	PathType pathType;
	PathSurfaceMaterial surfaceMaterial;
	int32_t laneCount;
	int32_t width;
	bool isOneWay = false;
};

enum class LanduseKind {
	Unknown,         // For unrecognized or unspecified land use types
	Residential,     // landuse=residential
	Commercial,      // landuse=commercial
	Industrial,      // landuse=industrial
	Military,        // landuse=military
	Retail,          // landuse=retail
	Farmland,        // landuse=farmland
	Farmyard,        // landuse=farmyard
	Forest,          // landuse=forest
	Meadow,          // landuse=meadow
	Grass,           // landuse=grass
	Orchard,         // landuse=orchard
	Vineyard,        // landuse=vineyard
	Quarry,          // landuse=quarry
	Cemetery,        // landuse=cemetery
	Allotments,      // landuse=allotments (community gardens)
	RecreationGround,// landuse=recreation_ground
	VillageGreen,    // landuse=village_green (open green spaces in villages)
	Reservoir,       // landuse=reservoir
	Basin,           // landuse=basin (for water retention)
	Landfill,        // landuse=landfill (waste disposal)
	Brownfield,      // landuse=brownfield (previously developed land)
	Greenfield,      // landuse=greenfield (undeveloped land)
	Religious,       // landuse=religious (land designated for religious purposes)
	Railway,         // landuse=railway
	Port,            // landuse=port
	Construction,    // landuse=construction (areas under construction)
	Garages,         // landuse=garages (areas for garages)
	Parking,         // landuse=parking (parking lots or spaces)
	Conservation,    // landuse=conservation (protected natural areas)
	NatureReserve    // landuse=nature_reserve
};

struct FLanduseData : public FMapElement {
	LanduseKind kind;
};

enum class BuildingKind {
	Unknown,
	House,
	Apartments,
	Commercial,
	Industrial,
	Retail,
	Residential,
	Church,
	Cathedral,
	School,
	Hospital,
	Warehouse,
	Garage,
	Shed,
	Hut,
	Cabin,
	Barn,
	Detached,
	Public,
	Kiosk,
	Office,
	Bunker,
	Hotel,
	Dormitory,
	Stable,
	Roof,
	TrainStation,
	Service,
	Terrace,
	Supermarket,
	University,
	GarageDetached,
	Construction,
	Ruins,
	Mosque,
	Temple,
	Civic,
	SportsHall,
	Hangar,
	StaticCaravan,
	Greenhouse
};

enum class RoofShape {
	Unknown,
	Flat,
	Gabled,
	Hipped,
	Pitched,
	Gambrel,
	Mansard,
	HalfHipped,
	Round,
	Saltbox,
	Skillion,
	Dome,
	Pyramidal,
	Onion,
	Bonnet,
	Sawtooth,
	Tent,
	Butterfly,
	SideHipped,
	Barrel,
	Conical,
	Hexagonal,
	CrossGabled
};

enum class MaterialProperty {
	Unknown,
	Wood,
	Concrete,
	Metal,
	Steel,
	Stone,
	ReinforcedConcrete,
	Plastic,
	Brick,
	Granite,
	Brass,
	Glass,
	Sandstone,
	Rock,
	Aluminium,
	Copper,
	Soil,
	Marble,
	Limestone,
	Tufa,
	DryStone,
	Andesite,
	Adobe,
	Iron,
	CastIron,
	Sand,
	Plaster,
	Slate,
	WeatheringSteel
};

enum class ColorProperty {
	Unknown,
	Black,
	Gray,
	Maroon,
	Olive,
	Green,
	Teal,
	Navy,
	Purple,
	White,
	Silver,
	Red,
	Yellow,
	Lime,
	Aqua,
	Blue,
	Fuchsia,
	Brown,
	Orange,
	Custom
};

struct FBuildingData : public FMapElement {
	BuildingKind kind;
	RoofShape roofShape;
	MaterialProperty material;
	ColorProperty buildingColor;
	ColorProperty roofColor;
	FLanduseData* belongingLanduse;
	int minHeight;
	bool isHeightKnown;
	int height;
	int levels;
	int roofHeight;
};

struct FTileMapData
{
public:
	ARRAY<FPathData*> paths;
	ARRAY<FBuildingData*> buildings;
	ARRAY<FLanduseData*> landuse;
	ARRAY<FMapElement*> water;
};
