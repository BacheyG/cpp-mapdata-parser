// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "type_defines.h"

struct FCoordinate {
public:
	FCoordinate() : latitude(0), longitude(0) {}
	double latitude;
	double longitude;
	VECTOR2D localPosition;
};

struct FShape {
public:
	ARRAY<FCoordinate> coordinates;
	bool orientation; // true: clockwise, false: counterclockwise
};

struct FFeatureGeometry {
public:
	STRING type;

	ARRAY<FShape> shapes;
};

struct FProperties {
public:
	STRING kind;
	STRING kindDetail;
	STRING name;
	int area;
};

struct FFeature {
public:
	STRING type;
	FFeatureGeometry geometry;
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