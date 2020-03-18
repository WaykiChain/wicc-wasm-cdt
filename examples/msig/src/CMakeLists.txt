project(msig)
cmake_minimum_required(VERSION 3.5)

set(WASM_WASM_OLD_BEHAVIOR "Off")
find_package(wasm.cdt)

add_contract( msig msig msig.cpp )
target_include_directories( msig PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( msig ${CMAKE_SOURCE_DIR}/../ricardian )
