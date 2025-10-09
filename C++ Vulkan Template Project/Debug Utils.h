#pragma once

// Ensure this header is included only once to avoid multiple definitions
#include <vulkan/vulkan.h>

#ifdef DEBUG
#define ASSERT(x) if(!(x)) __debugbreak()
#define ASSERT_MSG(x, msg) if(!(x)) { std::cout << msg << std::endl; __debugbreak();}
#define LOG(msg) std::cout << msg << std::endl

#else
#define ASSERT(x) x
#define ASSERT_MSG(x, msg) x
#define LOG(msg)


#endif

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif



// --- Debug Utils Functions ---
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT * pCreateInfo,
    const VkAllocationCallbacks * pAllocator,
    VkDebugUtilsMessengerEXT * pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks * pAllocator);