#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
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

// NOTE: shouldn't I have a constructor and a destructor
// Engine
//	Engine()
//	~Engine()
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

  void initVulkan() { createInstance(); }

  void mainLoop() {
    while (!glfwWindowShouldClose(this->m_Window)) {
      glfwPollEvents();
      closeWindow(this->m_Window);
    }
  }
  void cleanUp() {
    std::cout << "Cleaning Window" << std::endl;
    glfwDestroyWindow(m_Window);
    this->m_Window = NULL;

    glfwTerminate();
  }
  // NOTE: VULKAN STUFF
  //

  void createInstance() {
    // NOTE: App info data, minimum vulkan support version etc
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

    createInfo.enabledLayerCount = 0;
    createInfo.pNext = NULL;

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
