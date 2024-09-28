set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Time.hpp"  
    "${CMAKE_CURRENT_SOURCE_DIR}/Time.cpp"  
)

set(LIBRARY_NAME "TimeSystem")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")
