#include <jni.h>
#include <string>
#include <android/log.h>
#include <benchmark/benchmark.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <thread>
#include <errno.h>
#include <sched.h>
#include <fstream>
#include <shaderc/shaderc.hpp>
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>

#include "benchmark_cpu.hpp"
#include "benchmark_gpu.hpp"
#include "easyvk/easyvk.h"



//std::vector<uint32_t> spvCode = {119734787, 66304, 1376256, 64, 0, 131089, 1, 131089, 10, 524298, 1599492179, 1599227979, 1601073006, 1634559347, 1667855470, 1718511967, 111, 655371, 45, 1399746382, 1851878757, 778266996, 1886612547, 1717916278, 1952671084, 778989417, 53, 196622, 0, 1, 458767, 5, 26, 1836345708, 1952412533, 7631717, 7, 196611, 3, 200, 327687, 46, 1836345708, 1952412533, 7631717, 327687, 47, 1801412384, 1701737061, 108, 196615, 50, 97, 196615, 53, 98, 196615, 57, 99, 327752, 3, 0, 35, 0, 196679, 3, 2, 262215, 7, 11, 28, 262215, 11, 11, 25, 262215, 14, 6, 4, 327752, 15, 0, 35, 0, 196679, 15, 2, 262215, 19, 6, 8, 327752, 20, 0, 35, 0, 196679, 20, 2, 262215, 17, 34, 0, 262215, 17, 33, 0, 262215, 22, 34, 0, 262215, 22, 33, 1, 262215, 23, 34, 0, 262215, 23, 33, 2, 196679, 43, 42, 262215, 8, 1, 0, 262215, 9, 1, 1, 262215, 10, 1, 2, 262165, 1, 32, 0, 262167, 2, 1, 3, 196638, 3, 2, 262176, 4, 9, 3, 262176, 6, 1, 2, 262194, 1, 8, 1, 262194, 1, 9, 1, 262194, 1, 10, 1, 393267, 2, 11, 8, 9, 10, 262176, 12, 6, 2, 196637, 14, 1, 196638, 15, 14, 262176, 16, 12, 15, 196630, 18, 64, 196637, 19, 18, 196638, 20, 19, 262176, 21, 12, 20, 131091, 24, 196641, 25, 24, 262176, 28, 1, 1, 262187, 1, 29, 0, 262176, 32, 9, 1, 262176, 36, 12, 1, 262176, 40, 12, 18, 262187, 1, 48, 3, 262187, 1, 55, 1, 262187, 1, 59, 2, 262187, 1, 61, 12, 262203, 4, 5, 9, 262203, 6, 7, 1, 327739, 12, 13, 6, 11, 262203, 16, 17, 12, 262203, 21, 22, 12, 262203, 21, 23, 12, 327734, 24, 26, 0, 25, 131320, 27, 327745, 28, 30, 7, 29, 262205, 1, 31, 30, 393281, 32, 33, 5, 29, 29, 262205, 1, 34, 33, 327808, 1, 35, 34, 31, 393281, 36, 37, 17, 29, 35, 262205, 1, 38, 37, 262256, 18, 39, 38, 393281, 40, 41, 22, 29, 35, 262205, 18, 42, 41, 327809, 18, 43, 42, 39, 393281, 40, 44, 23, 29, 35, 196670, 44, 43, 65789, 65592, 458764, 24, 62, 45, 18, 29, 61, 655372, 24, 49, 45, 1, 26, 46, 48, 29, 47, 393228, 24, 51, 45, 2, 50, 655372, 24, 52, 45, 3, 49, 29, 29, 29, 51, 393228, 24, 54, 45, 2, 53, 655372, 24, 56, 45, 3, 49, 55, 29, 55, 54, 393228, 24, 58, 45, 2, 57, 655372, 24, 60, 45, 3, 49, 59, 29, 59, 58, 524300, 24, 63, 45, 12, 29, 55, 59};

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_redwood_MainActivity_runBenchmarkCPU(JNIEnv *env, jobject thiz, jstring benchmark) {
    std::string bm = env->GetStringUTFChars(benchmark, 0);
    std::string bmResult = runCPUBenchmark(bm);
    return env->NewStringUTF(bmResult.c_str());
}

std::vector<uint32_t> compile_file(const std::string& name, shaderc_shader_kind kind, const std::string& data) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    // Like -DMY_DEFINE=1
    options.AddMacroDefinition("MY_DEFINE", "1");

    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
            data.c_str(), data.size(), kind, name.c_str(), options);

    if (module.GetCompilationStatus() !=
        shaderc_compilation_status_success) {
        __android_log_print(ANDROID_LOG_DEBUG, "spirv", "%s", module.GetErrorMessage().c_str());
        std::cerr << module.GetErrorMessage();
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, "spirv", "successful compilation");
    }
    std::vector<uint32_t> result(module.cbegin(), module.cend());
    return result;
}

std::vector<uint32_t> LoadBinaryFileToVector(const char *file_path,
                                             AAssetManager *assetManager) {
    std::vector<uint32_t> file_content;
    assert(assetManager);
    AAsset *file =
            AAssetManager_open(assetManager, file_path, AASSET_MODE_BUFFER);
    size_t file_length = AAsset_getLength(file);

    if (!file) {
        __android_log_print(ANDROID_LOG_DEBUG, "spirv", "Failed to load asset");
    }

    file_content.resize((file_length+3)/4);

    AAsset_read(file, file_content.data(), file_length);
    AAsset_close(file);
    return file_content;
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_redwood_MainActivity_runBenchmarkGPU(JNIEnv *env, jobject thiz, jobject assetManager) {
//    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
//    auto spvCode = LoadBinaryFileToVector("stripped.spv", mgr);
//    std::string vertexShaderSource = R"glsl(
//                            #version 450
//
//                            layout(location = 0) in vec3 inPosition;
//
//                            void main() {
//                                gl_Position = vec4(inPosition, 1.0);
//                            }
//                            )glsl";
//    auto spvCode = compile_file("my shader", shaderc_vertex_shader, vertexShaderSource);
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    auto spvCode = LoadBinaryFileToVector("stripped.spv", mgr);
    jintArray result = runShader(env, spvCode);
    return env->NewStringUTF("finished!");
}

class ThreadAffinitySetter {
public:
  static int stickThisThreadToCore(int core_id) {
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (core_id < 0 || core_id >= num_cores)
      return EINVAL;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t current_thread = pthread_self();

    pid_t pid = gettid();
    return sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
  }
};

void threadFunction(int core_id) {
  // Set this thread's affinity to the specified core
  int result = ThreadAffinitySetter::stickThisThreadToCore(core_id);
  if (result == 0) {
      __android_log_print(ANDROID_LOG_ERROR, "Pthread", "Thread pinned to core %d", core_id);
  } else {
      __android_log_print(ANDROID_LOG_ERROR, "Pthread", "Error pinning thread to core %d: %s", core_id,
                          strerror(result));
  }
  // Perform thread's tasks here...
  runCPUBenchmark("k_ComputeMortonCode");

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_redwood_MainActivity_runPthreadTest(JNIEnv *env, jobject thiz) {
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

    __android_log_print(ANDROID_LOG_ERROR, "Pthread", "Number of cores %d", num_cores);

    //pthread_t thread;
    std::thread thread;
    for (int i = 0; i <= 7; i++) {
        thread = std::thread(threadFunction, i);
        //pthread_create(&thread, NULL, threadFunction, NULL);
        //pthread_join(thread, NULL);
        thread.join();
    }
}