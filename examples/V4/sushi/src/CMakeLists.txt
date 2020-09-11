project(sushi)
cmake_minimum_required(VERSION 3.5)

find_package(wasm.cdt)

add_contract( sushi sushi sushi.cpp )
target_include_directories( uniswap PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( uniswap ${CMAKE_SOURCE_DIR}/../ricardian )
