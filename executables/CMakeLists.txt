########################################

set(EXECUTABLE "MeshViewer")

set(EXECUTABLE_RESOURCES)

list(
    APPEND EXECUTABLE_RESOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/WebViewDemoMain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/WebViewContainer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/WebViewContainer.cpp"
)

add_executable(${EXECUTABLE} ${EXECUTABLE_RESOURCES})


########################################