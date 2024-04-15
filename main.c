#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cglm/mat4.h>

#include <stdio.h>
#include "core.h"

#ifdef DEBUG
    const b32 enableValidationsLayers = true;
#else
    const b32 enableValidationsLayers = false;
#endif

const u32 WIDTH = 800;
const u32 HEIGHT = 600;

const char* const validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

static b32 checkValidationLayerSupport(arena scratch) {
    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    //VkLayerProperties* availableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
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
    //free(availableLayers); //TODO: Memory
    return true;  
}

int main(/*int argc, char* argv[]*/) {
    //App Memory, 256MB
    enum { CAP = 1 << 28 };
    arena mem = newarena(CAP);
    if (mem.end == NULL) {oom();}
    
    //GLFW Initialization
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //resizing sisabled until handling buffers 

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Window", NULL, NULL);

    if (window == NULL) {
        glfwTerminate();
        osfail();
    }

    // Vulkan Instance setup
    // Validation Layers
    if (enableValidationsLayers && !checkValidationLayerSupport(mem)) {
        fprintf(stderr, "validation layers requested, but not available!\n");
        osfail();
    }

    //OPTIONAL:
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Bench";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 1);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_0; //NOTE: telling driver we are on Vulkan 1.0

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    u32 glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    if (enableValidationsLayers) {
        createInfo.enabledLayerCount = (u32) countof(validationLayers);
        createInfo.ppEnabledLayerNames = validationLayers;
    }
    else {
        createInfo.enabledLayerCount = 0; //NOTE: telling the driver we aren't using validation layers
    }
    
    VkInstance instance;
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        fprintf(stderr, "failed to create Vulkan instance!\n");
        osfail();
    }
    u32 extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties* extensions = new(&mem, VkExtensionProperties, extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

    printf("%u extensions supported:\n", extensionCount);
    for (u32 i = 0; i < extensionCount; i++) {
        printf("\t %s \n", extensions[i].extensionName);
    }

    glfwMakeContextCurrent(window);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    //NOTE: memory, I don't think there is a point in destroying these if we are exiting
    //vkDestroyInstance(instance, NULL);

    //glfwDestroyWindow(window);

    //glfwTerminate();

    
    return 0;
}