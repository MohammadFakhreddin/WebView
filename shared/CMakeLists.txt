set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/BinaryReader.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BinaryReader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/FontData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/FontData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/FontParser.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/FontParser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/FontAtlasPipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/FontAtlasPipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/WebViewContainer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/WebViewContainer.cpp"
)

set(LIBRARY_NAME "shared")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")
