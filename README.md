# AMDBugreportDeviceUUID
Repro case for AMD bug report https://community.amd.com/thread/241599

## How to use
1. Open solution in Visual Studio (2019)
2. Change path to the valid VulkanSDK path in project "includes" and "libraries" paths
3. Compile and run

## Bug description

The application lists available OpenGL and Vulkan devices and prints their UUIDs to the terminal. UUID is expected to be equal for the same GPU in both OpenGL and Vulkan for interoperability purpose.

### Expected

OpenGL and Vulkan device UUIDs match.

### Actual

OpenGL and Vulkan device UUIDs don't match.
