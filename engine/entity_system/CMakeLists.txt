
set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES

    "${CMAKE_CURRENT_SOURCE_DIR}/Transform.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Transform.cpp"
)

set(LIBRARY_NAME "EntitySystem")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")
