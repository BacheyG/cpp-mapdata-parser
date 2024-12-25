cmake_minimum_required(VERSION 3.30)
project(cpp-mapdata-parser)

###############################################################################
## File Globbing ##############################################################
###############################################################################

file(GLOB_RECURSE sources Source/*.cpp Source/*.h Source/*.hpp)

###############################################################################
## Target Definitions #########################################################
###############################################################################

# Add shared library for map data utilities
add_library(mapdatautils_export SHARED 
	Source/LatLong.cpp
	Source/MapDataUtils.cpp
	Source/OsmParserUtils.cpp
	Source/ShapeUtils.cpp
    Source/TileBuildingDataUtils.cpp
	Source/TileUtils.cpp
	Source/tinyxml2.cpp
)

# Include directory for shared library
target_include_directories(mapdatautils_export PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# Add executable
add_executable(cpp-mapdata-parser ${sources})

# Compiler flags
target_compile_options(cpp-mapdata-parser PUBLIC /std:c++17 /Wall)

# Include directories for the executable
target_include_directories(cpp-mapdata-parser PUBLIC Source)

# Link executable with the shared library
target_link_libraries(cpp-mapdata-parser PRIVATE mapdatautils_export)

###############################################################################
## Output Properties ##########################################################
###############################################################################

# Set output name and directory for shared library
set_target_properties(mapdatautils_export PROPERTIES 
    OUTPUT_NAME "mapdatautils"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Set output directory for the executable
set_target_properties(cpp-mapdata-parser PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

###############################################################################
## Packaging ##################################################################
###############################################################################

install(TARGETS cpp-mapdata-parser DESTINATION bin)
install(TARGETS mapdatautils_export DESTINATION bin)
