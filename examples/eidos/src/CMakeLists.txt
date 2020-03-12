project(eidos)
cmake_minimum_required(VERSION 3.5)

find_package(wasm.cdt)

add_contract( eidos eidos eidos.cpp )
target_include_directories( eidos PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( eidos ${CMAKE_SOURCE_DIR}/../ricardian )
