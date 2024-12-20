#include "BedrockPath.hpp"
#include "BedrockAssert.hpp"
#include "LogicalDevice.hpp"
#include "WebViewApp.hpp"

using namespace MFA;


int main()
{
    LogicalDevice::InitParams params
	{
		.windowWidth = 800,
		.windowHeight = 600,
		.resizable = true,
		.fullScreen = false,
		.applicationName = "Web-View"
	};

    auto const device = LogicalDevice::Instantiate(params);
	MFA_ASSERT(device->IsValid() == true);

    {
        auto webViewApp = std::make_unique<WebViewApp>();
        webViewApp->Run();
    }

	return 0;
}