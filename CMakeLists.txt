cmake_minimum_required(VERSION 3.10)
project(Stockdredis)

set(CMAKE_CXX_STANDARD 17)

find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
find_library(HIREDIS_LIB hiredis)

if (NOT HIREDIS_INCLUDE_DIR OR NOT HIREDIS_LIB)
    message(FATAL_ERROR "hiredis not found. Please install it.")
endif()

include_directories(${HIREDIS_INCLUDE_DIR})
include_directories(include)

add_executable(Stockdredis
    main.cpp
    src/StockManager.cpp
)

target_link_libraries(Stockdredis ${HIREDIS_LIB})
