#pragma once

#include "ShapeUtils.h"
#include "FTileMapData.h"

static void SeekToValue(const STRING& input, int& i) {
	while (i < LENGTH(input) && (isspace(input[i]) || input[i] == ':')) {
		++i;
	}
}

template<typename T>
static void TryParseValueFor(const STRING& itemNameToParse, T& target, const STRING& foundKey, const TileData& inputData, int32_t& i, void (*parseFunction)(const TileData&, int&, T&)) {
	if (foundKey == itemNameToParse) {
		++i;
		SeekToValue(inputData.mapDataJson, i);
		parseFunction(inputData, i, target);
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

static void SeekToNextCoordinate(const STRING& mapDataJson, int32_t& i) {
	while (i < LENGTH(mapDataJson) && !IsNumber(mapDataJson[i])) {
		++i;
	}
}

struct JsonParserState {
public:
	JsonParserState() {
		_previousCharacter = ' ';
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

static void ParseShapes(const TileData& inputData, int32_t& i, ARRAY<FMapGeometry*>& shapes) {
	JsonParserState parser;
	FLine* currentShape = new FLine;
	int32_t maxArrayDepth = 0;
	while (i < LENGTH(inputData.mapDataJson)) {
		parser.UpdateState(inputData.mapDataJson[i]);
		maxArrayDepth = MAX(parser.GetArrayDepth(), maxArrayDepth);
		if (maxArrayDepth == 3 && parser.GetArrayDepth() == 1 && inputData.mapDataJson[i] == ']') {
			ADD(shapes, currentShape);
			//ShapeUtils::CalculateShapeOrientation(currentShape);
			currentShape = new FLine();
		}
		if (IsNumber(inputData.mapDataJson[i])) {
			FCoordinate* coordinate;
			ParseNumber<double>(inputData, i, coordinate->globalPosition.longitude);
			++i;
			SeekToNextCoordinate(inputData.mapDataJson, i);
			ParseNumber<double>(inputData, i, coordinate->globalPosition.latitude);
			coordinate->localPosition.X = GetRangeMappedValue(coordinate->globalPosition.longitude,
				inputData.lowerCorner.longitude,
				inputData.upperCorner.longitude);
			coordinate->localPosition.Y = GetRangeMappedValue(coordinate->globalPosition.latitude,
				inputData.lowerCorner.latitude,
				inputData.upperCorner.latitude);
			ADD(currentShape->coordinates, coordinate);
		}
		if (parser.IsInValidState()) {
			break;
		}
		i += 1;
	}
	if (!EMPTY(currentShape->coordinates)) {
		//ShapeUtils::CalculateShapeOrientation(currentShape);
		ADD(shapes, currentShape);
	}
}

static void ParseGeometry(const TileData& inputData, int32_t& i, FMapGeometry& geometry) {
	JsonParserState parser;
	while (i < LENGTH(inputData.mapDataJson)) {
		parser.UpdateState(inputData.mapDataJson[i]);

		if (inputData.mapDataJson[i] == '"' && parser.GetBracketDepth() == 1) {
			parser.FlipIsInString();
			STRING foundKey;
			ParseString(inputData, i, foundKey);
			//TryParseValueFor<STRING>("type", geometry.type, foundKey, inputData, i, ParseString);
			//TryParseValueFor<ARRAY<FShape>>("coordinates", geometry.shapes, foundKey, inputData, i, ParseShapes);
		}
		if (parser.IsInValidState()) {
			break;
		}
		i += 1;
	}
}

static void ParseProperties(const TileData& inputData, int32_t& i, FMapElement& properties) {
	JsonParserState parser;
	while (i < LENGTH(inputData.mapDataJson)) {
		parser.UpdateState(inputData.mapDataJson[i]);

		if (inputData.mapDataJson[i] == '"' && parser.GetBracketDepth() == 1) {
			parser.FlipIsInString();
			STRING foundKey;
			ParseString(inputData, i, foundKey);
			//TryParseValueFor<STRING>("kind", properties.kind, foundKey, inputData, i, ParseString);
			//TryParseValueFor<STRING>("kind_detail", properties.kindDetail, foundKey, inputData, i, ParseString);
			TryParseValueFor<STRING>("name", properties.name, foundKey, inputData, i, ParseString);
			TryParseValueFor<int32_t>("area", properties.area, foundKey, inputData, i, ParseNumber<int32_t>);
		}
		if (parser.IsInValidState()) {
			break;
		}
		i += 1;
	}
}
