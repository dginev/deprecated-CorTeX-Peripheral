cmake_minimum_required(VERSION 2.8)
project(CorTeX C)

find_library(LIBJSON json-c)
find_library(LIBGEARMAN gearman)
find_library(uthash uthash)

FIND_PACKAGE(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

ADD_DEFINITIONS(
    -std=c99)

add_executable(idf_v0_1.out idf_v0_1.c)
target_link_libraries(idf_v0_1.out ${LIBJSON} ${LIBGEARMAN} ${uthash_LIBRARIES} ${LIBXML2_LIBRARIES})