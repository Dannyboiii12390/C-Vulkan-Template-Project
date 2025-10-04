#pragma once

// Ensure this header is included only once to avoid multiple definitions
#include <vulkan/vulkan.h>

#ifdef DEBUG
#define ASSERT(x) if(!(x)) __debugbreak();



#else
#define ASSERT(x) x


#endif

// --- Debug Utils Functions ---
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT * pCreateInfo,
    const VkAllocationCallbacks * pAllocator,
    VkDebugUtilsMessengerEXT * pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks * pAllocator);