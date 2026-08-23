cmake_minimum_required(VERSION 3.28)
project(ModDeclare)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Runtime/ModDeclare)

include(CPM.cmake)

CPMAddPackage("gh:nlohmann/json@3.10.5")

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG f091b2c061b307ee89b164c39976fc9202a1c79d)
FetchContent_MakeAvailable(cpr)

include_directories(${PROJECT_SOURCE_DIR}/ModDeclare/Util)
include_directories(${PROJECT_SOURCE_DIR}/ModDeclare/Modrinth)

file(GLOB_RECURSE HDR_FILES ${PROJECT_SOURCE_DIR}/ModDeclare/**.hpp)
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/ModDeclare/**.cpp)

add_executable(ModDeclare main.cpp ${HDR_FILES} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} cpr::cpr nlohmann_json::nlohmann_json)