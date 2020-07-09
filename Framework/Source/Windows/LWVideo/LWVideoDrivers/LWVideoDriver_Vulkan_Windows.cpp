#include "LWVideo/LWVideoDrivers/LWVideoDriver_Vulkan.h"
#include <LWCore/LWText.h>
#include <LWPlatform/LWWindow.h>
#include <iostream>

LWVideoDriver_Vulkan *LWVideoDriver_Vulkan::MakeVideoDriver(LWWindow *Window, uint32_t Type) {
	const bool VerboseError = true;
	LWVulkan_Context Context;
	const std::vector<const char*> DebugLayerNames = { "VK_LAYER_KHRONOS_validation" };
	bool DebugLayerEnabled = (Type&DebugLayer) != 0;
	LWWindowContext &WinCon = Window->GetContext();
	LWVector2i WinSize = Window->GetSize();

	struct DeviceQueueList {
		uint32_t GraphicsQueueID = -1;
		VkSurfaceFormatKHR SurfaceFormat;
		VkPresentModeKHR PresentFormat;
	};

	std::vector<VkExtensionProperties> ExtensionList;
	std::vector<VkSurfaceFormatKHR> SurfaceFormatList;
	std::vector<VkPresentModeKHR> PresentModeList;
	std::vector<const char*> InstanceExtensionList = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
	std::vector<const char*> DeviceExtensionList = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	auto HasRequiredExtensions = [](const std::vector<VkExtensionProperties> &ExtensionList, std::vector<const char*> &RequiredExtensionList) -> bool {
		for (auto &&ExtName : RequiredExtensionList) {
			bool Has = false;
			for (auto &&Ext : ExtensionList) {
				if (Has = LWText::Compare(ExtName, Ext.extensionName)) break;
			}
			if (!Has) return false;
		}
		return true;
	};

	auto InstanceHasRequiredExtensions = [&ExtensionList, &InstanceExtensionList, &HasRequiredExtensions]()->bool {
		uint32_t ExtensionCount = 0;
		if (vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr) != VK_SUCCESS) return false;
		ExtensionList.resize(ExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, ExtensionList.data());
		return HasRequiredExtensions(ExtensionList, InstanceExtensionList);
	};

	auto HasDebugLayers = [](const std::vector<const char*> &DebugLayerNames)->bool {
		uint32_t DebugLayerCount = 0;
		if (vkEnumerateInstanceLayerProperties(&DebugLayerCount, nullptr) != VK_SUCCESS) return false;
		std::vector<VkLayerProperties> vDebugLayer(DebugLayerCount);
		vkEnumerateInstanceLayerProperties(&DebugLayerCount, vDebugLayer.data());
		for (auto &&DName : DebugLayerNames) {
			bool Has = false;
			for (auto &Iter = vDebugLayer.begin(); Iter != vDebugLayer.end() && !Has; ++Iter) {
				Has = LWText::Compare(Iter->layerName, DName);
			}
			if (!Has) return false;
		}
		return true;
	};

	auto FindSupportedColorFormat = [&Context, &SurfaceFormatList](VkPhysicalDevice Device, VkSurfaceFormatKHR &Format, uint32_t &Score) -> bool {
		uint32_t FormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Context.m_Surface, &FormatCount, nullptr);
		if (FormatCount == 0) return false;
		SurfaceFormatList.resize(FormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Context.m_Surface, &FormatCount, SurfaceFormatList.data());
		for (auto &&SFormat : SurfaceFormatList) {
			if (SFormat.format == VK_FORMAT_B8G8R8A8_SRGB && SFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				Format = SFormat;
				Score += 100; //Higher score for having the format we desire.
				return true;
			}
		}
		Format = SurfaceFormatList[0];
		return true;
	};

	auto FindSupportedPresentFormat = [&Context, &PresentModeList](VkPhysicalDevice Device, VkPresentModeKHR &PresentMode, uint32_t &Score)->bool {
		uint32_t PresentCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Context.m_Surface, &PresentCount, nullptr);
		if (PresentCount == 0) return false;
		PresentModeList.resize(PresentCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Context.m_Surface, &PresentCount, PresentModeList.data());
		for (auto &&PMode : PresentModeList) {
			if (PMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				PresentMode = PMode;
				Score += 100; //Higher score for having the format we desire.
				return true;
			}
		}
		PresentMode = VK_PRESENT_MODE_FIFO_KHR; //Guaranteed mode to have.
		return true;
	};

	auto EvaluateDevice = [&Context, &ExtensionList, &DeviceExtensionList, &HasRequiredExtensions, &FindSupportedColorFormat, &FindSupportedPresentFormat](VkPhysicalDevice Device, DeviceQueueList &TargetQueues)->uint32_t {
		VkPhysicalDeviceProperties Properties;
		VkPhysicalDeviceFeatures Features;
		vkGetPhysicalDeviceProperties(Device, &Properties);
		vkGetPhysicalDeviceFeatures(Device, &Features);
		uint32_t ExtensionCount = 0;
		if (vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, nullptr) != VK_SUCCESS) return 0;
		ExtensionList.resize(ExtensionCount);
		vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, ExtensionList.data());
		if (!Features.textureCompressionBC) return 0;
		if (!HasRequiredExtensions(ExtensionList, DeviceExtensionList)) return 0;
		uint32_t Score = 1;
		if (!FindSupportedColorFormat(Device, TargetQueues.SurfaceFormat, Score)) return 0;
		if (!FindSupportedPresentFormat(Device, TargetQueues.PresentFormat, Score)) return 0;
		if (Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) Score += 1000;
		uint32_t QueueCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueCount, nullptr);
		std::vector<VkQueueFamilyProperties> QueueList(QueueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueCount, QueueList.data());
		uint32_t QueueIndex = 0;
		for (auto &&Iter : QueueList) {
			VkBool32 PresentSupported = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(Device, QueueIndex, Context.m_Surface, &PresentSupported);
			if (PresentSupported) {
				if (Iter.queueFlags&VK_QUEUE_GRAPHICS_BIT) TargetQueues.GraphicsQueueID = QueueIndex;
			}
			QueueIndex++;
		}
		if (TargetQueues.GraphicsQueueID == -1) return 0;
		return Score;
	};

	auto HasPhysicalDevice = [&Context, &EvaluateDevice](VkPhysicalDevice &Result, DeviceQueueList &QueueList)->bool {
		uint32_t DeviceCount = 0;
		if (vkEnumeratePhysicalDevices(Context.m_Instance, &DeviceCount, nullptr) != VK_SUCCESS) return false;
		std::vector<VkPhysicalDevice> DeviceList(DeviceCount);
		vkEnumeratePhysicalDevices(Context.m_Instance, &DeviceCount, DeviceList.data());
		uint32_t CurrentScore = 0;
		for (auto &&Iter : DeviceList) {
			bool ValidDevice = false;
			DeviceQueueList DQueueList;
			uint32_t Score = EvaluateDevice(Iter, DQueueList);
			if(Score==0) continue;
			if (Score > CurrentScore) {
				Result = Iter;
				CurrentScore = Score;
				QueueList = DQueueList;
			}
		}
		return CurrentScore!=0;
	};

	auto EvaluateResult = [&Context](const char *ErrorValue, VkResult Result, bool Verbose)->bool {
		if(!DebugResult(ErrorValue, Result, Verbose)){
			for (uint32_t i = 0; i < (uint32_t)Context.m_SwapImageViewList.size(); i++) {
				VkImageView View = Context.m_SwapImageViewList[i];
				if(!View) continue;
				vkDestroyImageView(Context.m_Device, View, nullptr);
			}
			if (Context.m_Swapchain) vkDestroySwapchainKHR(Context.m_Device, Context.m_Swapchain, nullptr);
			if (Context.m_Device) vkDestroyDevice(Context.m_Device, nullptr);
			if (Context.m_Surface) vkDestroySurfaceKHR(Context.m_Instance, Context.m_Surface, nullptr);
			if (Context.m_Instance) vkDestroyInstance(Context.m_Instance, nullptr);
			return false;
		}
		return true;
	};

	auto EvaluateBool = [&EvaluateResult](const char *ErrorValue, bool Result, bool Verbose)->bool {
		return EvaluateResult(ErrorValue, Result == true ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED, Verbose);
	};

	if (!EvaluateBool("InstanceHasRequiredExtensions", InstanceHasRequiredExtensions(), VerboseError)) return nullptr;
	VkApplicationInfo AppInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, (const char*)Window->GetTitle().GetCharacters(), VK_MAKE_VERSION(1,0,0), "Lightwave", VK_MAKE_VERSION(1,0,0), VK_API_VERSION_1_2 };
	VkInstanceCreateInfo InstanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, &AppInfo, 0, nullptr, (uint32_t)InstanceExtensionList.size(), InstanceExtensionList.data() };
	if (DebugLayerEnabled) {
		if (HasDebugLayers(DebugLayerNames)) {
			InstanceInfo.enabledLayerCount = (uint32_t)DebugLayerNames.size();
			InstanceInfo.ppEnabledLayerNames = DebugLayerNames.data();
		}
	}
	if(!EvaluateResult("vkCreateInstance", vkCreateInstance(&InstanceInfo, nullptr, &Context.m_Instance), VerboseError)) return nullptr;

	VkWin32SurfaceCreateInfoKHR SurfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, WinCon.m_Instance, WinCon.m_WND };
	if(!EvaluateResult("vkCreateWin32SurfaceKHR", vkCreateWin32SurfaceKHR(Context.m_Instance, &SurfaceInfo, nullptr, &Context.m_Surface), VerboseError)) return nullptr;

	VkPhysicalDevice PDevice;
	DeviceQueueList PDeviceQueues;
	float PQueuePriorities = 1.0f;
	if (!EvaluateBool("HasPhysicalDevice", HasPhysicalDevice(PDevice, PDeviceQueues), VerboseError)) return nullptr;

	VkDeviceQueueCreateInfo QueueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, PDeviceQueues.GraphicsQueueID, 1, &PQueuePriorities };
	VkPhysicalDeviceFeatures DeviceFeatures = {};
	VkDeviceCreateInfo DeviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, 1, &QueueInfo, 0, nullptr, (uint32_t)DeviceExtensionList.size(), DeviceExtensionList.data(), &DeviceFeatures };
	if (DebugLayerEnabled) {
		DeviceInfo.enabledLayerCount = (uint32_t)DebugLayerNames.size();
		DeviceInfo.ppEnabledLayerNames = DebugLayerNames.data();
	}
	if (!EvaluateResult("vkCreateDevice", vkCreateDevice(PDevice, &DeviceInfo, nullptr, &Context.m_Device), VerboseError)) return nullptr;
	vkGetDeviceQueue(Context.m_Device, PDeviceQueues.GraphicsQueueID, 0, &Context.m_GraphicsQueue);

	VkSurfaceCapabilitiesKHR SurfaceCapabilitys;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PDevice, Context.m_Surface, &SurfaceCapabilitys);

	uint32_t ImageCount = SurfaceCapabilitys.minImageCount + 1;
	VkExtent2D Extents = SurfaceCapabilitys.currentExtent;
	if (Extents.width == UINT32_MAX) {
		VkExtent2D MinExtents = SurfaceCapabilitys.minImageExtent;
		VkExtent2D MaxExtents = SurfaceCapabilitys.maxImageExtent;
		Extents.width = std::min<uint32_t>(std::max<uint32_t>(WinSize.x, MinExtents.width), MaxExtents.width);
		Extents.height = std::min<uint32_t>(std::max<uint32_t>(WinSize.y, MinExtents.height), MaxExtents.height);
	}
	if (SurfaceCapabilitys.maxImageCount > 0) ImageCount = std::min<uint32_t>(ImageCount, SurfaceCapabilitys.maxImageCount);
	VkSwapchainCreateInfoKHR SwapchainInfo = {};
	SwapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapchainInfo.surface = Context.m_Surface;
	SwapchainInfo.minImageCount = ImageCount;
	SwapchainInfo.imageFormat = PDeviceQueues.SurfaceFormat.format;
	SwapchainInfo.imageColorSpace = PDeviceQueues.SurfaceFormat.colorSpace;
	SwapchainInfo.imageExtent = Extents;
	SwapchainInfo.imageArrayLayers = 1;
	SwapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	SwapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	SwapchainInfo.preTransform = SurfaceCapabilitys.currentTransform;
	SwapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapchainInfo.presentMode = PDeviceQueues.PresentFormat;
	SwapchainInfo.clipped = VK_TRUE;
	SwapchainInfo.oldSwapchain = VK_NULL_HANDLE;
	if (!EvaluateResult("vkCreateSwapchainKHR", vkCreateSwapchainKHR(Context.m_Device, &SwapchainInfo, nullptr, &Context.m_Swapchain), VerboseError)) return nullptr;
	if(!EvaluateResult("vkGetSwapchainImagesKHR", vkGetSwapchainImagesKHR(Context.m_Device, Context.m_Swapchain, &ImageCount, nullptr), VerboseError)) return nullptr;
	Context.m_SwapImageList.resize(ImageCount);
	Context.m_SwapImageViewList.resize(ImageCount);

	std::fill(Context.m_SwapImageViewList.begin(), Context.m_SwapImageViewList.end(), nullptr);
	vkGetSwapchainImagesKHR(Context.m_Device, Context.m_Swapchain, &ImageCount, Context.m_SwapImageList.data());
	for (uint32_t i = 0; i < (uint32_t)Context.m_SwapImageList.size(); i++) {
		VkImageViewCreateInfo ImageViewCreateInfo = {};
		ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCreateInfo.image = Context.m_SwapImageList[i];
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCreateInfo.format = PDeviceQueues.SurfaceFormat.format;
		ImageViewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		if (!EvaluateResult("vkCreateImageView", vkCreateImageView(Context.m_Device, &ImageViewCreateInfo, nullptr, &Context.m_SwapImageViewList[i]), VerboseError)) return nullptr;

	}

	return Window->GetAllocator()->Allocate<LWVideoDriver_Vulkan>(Window, Context, 16);
}

bool LWVideoDriver_Vulkan::DestroyVideoContext(LWVideoDriver_Vulkan *Driver) {
	LWVulkan_Context &Con = Driver->GetContext();
	for (uint32_t i = 0; i < (uint32_t)Con.m_SwapImageViewList.size(); i++) {
		VkImageView View = Con.m_SwapImageViewList[i];
		vkDestroyImageView(Con.m_Device, View, nullptr);
	}
	vkDestroySwapchainKHR(Con.m_Device, Con.m_Swapchain, nullptr);
	vkDestroyDevice(Con.m_Device, nullptr);
	vkDestroySurfaceKHR(Con.m_Instance, Con.m_Surface, nullptr);
	vkDestroyInstance(Con.m_Instance, nullptr);
	return true;
}

bool LWVideoDriver_Vulkan::Update(void) {
	return true;
}

LWVideoDriver &LWVideoDriver_Vulkan::Present(uint32_t SwapInterval) {
	return *this;
}