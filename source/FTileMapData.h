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

struct FGeometry {
public:
	EGeometryType type;
	virtual FLine* GetMainSegment() = 0;
	virtual ARRAY<FLine*> GetHoleSegments() = 0;
	virtual int GetComponentCount() { return 1; }
	virtual ~FGeometry() = default;
};

struct FCoordinate : public FGeometry {
public:
	FCoordinate() : latitudeLongitude(0, 0) {}
	FLine* GetMainSegment() override { return nullptr; } // Invalid, a single node may not have segment
	ARRAY<FLine*> GetHoleSegments() override { return ARRAY<FLine*>() ; } // Invalid, a single node may not have holes
	LatLong latitudeLongitude;
	VECTOR2D localPosition;
};

struct FLine : public FGeometry {
public:
	ARRAY<FCoordinate> coordinates;
	bool isClosed; //for closed ways, e.g. simple buildings or areas

	FLine* GetMainSegment() override { return this; } // This shape is the outer segment itself
	ARRAY<FLine*> GetHoleSegments() override { return ARRAY<FLine*>(); } // A FLine will not have any holes
};

struct FPolygon : public FGeometry {
public:
	FLine outerShape;
	ARRAY<FLine*> innerShapes;

	FLine* GetMainSegment() override { return &outerShape; }
	ARRAY<FLine*> GetHoleSegments() override { return innerShapes; }
};

struct FCompositeGeometry : public FGeometry {
public:
	ARRAY<FGeometry*> geometries;

	FGeometry* GetActiveGeometry() {
		return geometries[CurrentIndex];
	}
	FLine* GetMainSegment() override { return GetActiveGeometry()->GetMainSegment(); }
	ARRAY<FLine*> GetHoleSegments() override { return GetActiveGeometry()->GetHoleSegments(); }
	int GetComponentCount() { return geometries.size(); }
	void SetGeometryIndex(int index) { CurrentIndex = index; }
private:
	int CurrentIndex = 0;
};

struct FProperties {
public:
	STRING kind;
	STRING kindDetail;
	STRING name;
	int area;
	int height;
};

struct FFeature {
public:
	STRING type;
	FGeometry* geometry;
	FProperties properties;
};

struct FMapLayer {
public:
	STRING type;
	ARRAY<FFeature*> features;
};

struct FTileMapData
{
public:
	FMapLayer water;
	FMapLayer buildings;
};