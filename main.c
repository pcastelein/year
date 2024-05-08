#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cglm/mat4.h>

#include <stdio.h>
#include "core.h"
#include "vksetup.h"


int main(int argc, char* argv[]) {
    //App Memory, 256MB
    enum { CAP = 1 << 28 };
    arena mem = newarena(CAP);
    if (mem.end == NULL) {oom();}

    //GLFW Initialization
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //resizing disabled until handling buffers 

    // Vulkan Instance setup
    // Validation Layers
    if (enableValidationsLayers && !checkValidationLayerSupport(mem)) {
        fprintf(stderr, "validation layers requested, but not available!\n");
        osfail();
    }

    //OPTIONAL:
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Year";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1; //NOTE: telling driver we are on Vulkan 1.0

    // Debug Message Info Struct
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
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
    printf("%u glfwExtensions supported:\n", glfwExtensionCount);
    for (u32 i = 0; i < glfwExtensionCount; i++) {
        printf("\t %s \n", glfwExtensions[i]);
    }
    //NOTE: keeping around glfwExt pointers in our memory, maybe REFACTOR
    if (enableValidationsLayers) {
        createInfo.enabledLayerCount = (u32) countof(validationLayers);
        createInfo.ppEnabledLayerNames = validationLayers;
        
        const char** glfwExtPlus = new(&mem, const char*, glfwExtensionCount + 1);
        for (u32 i = 0; i < glfwExtensionCount; i++) {
            glfwExtPlus[i] = glfwExtensions[i];
        }
        glfwExtPlus[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
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
    if (enableValidationsLayers) {
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
    }

    //VK Physical Device Setup TODO - janky interaction with selecting queue and not commpatible with iGPUs
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    u32 deviceCount = 0;
    u32 familyIndex = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        fprintf(stderr, "Failed to find a GPU with Vulkan support.");
        osfail();
    }
    else { // stack it
        arena scratch = mem;
        VkPhysicalDevice *pdarr = new(&scratch, VkPhysicalDevice, deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, pdarr);
        for (u32 i = 0; i < deviceCount; i++) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(pdarr[i], &props);
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			    printf("Picking discrete GPU: %s\n", props.deviceName);
                familyIndex = getGraphicsQueueFamily(pdarr[i], scratch);
                physicalDevice = pdarr[i];
                break;
		    }
        }
        if(physicalDevice == VK_NULL_HANDLE) { //TODO: robust device selection
            fprintf(stderr, "Failed to find a discrete GPU supporting rasterization.\n");
            osfail();
        }
    }

    //Logical Device
    VkDevice device = createDevice(physicalDevice, familyIndex);
    assert(device);

    GLFWwindow* window = glfwCreateWindow(1024, 768, "year", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        osfail();
    }

    VkSurfaceKHR surface = createSurface(instance, window);
    assert(surface);

    VkBool32 presentSupported = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface, &presentSupported));
    assert(presentSupported);

    i32 windowWidth = 0, windowHeight = 0;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    VkFormat swapchainFormat = getSwapchainFormat(physicalDevice, surface, mem);

	VkSwapchainKHR swapchain = createSwapchain(device, surface, familyIndex, swapchainFormat, windowWidth, windowHeight);
	assert(swapchain);

    VkQueue queue = 0;
	vkGetDeviceQueue(device, familyIndex, 0, &queue);

	VkRenderPass renderPass = createRenderPass(device, swapchainFormat);
	assert(renderPass);

    VkShaderModule triangleVS = loadShader(device, "shaders/vert.spv", mem);
	assert(triangleVS);

	VkShaderModule triangleFS = loadShader(device, "shaders/frag.spv", mem);
	assert(triangleFS);

    // TODO: this is critical for performance!
	VkPipelineCache pipelineCache = 0;

	VkPipelineLayout triangleLayout = createPipelineLayout(device);
	assert(triangleLayout);

	VkPipeline trianglePipeline = createGraphicsPipeline(device, pipelineCache, renderPass, triangleVS, triangleFS, triangleLayout);
	assert(trianglePipeline);

    u32 swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, NULL);
    VkImage *swapchainImages = new(&mem, VkImage, swapchainImageCount, NOZERO);
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages));

    VkImageView *swapchainImageViews = new(&mem, VkImageView, swapchainImageCount, NOZERO);
    for (u32 i = 0; i < swapchainImageCount; i++) {
        swapchainImageViews[i] = createImageView(device, swapchainImages[i], swapchainFormat);
        assert(swapchainImageViews[i]);
    }

    VkFramebuffer *swapchainFramebuffers = new(&mem, VkFramebuffer, swapchainImageCount, NOZERO);
	for (uint32_t i = 0; i < swapchainImageCount; ++i)
	{
		swapchainFramebuffers[i] = createFramebuffer(device, renderPass, swapchainImageViews[i], windowWidth, windowHeight);
		assert(swapchainFramebuffers[i]);
	}
    
    VkCommandPool commandPool = createCommandPool(device, familyIndex);
	assert(commandPool);

    VkCommandBufferAllocateInfo allocateInfo = { .sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocateInfo.commandPool = commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = 0;
	VK_CHECK(vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer));

 	VkSemaphore acquireSemaphore = createSemaphore(device);
	assert(acquireSemaphore);

	VkSemaphore releaseSemaphore = createSemaphore(device);
	assert(releaseSemaphore);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        u32 imageIndex = 0;
        VK_CHECK(vkAcquireNextImageKHR(device, swapchain, ~0ull, acquireSemaphore, VK_NULL_HANDLE, &imageIndex));

        VK_CHECK(vkResetCommandPool(device, commandPool, 0));

		VkCommandBufferBeginInfo beginInfo = { .sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		VkClearColorValue color = {{ (48.f / 255.f), (10.f / 255.f), (36.f / 255.f), 1.0f }};
		VkClearValue clearColor = { color };

		VkRenderPassBeginInfo passBeginInfo = { .sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		passBeginInfo.renderPass = renderPass;
		passBeginInfo.framebuffer = swapchainFramebuffers[imageIndex];
		passBeginInfo.renderArea.extent.width = windowWidth;
		passBeginInfo.renderArea.extent.height = windowHeight;
		passBeginInfo.clearValueCount = 1;
		passBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = { 0, (f32)windowHeight, (f32)windowWidth, -(f32)windowHeight, 0.0f, 1.0f };
		VkRect2D scissor = { {0, 0}, {(u32)windowWidth, (u32)windowHeight} };

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		VK_CHECK(vkEndCommandBuffer(commandBuffer));

		VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo = { .sType=VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &acquireSemaphore;
		submitInfo.pWaitDstStageMask = &submitStageMask;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &releaseSemaphore;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

		VkPresentInfoKHR presentInfo = { .sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &releaseSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &imageIndex;

		VK_CHECK(vkQueuePresentKHR(queue, &presentInfo));

		VK_CHECK(vkDeviceWaitIdle(device));
    }

    //NOTE: memory, I don't think there is a point in destroying these if we are exiting without running the sanitizer
    if (enableValidationsLayers) {
        //Aquire function to destroy debug mess
        PFN_vkDestroyDebugUtilsMessengerEXT funcDestroyDebugMess = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (funcDestroyDebugMess == NULL) {
            fprintf(stderr, "Failed to aquire function to destroy Debug messenger.\n");
            osfail();
        }
        vkDestroySemaphore(device, releaseSemaphore, NULL);
        vkDestroySemaphore(device, acquireSemaphore, NULL);
        vkDestroyCommandPool(device,commandPool, NULL);
        for (uint32_t i = 0; i < swapchainImageCount; ++i) {
            vkDestroyFramebuffer(device, swapchainFramebuffers[i], NULL);
	    }
        for (uint32_t i = 0; i < swapchainImageCount; ++i) {
            vkDestroyImageView(device, swapchainImageViews[i], NULL);
	    }
        vkDestroyPipeline(device, trianglePipeline, NULL);
        vkDestroyPipelineLayout(device, triangleLayout, NULL);
        vkDestroyPipelineCache(device, pipelineCache, NULL);
        vkDestroyShaderModule(device, triangleVS, NULL);
        vkDestroyShaderModule(device, triangleFS, NULL);
        vkDestroyRenderPass(device, renderPass, NULL);
        vkDestroySwapchainKHR(device, swapchain, NULL);
        vkDestroySurfaceKHR(instance, surface, NULL);
        vkDestroyDevice(device, NULL);

        funcDestroyDebugMess(instance, debugMessenger, NULL);
        vkDestroyInstance(instance, NULL);
        

        glfwDestroyWindow(window);
        glfwTerminate();
        freearena(mem);
    } 
    
    return 0;
}