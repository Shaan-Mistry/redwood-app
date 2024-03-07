#include <ostream>
#include <string>
#include <android/log.h>
#include <benchmark/benchmark.h>
#include <unordered_map>

// Struct for Printing Benchmark outputs
struct BenchmarkStream : std::ostream, std::streambuf
{
    BenchmarkStream() : std::ostream(this) {}
    std::vector<char> buffer;

    int overflow(int c)
    {
        buffer.push_back(c);
        return 0;
    }
};

std::string benchmarkResults = "";

// Map benchmark names
const std::unordered_map<std::string, std::string> benchmarkNames = {
        {"k_ComputeMortonCode","BM_Morton32"},
        {"k_SimpleRadixSort","BM_SimpleRadixSort"},
        {"k_CountUnique", "BM_Unique"},
        {"k_BuildRadixTree", "BM_RadixTree"},
        {"k_EdgeCount","BM_EdgeCount"},
        {"k_PartialSum","BM_PrefixSum"},
        {"k_MakeOctNodes","BM_MakeOctNodes"},
        {"k_LinkOctreeNodes","BM_LinkOctreeNodes"},
        {"All Benchmarks", "all"}
};

std::string runCPUBenchmark(std::string benchmark) {
    // Initialize Benchmark and set configuration flags
    int argc = 3;
    char* argv[3] = {"test", "--benchmark_repetitions=1", "--benchmark_report_aggregates_only=true"};
    // Set output format
    benchmark::CSVReporter reporter;
    BenchmarkStream outStream;
    reporter.SetOutputStream(&outStream);
    // Initialize Benchmark
    benchmark::Initialize(&argc, argv);
    // Get the Benchmark Name
    std::string bmName = benchmarkNames.at(benchmark);
    // Run the specified benchmark
    benchmark::RunSpecifiedBenchmarks(&reporter, nullptr, {bmName});
    // Convert benchmark Results to a string
    benchmarkResults = "";
    for (auto c : outStream.buffer) {
    benchmarkResults += c;
    }
    // Print Results to Logcat
    __android_log_print(ANDROID_LOG_INFO, "CPU Benchmark","%s", benchmarkResults.c_str());
    return benchmarkResults;
}
