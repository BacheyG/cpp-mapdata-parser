#ifndef UPROPERTY
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <sstream>
#include "MapDataUtils.h"

std::string wstringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

int main() {
    std::fstream exampleFile;
    std::string jsonContent;
    exampleFile.open("../data/example.json", std::ios::in);
    while (exampleFile) {
        std::string temp;
        exampleFile >> temp;
        jsonContent += temp;
    }
    exampleFile.close();

    FTileMapData parsedTileFromJson;
    MapDataUtils::ProcessMapDataFromGeoJson(jsonContent, &parsedTileFromJson, 36232, 22913, 16);

    std::wifstream osmExampleFile("../data/example.osm");
    std::wstringstream osmContent;
    if (osmExampleFile.is_open()) {
        osmContent << osmExampleFile.rdbuf();
    }
    osmExampleFile.close();

    std::wstring contentString = osmContent.str();
    FTileMapData parsedTileFromOsm;
    MapDataUtils::ProcessMapDataFromOsm(wstringToString(contentString), &parsedTileFromOsm, 9058, 5728, 14);

    return 0;
}
#endif