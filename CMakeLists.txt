cmake_minimum_required(VERSION 3.30)
project(cpp-mapdata-parser)

###############################################################################
## file globbing ##############################################################
###############################################################################

# these instructions search the directory tree when CMake is
# invoked and put all files that match the pattern in the variables
# `sources` and `data`
file(GLOB_RECURSE sources  Source/*.cpp Source/*.h)


###############################################################################
## target definitions #########################################################
###############################################################################

# add the data to the target, so it becomes visible in some IDE
add_executable(cpp-mapdata-parser ${sources} ${data})

# just for example add some compiler flags
target_compile_options(cpp-mapdata-parser PUBLIC /std=c++1y /Wall)

# this lets me include files relative to the root source directory with a <> pair
target_include_directories(cpp-mapdata-parser PUBLIC Source)

###############################################################################
## packaging ##################################################################
###############################################################################

# all install commands get the same destination. this allows us to use paths
# relative to the executable.
install(TARGETS cpp-mapdata-parser DESTINATION bin)