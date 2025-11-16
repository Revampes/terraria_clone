cmake_minimum_required(VERSION 3.10)
project(terraria_clone)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Find Box2D manually
find_library(BOX2D_LIBRARY NAMES box2d)
find_path(BOX2D_INCLUDE_DIR box2d/box2d.h)

# Add source files
set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/World.cpp
    src/Player.cpp
    src/PerlinNoise.cpp
)

# Add header files
set(HEADERS
    include/Game.h
    include/World.h
    include/Player.h
    include/PerlinNoise.h
    include/Constants.h
)

# Create executable
add_executable(terraria_clone ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(terraria_clone sfml-graphics sfml-window sfml-system ${BOX2D_LIBRARY})

# Include directories
target_include_directories(terraria_clone PRIVATE include ${BOX2D_INCLUDE_DIR})

# Copy resources to build directory
file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})
