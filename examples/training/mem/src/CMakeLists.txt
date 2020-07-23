project(mem)
cmake_minimum_required(VERSION 3.5)

find_package(wasm.cdt)

add_contract( mem mem mem.cpp )
target_include_directories( mem PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( mem ${CMAKE_SOURCE_DIR}/../ricardian )
