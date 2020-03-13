project(flip)
cmake_minimum_required(VERSION 3.5)

set(WASM_WASM_OLD_BEHAVIOR "Off")
find_package(wasm.cdt)

add_contract( flip flip flip.cpp )
target_include_directories( flip PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( flip ${CMAKE_SOURCE_DIR}/../ricardian )
