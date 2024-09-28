
set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES

    # Bedrock
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockLog.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockLog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockAssert.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockPlatforms.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockSignal.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockSignalTypes.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockString.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockMemory.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockFile.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockFile.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockPath.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockPath.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockMath.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockMath.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockCommon.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockRotation.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockRotation.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockString.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BedrockString.cpp"
)

set(LIBRARY_NAME "Bedrock")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})