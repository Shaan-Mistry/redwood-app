#include <iostream>
#include <jni.h>
#include <assert.h>
#include <shaderc/shaderc.hpp>

#include "easyvk/easyvk.h"

//const int size = 1024 * 16;

const int gridSize = 32;
const int workGroupSize = 32;
const int size = 4090 * gridSize;
jintArray runShader(JNIEnv *env ,std::vector<uint32_t> spvCode) {
// Initialize instance.
    auto instance = easyvk::Instance(true);
    // Get list of available physical devices.
    auto physicalDevices = instance.physicalDevices();
    // Create device from first physical device.
    auto device = easyvk::Device(instance, physicalDevices.at(0));
    std::cout << "Using device: " << device.properties.deviceName << "\n";

    auto numIters = 1;
    for (int n = 0; n < numIters; n++) {
        // Define the buffers to use in the kernel.
        auto a = easyvk::Buffer(device, size, sizeof(uint32_t));
        auto b = easyvk::Buffer(device, size, sizeof(uint32_t));
        auto c = easyvk::Buffer(device, size, sizeof(uint32_t));

        // Write initial values to the buffers.
        for (int i = 0; i < size; i++) {
            // The buffer provides an untyped view of the memory, so you must specify
            // the type when using the load/store methods.
            a.store<uint32_t>(i, i);
            b.store<uint32_t>(i, i + 1);
        }
        c.clear();
        std::vector<easyvk::Buffer> bufs = {a, b, c};

        auto program = easyvk::Program(device, spvCode, bufs);

        program.setWorkgroups(size);
        program.setWorkgroupSize(1);

        // Run the kernel.
        program.initialize("litmus_test");

        program.run();

        // Check the output.
        for (int i = 0; i < size; i++) {
            // std::cout << "c[" << i << "]: " << c.load(i) << "\n";
            assert(c.load<uint32_t>(i) == a.load<uint32_t>(i) + b.load<uint32_t>(i));
        }

        // Cleanup.
        program.teardown();
        a.teardown();
        b.teardown();
        c.teardown();
    }

    device.teardown();
    instance.teardown();
    __android_log_print(ANDROID_LOG_INFO, "EasyVK", "Finished!!");
    return nullptr;
    //return resultArray;
}




