cmake_minimum_required(VERSION 2.8)
project(CorTeX C)

find_library(LIBJSON json-c)
find_library(LIBGEARMAN gearman)
find_library(uthash uthash)

FIND_PACKAGE(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

add_executable(idf_v0_1 lib/idf_v0_1.c)
target_link_libraries(idf_v0_1 ${LIBJSON} ${LIBGEARMAN} ${uthash_LIBRARIES} ${LIBXML2_LIBRARIES})

#SET(
#  CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

# Output executables at the root of build tree
SET_PROPERTY( TARGET idf_v0_1
        PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin )