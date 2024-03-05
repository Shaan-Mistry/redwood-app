# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("redwood" LANGUAGES CXX)
# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.


get_filename_component(SHADERC_SRC
        ${ANDROID_NDK}/sources/third_party/shaderc
        ABSOLUTE)
add_library(shaderc_lib STATIC IMPORTED)
set_target_properties(shaderc_lib PROPERTIES IMPORTED_LOCATION
        ${SHADERC_SRC}/libs/${ANDROID_STL}/${ANDROID_ABI}/libshaderc.a)
include_directories(${SHADERC_SRC}/include)


# Add External Dependencies
include(FetchContent)
FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG main
)
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG master
)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES 1)
FetchContent_MakeAvailable(benchmark glm)

# Include directories for the main executable
file(GLOB SOURCES
        pipeline/benchmarks/*
        pipeline/include*.h
        pipeline/kernels/*
        )
# Include directories for the kernel headers
add_library(${CMAKE_PROJECT_NAME} SHARED
        # List all C/C++ source files with relative paths to this CMakeLists.txt.
        native-lib.cpp
        benchmark_cpu.cpp
        benchmark_gpu.cpp
        easyvk/easyvk.cpp
        ${SOURCES}
        )

# Include Vulkan headers from the NDK


# Include directories for header files
include_directories(${CMAKE_PROJECT_NAME} PUBLIC pipeline/include)# Include directory for header files
# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
        # List libraries link to the target library
        android
        benchmark::benchmark
        vulkan
        glm
        shaderc_lib
        log)




