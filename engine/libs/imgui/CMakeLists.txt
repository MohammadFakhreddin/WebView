list(
    APPEND LIBRARY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui_demo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui_internal.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui_tables.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui_widgets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui_stdlib.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui_stdlib.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/imstb_rectpack.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/imstb_textedit.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/imstb_truetype.h"
)

set(LIBRARY_NAME "Imgui")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})