#include "morton.hpp"

#include <pthread.h>
#include <thread>
#include <android/log.h>
#include <unistd.h>
#include <errno.h>
#include <string>

MortonT single_point_to_code_v2(
    float x, float y, float z, const float min_coord, const float range) {
  constexpr auto bit_scale = 1024.0f;

  x = (x - min_coord) / range;
  y = (y - min_coord) / range;
  z = (z - min_coord) / range;

  return m3D_e_magicbits(static_cast<CoordT>(x * bit_scale),
                         static_cast<CoordT>(y * bit_scale),
                         static_cast<CoordT>(z * bit_scale));
}

void morton32_to_xyz(glm::vec4* ret,
                     const MortonT code,
                     const float min_coord,
                     const float range) {
  constexpr auto bit_scale = 1024.0f;

  CoordT dec_raw_x[3];
  m3D_d_magicbits(code, dec_raw_x);

  const auto dec_x =
      (static_cast<float>(dec_raw_x[0]) / bit_scale) * range + min_coord;
  const auto dec_y =
      (static_cast<float>(dec_raw_x[1]) / bit_scale) * range + min_coord;
  const auto dec_z =
      (static_cast<float>(dec_raw_x[2]) / bit_scale) * range + min_coord;

  // vec4 result = {dec_x, dec_y, dec_z, 1.0f};
  // glm_vec4_copy(result, *ret);
  (*ret)[0] = dec_x;
  (*ret)[1] = dec_y;
  (*ret)[2] = dec_z;
  (*ret)[3] = 1.0f;
}

// functor for uint32_t, used in qsort
int compare_uint32_t(const void* a, const void* b) {
  const auto value1 = *static_cast<const unsigned int*>(a);
  const auto value2 = *static_cast<const unsigned int*>(b);

  if (value1 < value2) return -1;
  if (value1 > value2) return 1;
  return 0;
}

void k_ComputeMortonCode(const glm::vec4* data,
                         unsigned int* morton_keys,
                         const size_t start,
                         const size_t end,
                         const float min_coord,
                         const float range) {

  for (auto i = start; i < end; i++) {
    morton_keys[i] = single_point_to_code_v2(
        data[i][0], data[i][1], data[i][2], min_coord, range);
  }
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

// Define a struct to hold arguments for the thread function
struct ThreadArgs {
    const glm::vec4* data;
    unsigned int* morton_keys;
    size_t start;
    size_t end;
    float min_coord;
    float range;
    int core_id;
};

void* computeMortonCodeThread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int core_id = args->core_id;

    // Set this thread's affinity to the specified core
    int result = ThreadAffinitySetter::stickThisThreadToCore(core_id);
    if (result == 0) {
        __android_log_print(ANDROID_LOG_ERROR, "Pthread", "Thread pinned to core %d", core_id);
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "Pthread", "Error pinning thread to core %d: %s", core_id,
                            strerror(result));
    }
    // Perform thread's tasks here...
    k_ComputeMortonCode(args->data, args->morton_keys, args->start, args->end, args->min_coord, args->range);
    return NULL;
}

void  k_ComputeMortonCodeParallel(const glm::vec4* data,
                                  unsigned int* morton_keys,
                                  const size_t n,
                                  const float min_coord,
                                  const float range) {

    // Initialize the arguments structs
    ThreadArgs args1 = {data, morton_keys, 0, n / 2, min_coord, range, 6};
    ThreadArgs args2 = {data, morton_keys, n / 2, n,  min_coord, range, 7};
    //ThreadArgs args3 = {data, morton_keys, n /2, 3 * n / 4, min_coord, range, 5};
    //ThreadArgs args4 = {data, morton_keys, 3 * n / 4, n,  min_coord, range, 5};
    // Create a pthread_t variables
    pthread_t thread1, thread2, thread3, thread4;
    // Create the threads, passing &args as the argument
    pthread_create(&thread1, NULL, computeMortonCodeThread, (void*)&args1);
    pthread_create(&thread2, NULL, computeMortonCodeThread, (void*)&args2);
//    pthread_create(&thread3, NULL, computeMortonCodeThread, (void*)&args3);
//    pthread_create(&thread4, NULL, computeMortonCodeThread, (void*)&args4);
    // Run both threads in parallel
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
//    pthread_join(thread3, NULL);
//    pthread_join(thread4, NULL);
}