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

enum EGeometryType {
	Node,
	Line,
	Polygon,
	Composite
};

struct FLine;

struct FMapGeometry {
public:
	EGeometryType type;
	virtual FLine* GetMainSegment() = 0;
	virtual ARRAY<FLine*> GetHoleSegments() = 0;
	virtual int GetComponentCount() { return 1; }
	virtual ~FMapGeometry() = default;
};

struct FCoordinate : public FMapGeometry {
public:
	FCoordinate() : globalPosition(0, 0) {}
	FLine* GetMainSegment() override { return nullptr; } // Invalid, a single node may not have segment
	ARRAY<FLine*> GetHoleSegments() override { return ARRAY<FLine*>() ; } // Invalid, a single node may not have holes
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

enum class LanduseKind { Unknown, Residental, Commercial, Industrial, Military, Retail };

struct FLanduseData : public FMapElement {
	LanduseKind kind;
};

enum class BuildingKind { House, Residental, Retail, Church, Goverment, Museum  };
enum RoofType { Flat, Hipped, Pyramidal, Dome };

struct FBuildingData : public FMapElement {
	BuildingKind kind;
	RoofType roofType;
	FLanduseData* belongingLanduse;
	int minHeight;
	int height;
	int levels;
	int roofHeight;
};

struct FTileMapData
{
public:
	FMapElement* water;
	ARRAY<FBuildingData*> buildings;
	ARRAY<FLanduseData*> landuse;
};