
set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES

    # Bedrock
    "${CMAKE_CURRENT_SOURCE_DIR}/RenderBackend.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RenderBackend.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/LogicalDevice.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/LogicalDevice.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RenderTypes.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RenderTypes.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DescriptorSetSchema.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DescriptorSetSchema.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BufferTracker.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BufferTracker.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/UI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/UI.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/render_resource/RenderResource.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/render_resource/RenderResource.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/render_resource/DepthRenderResource.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/render_resource/DepthRenderResource.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/render_resource/SwapChainRenderResource.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/render_resource/SwapChainRenderResource.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/render_resource/MSAA_RenderResource.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/render_resource/MSAA_RenderResource.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/render_pass/RenderPass.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/render_pass/RenderPass.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/render_pass/DisplayRenderPass.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/render_pass/DisplayRenderPass.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/LinePipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/LinePipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/PointPipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/PointPipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/FlatShadingPipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/FlatShadingPipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/BlinnPhongPipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/BlinnPhongPipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/TextOverlayPipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/TextOverlayPipeline.hpp"
    
    "${CMAKE_CURRENT_SOURCE_DIR}/camera/PerspectiveCamera.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/camera/PerspectiveCamera.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/camera/ObserverCamera.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/camera/ObserverCamera.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/camera/ArcballCamera.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/camera/ArcballCamera.cpp"
    
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/LineRenderer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/LineRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/PointRenderer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/PointRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/ConsolasFontRenderer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/ConsolasFontRenderer.cpp"
    
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/MeshRenderer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/MeshRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/MeshInstance.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/MeshInstance.cpp"
)

set(LIBRARY_NAME "RenderSystem")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")
