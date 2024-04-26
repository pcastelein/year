#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cglm/mat4.h>

#include <stdio.h>

#include "defines.h"

int main(int argc, char* argv[]) {

    //GLFW Initialization
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //resizing sisabled until handling buffers 
    u32 WIDTH = 800;
    u32 HEIGHT = 600;
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", NULL, NULL);

    if (window == NULL) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Vulkan Instance setup
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
    createInfo.enabledLayerCount = 0; //NOTE: telling the driver we aren't using validation layers
    VkInstance instance;
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        fprintf(stderr, "failed to create Vulkan instance!");
        return EXIT_FAILURE;
    }
    u32 extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties* extensions = malloc(sizeof(extensionCount) * extensionCount); //TODO: memory
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

    printf("%d extensions supported:\n", extensionCount);
    for (u32 i = 0; i < extensionCount; i++) {
        printf("\t %s \n", extensions[i].extensionName);
    }

    glfwMakeContextCurrent(window);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    //TODO: memory
    vkDestroyInstance(instance, NULL);
    free(extensions);

    glfwDestroyWindow(window);

    glfwTerminate();

    
    return 0;
}