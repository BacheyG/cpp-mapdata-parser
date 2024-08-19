#include <iostream>
#include <fstream>

#include "MapJsonUtils.h"

int main() {
    std::fstream exampleFile;
    std::string jsonContent;
    exampleFile.open("data/example.txt", std::ios::in);
    while (exampleFile) {
        std::string temp;
        exampleFile >> temp;
        jsonContent += temp;
    }
    std::cout << jsonContent;
    exampleFile.close();

    FTileJsonData parsedTile;
    MapJsonUtils::ProcessMapDataFromJson(jsonContent, &parsedTile, 36232, 22913);

    return 0;
}