#include "ActionFile.h"
#include "DataStructure.h"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

constexpr std::size_t TOTAL_OPERATIONS = 600000;
constexpr int REPEAT_RUNS = 5;

void runSelfTest()
{
    OptimizedData data;
    assert(data.get(0) == 0);
    assert(data.get(1) == 0);
    assert(data.get(2) == 0);
    assert(static_cast<std::string>(data) == "(0, 0, 0)");

    data.set(0, 10);
    data.set(1, 20);
    data.set(2, 30);
    assert(data.get(0) == 10);
    assert(data.get(1) == 20);
    assert(data.get(2) == 30);
    assert(static_cast<std::string>(data) == "(10, 20, 30)");
}

double benchmarkOneThread(Scenario scenario)
{
    std::string path0 = buildFilePath(scenario, 1, 0);
    // Preload file content into memory (before starting timer)
    std::string content0 = loadFileToString(path0);

    double totalMs = 0.0;

    for (int run = 0; run < REPEAT_RUNS; ++run)
    {
        OptimizedData data;

        auto tStart = std::chrono::steady_clock::now();
        {
            std::jthread t0(executeActionsFromString, std::cref(content0), std::ref(data));
        }
        auto tEnd = std::chrono::steady_clock::now();

        double ms = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        totalMs += ms;
    }

    double avgMs = totalMs / REPEAT_RUNS;
    std::cout << std::format("  1 thread  (1 x {} ops, file {}): avg time = {:7.2f} ms\n",
                             countLinesInString(content0), path0, avgMs);
    return avgMs;
}

double benchmarkTwoThreads(Scenario scenario)
{
    std::string path0 = buildFilePath(scenario, 2, 0);
    std::string path1 = buildFilePath(scenario, 2, 1);
    // Preload both files into memory (before starting timer)
    std::string content0 = loadFileToString(path0);
    std::string content1 = loadFileToString(path1);

    double totalMs = 0.0;

    for (int run = 0; run < REPEAT_RUNS; ++run)
    {
        OptimizedData data;

        auto tStart = std::chrono::steady_clock::now();
        {
            std::jthread t0(executeActionsFromString, std::cref(content0), std::ref(data));
            std::jthread t1(executeActionsFromString, std::cref(content1), std::ref(data));
        }
        auto tEnd = std::chrono::steady_clock::now();

        double ms = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        totalMs += ms;
    }

    double avgMs = totalMs / REPEAT_RUNS;
    std::cout << std::format("  2 threads (2 x {} ops, files {}, {}): avg time = {:7.2f} ms\n",
                             countLinesInString(content0), path0, path1, avgMs);
    return avgMs;
}

double benchmarkThreeThreads(Scenario scenario)
{
    std::string path0 = buildFilePath(scenario, 3, 0);
    std::string path1 = buildFilePath(scenario, 3, 1);
    std::string path2 = buildFilePath(scenario, 3, 2);
    // Preload all three files into memory (before starting timer)
    std::string content0 = loadFileToString(path0);
    std::string content1 = loadFileToString(path1);
    std::string content2 = loadFileToString(path2);

    double totalMs = 0.0;

    for (int run = 0; run < REPEAT_RUNS; ++run)
    {
        OptimizedData data;

        auto tStart = std::chrono::steady_clock::now();
        {
            std::jthread t0(executeActionsFromString, std::cref(content0), std::ref(data));
            std::jthread t1(executeActionsFromString, std::cref(content1), std::ref(data));
            std::jthread t2(executeActionsFromString, std::cref(content2), std::ref(data));
        }
        auto tEnd = std::chrono::steady_clock::now();

        double ms = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        totalMs += ms;
    }

    double avgMs = totalMs / REPEAT_RUNS;
    std::cout << std::format("  3 threads (3 x {} ops, files {}, {}, {}): avg time = {:7.2f} ms\n",
                             countLinesInString(content0), path0, path1, path2, avgMs);
    return avgMs;
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    runSelfTest();

    std::cout << "Lab 4 (Variant 5): Thread-Safe Data Structure (m = 3)\n";
    std::cout << std::format("Total operations: {} | Benchmark repeat runs: {}\n",
                             TOTAL_OPERATIONS, REPEAT_RUNS);
    std::cout << "Synchronization: 3 separate std::shared_mutex (mtx0_, mtx1_, mtx2_)\n\n";


    std::cout << "1) Checking / generating action files in data/ directory...\n";
    ensureAllActionFiles(TOTAL_OPERATIONS);
    std::cout << "All 18 action files are ready.\n\n";

    std::cout << "2) Benchmark execution (file read time is EXCLUDED):\n\n";

    std::cout << scenarioTitle(Scenario::Variant5) << "\n";
    double a1 = benchmarkOneThread(Scenario::Variant5);
    double a2 = benchmarkTwoThreads(Scenario::Variant5);
    double a3 = benchmarkThreeThreads(Scenario::Variant5);
    std::cout << "\n";

    std::cout << scenarioTitle(Scenario::Equal) << "\n";
    double b1 = benchmarkOneThread(Scenario::Equal);
    double b2 = benchmarkTwoThreads(Scenario::Equal);
    double b3 = benchmarkThreeThreads(Scenario::Equal);
    std::cout << "\n";

    std::cout << scenarioTitle(Scenario::CustomMismatch) << "\n";
    double c1 = benchmarkOneThread(Scenario::CustomMismatch);
    double c2 = benchmarkTwoThreads(Scenario::CustomMismatch);
    double c3 = benchmarkThreeThreads(Scenario::CustomMismatch);
    std::cout << "\n";


    std::cout << "Summary Table 3x3 (average execution time for 600000 operations, ms):\n";
    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << std::format("{:<42} | {:>10} | {:>10} | {:>10}\n",
                             "Action sequence (scenario)", "1 thread", "2 threads", "3 threads");
    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << std::format("{:<42} | {:>7.2f} ms | {:>7.2f} ms | {:>7.2f} ms\n",
                             "a) Variant 5 frequencies", a1, a2, a3);
    std::cout << std::format("{:<42} | {:>7.2f} ms | {:>7.2f} ms | {:>7.2f} ms\n",
                             "b) Equal frequencies (1/7 each)", b1, b2, b3);
    std::cout << std::format("{:<42} | {:>7.2f} ms | {:>7.2f} ms | {:>7.2f} ms\n",
                             "c) Unequal mismatch (str=50%, w=44%)", c1, c2, c3);

    return 0;
}
