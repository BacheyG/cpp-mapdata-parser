#ifndef UPROPERTY
#include <iostream>
#include <fstream>

#include "MapDataUtils.h"

int main() {
    std::fstream exampleFile;
    std::string jsonContent;
    exampleFile.open("../data/example.json", std::ios::in);
    while (exampleFile) {
        std::string temp;
        exampleFile >> temp;
        jsonContent += temp;
    }
    std::cout << jsonContent;
    exampleFile.close();

    FTileMapData parsedTile;
    MapJsonUtils::ProcessMapDataFromGeoJson(jsonContent, &parsedTile, 36232, 22913);

    return 0;
}
#endif