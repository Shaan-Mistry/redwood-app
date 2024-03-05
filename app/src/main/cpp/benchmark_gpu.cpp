#include <iostream>
#include <jni.h>
#include <shaderc/shaderc.hpp>

#include "easyvk/easyvk.h"


jintArray runShader(JNIEnv *env ,std::vector<uint32_t> spvCode) {
    int gridSize = 32;
    int workGroupSize = 32;
    int size = 4090 * gridSize;

    jintArray resultArray = env->NewIntArray(size);

    // Initialize instance
    auto instance = easyvk::Instance(false);

    // Get list of available physical devices.
    auto physicalDevices = instance.physicalDevices();

    // Create device from first physical device.
    auto device = easyvk::Device(instance, physicalDevices.at(0));

    __android_log_print(ANDROID_LOG_INFO, "EasyVK", "Using device %s", device.properties.deviceName);

    auto a = easyvk::Buffer(device, size);
    auto b = easyvk::Buffer(device, size);
    auto c = easyvk::Buffer(device, size);
    auto canary = easyvk::Buffer(device, 1);

    long iterations = 0;

    // Write initial values to the buffers.
    for (int i = 0; i < size; i++) {
        a.store(i, i);
        b.store(i, i);
        c.store(i, i);
    }

    std::vector<easyvk::Buffer> bufs = {a, b, c, canary};

    auto program = easyvk::Program(device, spvCode, bufs);

    // Dispatch 4 work groups of size 1 to carry out the work.
    program.setWorkgroups(gridSize);
    program.setWorkgroupSize(workGroupSize);

    // Run the kernel.
    program.initialize("covertListener");

    program.run();

    jint *elements = env->GetIntArrayElements(resultArray, nullptr); // here

    //int nonZeros = 0;
    for(int i = 0; i < size; i++) {
        unsigned v = c.load(i);
        elements[i] = v;
    }

    program.teardown();

    // Cleanup.
    a.teardown();
    b.teardown();
    c.teardown();
    device.teardown();
    instance.teardown();

    env->ReleaseIntArrayElements(resultArray, elements, 0); // here

    return resultArray;
}




