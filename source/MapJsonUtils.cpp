// Fill out your copyright notice in the Description page of Project Settings.

#include "MapJsonUtils.h"
#include "FTileJsonData.h"
#include "TileUtils.h"

#include <algorithm>
#include <ctype.h>
#include <type_traits>

struct JsonParserState {
public:
	JsonParserState() {
		_arrayDepth = 0;
		_bracketDepth = 0;
		_isInString = false;
	}

	bool IsInValidState() {
		return _arrayDepth == 0 && _bracketDepth == 0 && !_isInString;
	}

	void FlipIsInString() {
		_isInString = !_isInString;
	}

	void UndoBracketIncrement() {
		--_bracketDepth;
	}

	int GetArrayDepth() const {
		return _arrayDepth;
	}

	void UpdateState(char input) {
		if (input == '{' && !_isInString) {
			++_bracketDepth;
		}
		if (input == '}' && !_isInString) {
			--_bracketDepth;
		}
		if (input == '[' && !_isInString) {
			++_arrayDepth;
		}
		if (input == ']' && !_isInString) {
			--_arrayDepth;
		}
		if (input == '"' && _previousCharacter != '\\') {
			FlipIsInString();
		}
		_previousCharacter = input;
	}

	int32_t GetBracketDepth() const {
		return _bracketDepth;
	}

private:
	int32_t _arrayDepth;
	int32_t _bracketDepth;
	bool _isInString;
	char _previousCharacter;
};

struct TileData {
public:
	TileData(LatLong lower, LatLong upper, const STRING& mapDataJson) : lowerCorner(lower), upperCorner(upper), mapDataJson(mapDataJson) {}
	LatLong lowerCorner;
	LatLong upperCorner;
	const STRING& mapDataJson;
};

static void SeekToValue(const STRING& input, int& i) {
	while (i < LENGTH(input) && (isspace(input[i]) || input[i] == ':')) {
		++i;
	}
}

bool IsNumber(char character) {
	return isdigit(character) || character == '.';
}

template<typename T>
static void ParseNumber(const TileData& inputData, int32_t& i, T& result) {
	int32_t count = 0;

	while (i + count < LENGTH(inputData.mapDataJson) && IsNumber(inputData.mapDataJson[i + count])) {
		++count;
	}
	STRING subString = SUBSTRING(inputData.mapDataJson, i, count);
	i += count - 1;
	if constexpr (std::is_same<std::decay_t<T>, double>::value) {
		result = ATOD(subString);
	}
	else {
		result = ATOI(subString);
	}
}

static void ParseString(const TileData& inputData, int32_t& i, STRING& result) {
	int32_t firstIndex = i + 1;
	int32_t count = 0;

	while ((firstIndex + count) < LENGTH(inputData.mapDataJson) && (inputData.mapDataJson[firstIndex + count] != '"' || (inputData.mapDataJson[firstIndex + count] == '"' && inputData.mapDataJson[firstIndex + count - 1] == '\\'))) {
		++count;
	}
	i = firstIndex + count;

	result = SUBSTRING(inputData.mapDataJson, firstIndex, count);
}


template<typename T>
static void TryParseValueFor(const STRING& itemNameToParse, T& target, const STRING& foundKey, const TileData& inputData, int32_t& i, void (*parseFunction)(const TileData&, int&, T&)) {
	if (foundKey == itemNameToParse) {
		++i;
		SeekToValue(inputData.mapDataJson, i);
		parseFunction(inputData, i, target);
	}
}

static void SeekToNextCoordinate(const STRING& mapDataJson, int32_t& i) {
	while (i < LENGTH(mapDataJson) && !IsNumber(mapDataJson[i])) {
		++i;
	}
}

static void CalculateShapeOrientation(FShape& shape) {
	float result = 0;
	for (int i = 0; i < SIZE(shape.coordinates); ++i) {
		int32_t next = (i + 1) % SIZE(shape.coordinates);
		result += (shape.coordinates[next].localPosition.X - shape.coordinates[i].localPosition.X) * (shape.coordinates[next].localPosition.Y + shape.coordinates[i].localPosition.Y);
	}
	shape.orientation = result > 0;
}

static void ParseShapes(const TileData& inputData, int32_t& i, ARRAY<FShape>& shapes) {
	JsonParserState parser;
	FShape currentShape;
	int32_t maxArrayDepth = 0;
	while (i < LENGTH(inputData.mapDataJson)) {
		parser.UpdateState(inputData.mapDataJson[i]);
		maxArrayDepth = MAX(parser.GetArrayDepth(), maxArrayDepth);
		if (maxArrayDepth == 3 && parser.GetArrayDepth() == 1 && inputData.mapDataJson[i] == ']') {
			ADD(shapes, currentShape);
			CalculateShapeOrientation(currentShape);
			currentShape = FShape();
		}
		if (IsNumber(inputData.mapDataJson[i])) {
			FCoordinate coordinate;
			ParseNumber<double>(inputData, i, coordinate.longitude);
			++i;
			SeekToNextCoordinate(inputData.mapDataJson, i);
			ParseNumber<double>(inputData, i, coordinate.latitude);
			coordinate.localPosition.X = GetRangeMappedValue(coordinate.longitude,
															 inputData.lowerCorner.longitude, 
															 inputData.upperCorner.longitude);
			coordinate.localPosition.Y = GetRangeMappedValue(coordinate.latitude,
															 inputData.lowerCorner.latitude,
															 inputData.upperCorner.latitude);
			ADD(currentShape.coordinates, coordinate);
		}
		if (parser.IsInValidState()) {
			break;
		}
		i += 1;
	}
	if (!EMPTY(currentShape.coordinates)) {
		CalculateShapeOrientation(currentShape);
		ADD(shapes, currentShape);
	}
}

static void ParseGeometry(const TileData& inputData, int32_t& i, FFeatureGeometry& geometry) {
	JsonParserState parser;
	while (i < LENGTH(inputData.mapDataJson)) {
		parser.UpdateState(inputData.mapDataJson[i]);

		if (inputData.mapDataJson[i] == '"' && parser.GetBracketDepth() == 1) {
			parser.FlipIsInString();
			STRING foundKey;
			ParseString(inputData, i, foundKey);
			TryParseValueFor<STRING>("type", geometry.type, foundKey, inputData, i, ParseString);
			TryParseValueFor<ARRAY<FShape>>("coordinates", geometry.shapes, foundKey, inputData, i, ParseShapes);
		}
		if (parser.IsInValidState()) {
			break;
		}
		i += 1;
	}
}

static void ParseProperties(const TileData& inputData, int32_t& i, FProperties& properties) {
	JsonParserState parser;
	while (i < LENGTH(inputData.mapDataJson)) {
		parser.UpdateState(inputData.mapDataJson[i]);

		if (inputData.mapDataJson[i] == '"' && parser.GetBracketDepth() == 1) {
			parser.FlipIsInString();
			STRING foundKey;
			ParseString(inputData, i, foundKey);
			TryParseValueFor<STRING>("kind", properties.kind, foundKey, inputData, i, ParseString);
			TryParseValueFor<STRING>("kind_detail", properties.kindDetail, foundKey, inputData, i, ParseString);
			TryParseValueFor<STRING>("name", properties.name, foundKey, inputData, i, ParseString);
			TryParseValueFor<int32_t>("area", properties.area, foundKey, inputData, i, ParseNumber<int32_t>);
		}
		if (parser.IsInValidState()) {
			break;
		}
		i += 1;
	}
}

static void ParseFeature(const TileData& inputData, int32_t& i, FFeature& feature) {
	JsonParserState parser;
	while (i < LENGTH(inputData.mapDataJson)) {
		parser.UpdateState(inputData.mapDataJson[i]);

		if (inputData.mapDataJson[i] == '"' && parser.GetBracketDepth() == 1) {
			parser.FlipIsInString();
			STRING foundKey;
			ParseString(inputData, i, foundKey);
			TryParseValueFor<FFeatureGeometry>("geometry", feature.geometry, foundKey, inputData, i, ParseGeometry);
			TryParseValueFor<FProperties>("properties", feature.properties, foundKey, inputData, i, ParseProperties);
		}
		if (parser.IsInValidState()) {
			break;
		}
		i += 1;
	}
}

static void ParseFeatureArray(const TileData& inputData, int32_t& i, ARRAY<FFeature*>& featureArray) {
	JsonParserState parser;
	while (i < LENGTH(inputData.mapDataJson)) {
		parser.UpdateState(inputData.mapDataJson[i]);

		if (inputData.mapDataJson[i] == '{' && parser.GetBracketDepth() == 1) {
			FFeature* feature = new FFeature();
			ParseFeature(inputData, i, *feature);
			ADD(featureArray, feature);
			parser.UndoBracketIncrement();
		}
		if (parser.IsInValidState()) {
			break;
		}
		i += 1;
	}
}

static void ParseLayer(const TileData& inputData, int32_t& i, FMapLayer& layer) {
	JsonParserState parser;
	while (i < LENGTH(inputData.mapDataJson)) {
		parser.UpdateState(inputData.mapDataJson[i]);
		if (parser.GetBracketDepth() == 1 && inputData.mapDataJson[i] == '"') {
			STRING foundKey;
			ParseString(inputData, i, foundKey);
			parser.FlipIsInString();
			TryParseValueFor<STRING>("type", layer.type, foundKey, inputData, i, ParseString);
			TryParseValueFor<ARRAY<FFeature*>>("features", layer.features, foundKey, inputData, i, ParseFeatureArray);
		}

		if (parser.IsInValidState()) {
			break;
		}
		++i;
	}
}

bool MapJsonUtils::ProcessMapDataFromJson(const STRING& mapDataJson, FTileJsonData* parsedMapData, int32_t tileX, int32_t tileY)
{
	if (EMPTY(mapDataJson)) {
		return false;
	}
	LatLong tileCornerLow = TileUtils::TileToLatLong(tileX, tileY, 16);
	LatLong tileCornerHigh = TileUtils::TileToLatLong(tileX + 1, tileY + 1, 16);

	TileData inputData(tileCornerLow, tileCornerHigh, mapDataJson);
	JsonParserState parser;
	for (int32_t i = 0; i < LENGTH(mapDataJson); ++i) {
		parser.UpdateState(mapDataJson[i]);
		if (parser.GetBracketDepth() == 1 && mapDataJson[i] == '"') {
			STRING foundKey;
			ParseString(inputData, i, foundKey);
			parser.FlipIsInString();
			TryParseValueFor<FMapLayer>("water", parsedMapData->water, foundKey, inputData, i, ParseLayer);
			TryParseValueFor<FMapLayer>("buildings", parsedMapData->buildings, foundKey, inputData, i, ParseLayer);
		}
	}

	return true;
}
