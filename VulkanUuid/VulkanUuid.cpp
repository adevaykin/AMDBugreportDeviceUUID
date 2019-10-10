#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/gl.h>

#define GLFW_DLL
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <iomanip>
#include <iostream>
#include <vector>
#include <array>
#include <optional>

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

using OpenGlUuid = std::array<GLubyte, GL_UUID_SIZE_EXT>;

template <unsigned N>
OpenGlUuid makeUuid(const uint8_t(&byteArray)[N])
{
	static_assert(N == GL_UUID_SIZE_EXT);
	OpenGlUuid returnValue;
	memcpy(returnValue.data(), byteArray, returnValue.size());
	return returnValue;
}

void printUuid(const OpenGlUuid& id) {
	std::cout << "UUID: ";
	for (size_t i = 0; i < GL_UUID_SIZE_EXT; i++) {
		std::cout << std::setfill('0') << std::setw(2) << std::hex << (int)id[i] << " ";
	}
	std::cout << std::endl;
}

// Get OpenGL device UUIDs
std::vector<OpenGlUuid> getOpenGLContextDeviceUuids()
{
	std::vector<OpenGlUuid> resultVector;
	if (GLEW_EXT_memory_object)
	{
		GLint numDevices = 0;
		glGetIntegerv(GL_NUM_DEVICE_UUIDS_EXT, &numDevices);

		for (auto i = 0; i < numDevices; i++)
		{
			OpenGlUuid currentUuid;
			glGetUnsignedBytei_vEXT(GL_DEVICE_UUID_EXT, GLuint(i), &currentUuid[0]);
			resultVector.push_back(currentUuid);
		}
	}
	return resultVector;
}

namespace vulkan {
	std::optional<vk::PhysicalDeviceIDProperties> getIdProperties( vk::DispatchLoaderDynamic& dldInstance, const vk::PhysicalDevice& device)
	{
		if (dldInstance.vkGetPhysicalDeviceProperties2)
		{
			vk::StructureChain<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceIDProperties> deviceProperties2;

			dldInstance.vkGetPhysicalDeviceProperties2(device, &(deviceProperties2.get<vk::PhysicalDeviceProperties2>().operator VkPhysicalDeviceProperties2 & ()));

			return deviceProperties2.get<vk::PhysicalDeviceIDProperties>();
		}

		return std::nullopt;
	}

	void printVulkanDevicesInfo()
	{
		vk::ApplicationInfo appInfo("Vulkan-OpenGL UUID Test", 1, nullptr, 0, VK_API_VERSION_1_1);
		auto requiredValidationLayers = std::vector<char*>();
		auto requiredExtensions = std::vector<char*>();
		vk::InstanceCreateInfo createInfo(vk::InstanceCreateFlags(), &appInfo,
			uint32_t(requiredValidationLayers.size()), requiredValidationLayers.data(),
			uint32_t(requiredExtensions.size()), requiredExtensions.data()
		);

		auto instance = vk::createInstanceUnique(createInfo);
		auto dldInstance = vk::DispatchLoaderDynamic(instance.get());

		const auto devices = instance->enumeratePhysicalDevices(dldInstance);
		if (devices.empty())
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

		bool foundSuitableDevice = false;
		for (const auto& device : devices)
		{
			const auto idProperties = getIdProperties(dldInstance, device);
			if (idProperties.has_value())
			{
				const auto deviceUuid = makeUuid(idProperties->deviceUUID);
				std::cout << "Vulkan Device ";
				printUuid(deviceUuid);
			}
			else
			{
				std::cerr << "Cannot query vulkan device UUID";
			}
		}
	}
}

namespace opengl {
	void printGlDevicesInfo() {
		const auto glversion = glGetString(GL_VERSION);
		if (glversion != nullptr)
		{
			std::cout << "GLVersion: " << glversion << std::endl;
		}
		const auto glrenderer = glGetString(GL_RENDERER);
		if (glversion != nullptr)
		{
			std::cout << "GLRenderer: " << glrenderer << std::endl;
		}

		for (auto device : getOpenGLContextDeviceUuids()) {
			std::cout << "OpenGL Device ";
			printUuid(device);
		}
	}
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glewInit();

	// Here are the problematic calls. Both functions supposed to return the same UUID for the same device.
	opengl::printGlDevicesInfo();
	vulkan::printVulkanDevicesInfo();


	while (!glfwWindowShouldClose(window))
	{
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}
