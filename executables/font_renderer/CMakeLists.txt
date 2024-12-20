########################################

set(EXECUTABLE "FontRenderer")

set(EXECUTABLE_RESOURCES)

list(
    APPEND EXECUTABLE_RESOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/FontRendererMain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/FontRendererApp.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/FontRendererApp.cpp"
)

add_executable(${EXECUTABLE} ${EXECUTABLE_RESOURCES})


########################################