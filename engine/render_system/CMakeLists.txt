list(
    APPEND LIBRARY_SOURCES

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

    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/IShadingPipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/TextOverlayPipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/TextOverlayPipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/SolidFillPipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/SolidFillPipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/ImagePipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/ImagePipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/BorderPipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/BorderPipeline.hpp"
    
    "${CMAKE_CURRENT_SOURCE_DIR}/camera/PerspectiveCamera.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/camera/PerspectiveCamera.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/camera/ObserverCamera.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/camera/ObserverCamera.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/camera/ArcballCamera.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/camera/ArcballCamera.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/utils/ConsolasFontRenderer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/ConsolasFontRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/CustomFontRenderer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/CustomFontRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/SolidFillRenderer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/SolidFillRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/ImageRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/ImageRenderer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/BorderRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/BorderRenderer.hpp"
)

set(LIBRARY_NAME "RenderSystem")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")
