#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core.h"

#ifdef DEBUG
    const b32 enableValidationsLayers = true;
#else
    const b32 enableValidationsLayers = false;
#endif

#define VK_CHECK(call) \
	do { \
		VkResult result_ = call; \
		assert(result_ == VK_SUCCESS); \
	} while (0)


const char* const validationLayers[] = {"VK_LAYER_KHRONOS_validation"};


static b32 checkValidationLayerSupport(arena scratch) {
    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties* availableLayers = new(&scratch, VkLayerProperties, layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (size i = 0; i < countof(validationLayers); i++) {
        b32 layerFound = false;
        for (size j = 0; j < layerCount; j++) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;  
}

// Debug Messaging Callback
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    
    //if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        printf("%s\n", pCallbackData->pMessage);
    //}
    

    return VK_FALSE;
}

static u32 getGraphicsQueueFamily(VkPhysicalDevice device, arena scratch) {
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* queueFamProps = new(&scratch, VkQueueFamilyProperties, queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamProps);

    for (u32 i = 0; i < queueFamilyCount; i++) {
        //TODO: Create the surface we want before selecting the queue and check for support here
        if (queueFamProps->queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            return i;
        }
    }

    //TODO: brittle in case of some bizzare compute only hardware
    assert("No queue families support graphics" == NULL);
    return 0;
}

VkDevice createDevice(VkPhysicalDevice physicalDevice, u32 familyIndex)
{
	float queuePriorities[] = { 1.0f };

	VkDeviceQueueCreateInfo queueInfo = { .sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueInfo.queueFamilyIndex = familyIndex;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = queuePriorities;

	/*const char* extensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};*/

    VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = { .sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &queueInfo;
    createInfo.pEnabledFeatures = &deviceFeatures;

	//TODO:?
    //createInfo.ppEnabledExtensionNames = extensions;
	//createInfo.enabledExtensionCount = count(extensions);

	VkDevice device = VK_NULL_HANDLE;
	VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, NULL, &device));

	return device;
}

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window) {
    VkSurfaceKHR surface;
    VkResult res = glfwCreateWindowSurface(instance, window, NULL, &surface);
    printf("%d\n", res);
    VK_CHECK(res);
    return surface;
}

//NOTE: maybe not neccessary
static void printvkInstanceExtensionProperties(arena scratch) {
    u32 extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties* extensions = new(&scratch, VkExtensionProperties, extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

    printf("%u extensions supported:\n", extensionCount);
    for (u32 i = 0; i < extensionCount; i++) {
        printf("\t %s \n", extensions[i].extensionName);
    }
}

