cmake_minimum_required(VERSION 3.10)
project(terraria_clone)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Add source files
set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/World.cpp
    src/Player.cpp
    src/PerlinNoise.cpp
    src/PlayerRenderer.cpp
    src/Items.cpp
    src/Inventory.cpp
)

# Add header files
set(HEADERS
    include/Game.h
    include/World.h
    include/Player.h
    include/PerlinNoise.h
    include/Constants.h
    include/PlayerRenderer.h
    include/Items.h
    include/Inventory.h
)

# Create executable
add_executable(terraria_clone ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(terraria_clone sfml-graphics sfml-window sfml-system)

# Include directories
target_include_directories(terraria_clone PRIVATE include)

# Copy resources to build directory
file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})
