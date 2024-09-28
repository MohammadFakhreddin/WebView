
set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES

    "${CMAKE_CURRENT_SOURCE_DIR}/stb_image/stb_font_consolas_24_latin1.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/stb_image/stb_image_resize.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/stb_image/stb_image_write.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/stb_image/stb_image.h"

    "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann/json_fwd.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann/json.hpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/tiny_gltf_loader/tiny_gltf_loader.h"

    "${CMAKE_CURRENT_SOURCE_DIR}/tiny_obj_loader/tiny_obj_loader.h"

    "${CMAKE_CURRENT_SOURCE_DIR}/LibConfig.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/LibConfig.cpp"
)

set(LIBRARY_NAME "LibConfig")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/stb_image")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/nlohmann")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/tiny_gltf_loader")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/tiny_obj_loader")

