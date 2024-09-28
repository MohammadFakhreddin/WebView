
set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES

    "${CMAKE_CURRENT_SOURCE_DIR}/JobSystem.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/JobSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ScopeLock.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ScopeLock.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ScopeProfiler.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ScopeProfiler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThreadPool.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThreadPool.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThreadSafeQueue.hpp"   
)

set(LIBRARY_NAME "JobSystem")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")
