########################################

set(EXECUTABLE "WebView")

set(EXECUTABLE_RESOURCES)

list(
    APPEND EXECUTABLE_RESOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/WebViewMain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/WebViewApp.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/WebViewApp.cpp"
)

add_executable(${EXECUTABLE} ${EXECUTABLE_RESOURCES})


########################################