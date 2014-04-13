cmake_minimum_required(VERSION 2.8)
project(CorTeX_Peripheral C)
SET( ${PROJECT_NAME}_MAJOR_VERSION 0 )
SET( ${PROJECT_NAME}_MINOR_VERSION 1 )

# Compiler flags
SET(COMMON_FLAGS
    "-Wall -W -Wextra")
SET(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")

find_library(LIBJSON json-c)
find_library(LIBGEARMAN gearman)
find_library(uthash uthash)

include_directories ("${PROJECT_SOURCE_DIR}/lib"
                     "${CMAKE_CURRENT_BINARY_DIR}")
add_library(IDF_Score lib/idf_score.c)

FIND_PACKAGE(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

add_executable(cortex_worker.o lib/cortex_worker.c)
target_link_libraries(cortex_worker.o IDF_Score ${LIBJSON} ${LIBGEARMAN} ${uthash_LIBRARIES} ${LIBXML2_LIBRARIES})

set (EXTRA_LIBS ${EXTRA_LIBS} IDF_Score)

# Output executables at the root of build tree
SET_PROPERTY( TARGET cortex_worker.o
        PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin )