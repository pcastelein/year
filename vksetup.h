#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core.h"

#ifdef DEBUG
    const b32 enableValidationsLayers = true;
#else
    const b32 enableValidationsLayers = false;
#endif

const u32 WIDTH = 800;
const u32 HEIGHT = 600;

const char* const validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

//TODO: Work in Progress
typedef struct {
    b32 queuegraphicsbit;
} QueueFamilyIndices;

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

static b32 isDeviceSuitable(VkPhysicalDevice device, arena scratch) {
    QueueFamilyIndices indices = {};
    
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* queueFamProps = new(&scratch, VkQueueFamilyProperties, queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamProps);

    for (u32 i = 0; i < queueFamilyCount; i++) {
        if (queueFamProps->queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.queuegraphicsbit = true;
        }
        if (indices.queuegraphicsbit) {
            break;
        }
    }

    return indices.queuegraphicsbit;
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

