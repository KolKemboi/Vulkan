#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 800;

// NOTE: debug macro
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance inst, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      inst, "vkCreateDebugUtilsMessengerEXT");
  if (func != NULL) {
    return func(inst, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
};

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != NULL) {
    func(instance, debugMessenger, pAllocator);
  }
}
// queue family

struct QueueFamilyIndices {
  std::optional<uint32_t> s_GraphicsFamily, s_PresentFamily;
  bool isComplete() {
    return s_GraphicsFamily.has_value() && s_PresentFamily.has_value();
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR s_Capabilities;
  std::vector<VkSurfaceFormatKHR> s_Formats;
  std::vector<VkPresentModeKHR> s_PresentModes;
};

class VulkanEngine {
public:
  // NOTE: to init the engine
  VulkanEngine() {
    initWindow();
    initVulkan();
  }
  void run() { mainLoop(); }
  // NOTE: Engine destructor, to destroy all instance vars
  ~VulkanEngine() { cleanUp(); }

private:
  GLFWwindow *m_Window;
  VkInstance m_Instance;
  VkDebugUtilsMessengerEXT m_DebugMessenger;

  VkSurfaceKHR m_Surface;
  VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
  VkDevice m_Device;
  VkQueue m_GraphicsQueue, m_PresentQueue;

private:
  // NOTE: user input
  void closeWindow(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_CAPS_LOCK) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, true);
    }
  }
  void initWindow() {
    glfwInit();
    if (enableValidationLayers) {
      std::cout << "Validation Layer Enabled" << std::endl;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Engine", NULL, NULL);
  }

  void initVulkan() {
    createInstance();
    setupDebugMessenger();

    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(this->m_Window)) {
      glfwPollEvents();
      closeWindow(this->m_Window);
    }
  }
  void cleanUp() {
    std::cout << "Cleaning Window" << std::endl;

    vkDestroyDevice(m_Device, NULL);
    vkDestroySurfaceKHR(m_Instance, m_Surface, NULL);

    glfwDestroyWindow(m_Window);
    this->m_Window = NULL;
    if (enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, NULL);
    }
    vkDestroyInstance(m_Instance, NULL);

    glfwTerminate();
  }
  // NOTE: VULKAN STUFF
  //

  void createInstance() {
    // NOTE: App info data, minimum vulkan support version etc
    //

    if (enableValidationLayers && !checkValidationLayerSupport()) {
      throw std::runtime_error(
          "ERROR::VALIDATION_LAYER_REQUESTED::NO_AVAILABLE_SUPPORT");
    }

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Engine",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    auto extensions = getReqExts();

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),

    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
      createInfo.enabledLayerCount =
          static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = NULL;
    }

    if (vkCreateInstance(&createInfo, NULL, &m_Instance) != VK_SUCCESS) {
      throw std::runtime_error("ERROR::FAILED_TO_SETUP_INSTANCE");
    } else {
      std::cout << "SUCCESS::INSTANCE_CREATED" << std::endl;
    }
  }

  std::vector<const char *> getReqExts() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions,
                                         glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
  }

  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo) {

    createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
    };
  }
  // NOTE: DEBUG LAYER stuff
  void setupDebugMessenger() {

    if (!enableValidationLayers) {
      return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, NULL,
                                     &m_DebugMessenger) != VK_SUCCESS) {
      throw std::runtime_error("ERROR::FAILED_TO_CREATE_DEBUG_MESSENGER");
    } else {
      std::cout << "SUCCESS::CREATED_DEBUG_MESSENGER" << std::endl;
    }
  }

  bool checkValidationLayerSupport() {

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers) {
      bool layerFound = false;
      for (const auto &layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          std::cout << layerName << std::endl;
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

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData) {
    std::cerr << "Validation Later: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
  }
  // NOTE: logical device and physical

  void createSurface() {
    if (glfwCreateWindowSurface(m_Instance, m_Window, NULL, &m_Surface) !=
        VK_SUCCESS) {
      throw std::runtime_error("ERROR::FAILED_TO_CREATE_WINDOW_SURFACE");
    } else {
      std::cout << "SUCCESS::CREATED_WINDOW_SURFACE" << std::endl;
    }
  }

  void pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, NULL);

    if (deviceCount == 0) {
      throw std::runtime_error("ERROR::FAILED_TO_FIND_GPU_WITH_VK_SUPPORT");
    } else {
      std::cout << "SUCCESS::NUMBER_OF_DEVICES_WITH_VK_SUPPORT::" << deviceCount
                << std::endl;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    for (const auto &device : devices) {
      if (isDeviceSuitable(device)) {
        m_PhysicalDevice = device;
        break;
      }
    }
    if (m_PhysicalDevice == VK_NULL_HANDLE) {
      throw std::runtime_error("ERROR::FAILED_TO_FIND_A_SUITABLE_GPU");
    } else {
      std::cout << "SUCCESS::FOUND_SUITABLE_DEVICE" << std::endl;
    }
  }
  bool isDeviceSuitable(VkPhysicalDevice dev) {
    QueueFamilyIndices indices = findQueueFamilies(dev);

    bool extensionsSupported = checkDeviceExtensionSupport(dev);

    bool swapChainAdequate = false;

    if (extensionsSupported) {
      SwapChainSupportDetails swapShainSupport = querySwapChainSupport(dev);
      swapChainAdequate = !swapShainSupport.s_Formats.empty() &&
                          !swapShainSupport.s_PresentModes.empty();
    }
    return indices.isComplete() && extensionsSupported && swapChainAdequate;
  }

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice dev) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, NULL);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies) {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.s_GraphicsFamily = i;
      }

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, m_Surface, &presentSupport);

      if (presentSupport) {
        indices.s_PresentFamily = i;
      }
      if (indices.isComplete()) {
        break;
      }
      i++;
    }
    return indices;
  }

  bool checkDeviceExtensionSupport(VkPhysicalDevice dev) {
    uint32_t extensionCount;

    vkEnumerateDeviceExtensionProperties(dev, NULL, &extensionCount, NULL);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(dev, NULL, &extensionCount,
                                         availableExtensions.data());

    std::set<std::string> reqExts(deviceExtensions.begin(),
                                  deviceExtensions.end());
    for (const auto &extensions : availableExtensions) {
      reqExts.erase(extensions.extensionName);
    }
    return reqExts.empty();
  }

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice dev) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, m_Surface,
                                              &details.s_Capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(dev, m_Surface, &formatCount, NULL);
    if (formatCount != 0) {
      details.s_Formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(dev, m_Surface, &formatCount,
                                           details.s_Formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(dev, m_Surface, &presentModeCount,
                                              NULL);
    if (presentModeCount != 0) {
      details.s_PresentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          dev, m_Surface, &presentModeCount, details.s_PresentModes.data());
    }
    return details;
  }

  void createLogicalDevice() {}
};

int main() {
  std::unique_ptr<VulkanEngine> engine = std::make_unique<VulkanEngine>();
  try {
    engine->run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
