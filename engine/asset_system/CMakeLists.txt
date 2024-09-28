
set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/AssetShader.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AssetShader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AssetTexture.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AssetTexture.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AssetGLTF_Mesh.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AssetGLTF_Mesh.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AssetGLTF_Model.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AssetGLTF_Model.cpp"
)

set(LIBRARY_NAME "AssetSystem")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")
