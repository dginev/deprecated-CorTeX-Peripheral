# CorTeX Peripherals 
cmake_minimum_required(VERSION 2.8)
project(CorTeX_Peripheral C)
SET( ${PROJECT_NAME}_MAJOR_VERSION 0 )
SET( ${PROJECT_NAME}_MINOR_VERSION 1 )
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

# Compiler flags
SET(COMMON_FLAGS
    "-Wall -W -Wextra")
SET(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")

find_library(LIBGEARMAN gearman)
find_library(uthash uthash)
find_package(Json REQUIRED)
find_library(LIBLLAMAPUN llamapun REQUIRED)

include_directories ("${PROJECT_SOURCE_DIR}/clib"
                     "${CMAKE_CURRENT_BINARY_DIR}"
                     ${JSON_INCLUDE_DIRS})
add_library(cortex_utils clib/cortex_utils.c)
add_library(IDF_Score clib/idf_score.c)
add_library(llamapun_interface clib/llamapun_interface.c)

FIND_PACKAGE(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

add_executable(cortex_worker clib/cortex_worker.c)
target_link_libraries(llamapun_interface ${LIBLLAMAPUN})
target_link_libraries(cortex_worker cortex_utils IDF_Score llamapun_interface
	${LIBLLAMAPUN} ${JSON_LIBRARIES} ${LIBGEARMAN} ${uthash_LIBRARIES} ${LIBXML2_LIBRARIES})

set (EXTRA_LIBS ${EXTRA_LIBS} IDF_Score)

# Output executables at the root of build tree
SET_PROPERTY( TARGET cortex_worker
        PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin )
