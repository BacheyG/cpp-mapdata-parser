#ifndef UPROPERTY
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
\
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

    FTileMapData parsedTile;
    MapDataUtils::ProcessMapDataFromGeoJson(jsonContent, &parsedTile, 36232, 22913);

    std::wfstream exampleFile2;
    std::wstring osmContent;
    exampleFile2.open("../data/example.osm", std::ios::in);
    while (exampleFile2) {
        std::wstring temp;
        exampleFile2 >> temp;
        osmContent += temp + L"\n";
    }
    exampleFile2.close();

    MapDataUtils::ProcessMapDataFromOsm(wstringToString(osmContent), &parsedTile, 36232, 22913);

    return 0;
}
#endif