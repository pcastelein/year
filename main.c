#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cglm/mat4.h>

#include <stdio.h>
#include "core.h"
#include "vksetup.h"


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

    // Debug Message Info Struct
    VkDebugUtilsMessengerEXT debugMessenger;
    VkDebugUtilsMessengerCreateInfoEXT createMessengerInfo = {};
    createMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createMessengerInfo.pfnUserCallback = debugCallback;
    createMessengerInfo.pUserData = NULL; // Optional

    // Instance info Struct
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    u32 glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    //NOTE: keeping around glfwExt pointers in our memory, maybe REFACTOR
    if (enableValidationsLayers) {
        createInfo.enabledLayerCount = (u32) countof(validationLayers);
        createInfo.ppEnabledLayerNames = validationLayers;
        
        const char** glfwExtPlus = new(&mem, const char*, glfwExtensionCount + 1);
        for (u32 i = 0; i < glfwExtensionCount; i++) {
            glfwExtPlus[i] = glfwExtensions[i];
        }
        const char extDebug[] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        glfwExtPlus[glfwExtensionCount] = extDebug;
        createInfo.enabledExtensionCount = glfwExtensionCount + 1;
        createInfo.ppEnabledExtensionNames = (const char**) glfwExtPlus;
        //Include debug messenger info to be able to debug vkCreateInstance and vkDestroyInsance usage.
        createInfo.pNext = &createMessengerInfo;
    }
    else {
        createInfo.enabledLayerCount = 0; //NOTE: telling the driver we aren't using validation layers
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
    }

    VkInstance instance;
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        fprintf(stderr, "failed to create Vulkan instance!\n");
        osfail();
    }

    //Setup Vulkan Layer Debugging TODO: send an allocator
    PFN_vkCreateDebugUtilsMessengerEXT funcDebugMess = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (funcDebugMess == NULL) {
        fprintf(stderr, "Failed to aquire function to setup Debug messenger.\n");
        osfail();
    }
    else {
        if(funcDebugMess(instance, &createMessengerInfo, NULL, &debugMessenger) != VK_SUCCESS) {
            fprintf(stderr, "Failed to setup Debug messenger.\n");
            osfail();
        }
    }

    glfwMakeContextCurrent(window);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    //NOTE: memory, I don't think there is a point in destroying these if we are exiting
/*     if (enableValidationsLayers) { //NOTE: vkDestroyDebug.. hasn't been loaded.
        vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
    } */
    //vkDestroyInstance(instance, NULL);

    //glfwDestroyWindow(window);

    //glfwTerminate();

    
    return 0;
}