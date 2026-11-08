#include "ActionFile.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>

std::string scenarioPrefix(Scenario scenario)
{
    switch (scenario)
    {
    case Scenario::Variant5:
        return "a_variant5";
    case Scenario::Equal:
        return "b_equal";
    case Scenario::CustomMismatch:
        return "c_custom";
    }
    return "unknown";
}

std::string scenarioTitle(Scenario scenario)
{
    switch (scenario)
    {
    case Scenario::Variant5:
        return "a) Variant 5 (r0=9%, w0=1%, r1=40%, w1=10%, r2=10%, w2=10%, str=20%)";
    case Scenario::Equal:
        return "b) Equal frequencies (all 7 operations at 1/7 ~= 14.29%)";
    case Scenario::CustomMismatch:
        return "c) Unequal mismatch (str=50%, w0=30%, w2=9%, w1=5%, r2=3%, r0=2%, r1=1%)";
    }
    return "";
}

std::string buildFilePath(Scenario scenario, int numThreads, int threadIndex)
{
    return std::format("data/{}_{}t_{}.txt", scenarioPrefix(scenario), numThreads, threadIndex);
}

static void appendCommandForScenario(std::string &buffer, Scenario scenario, std::mt19937 &engine)
{
    if (scenario == Scenario::Equal)
    {
        std::uniform_int_distribution<int> dist(0, 6);
        int r = dist(engine);
        if (r == 0)
            buffer += "read 0\n";
        else if (r == 1)
            buffer += "write 0 1\n";
        else if (r == 2)
            buffer += "read 1\n";
        else if (r == 3)
            buffer += "write 1 1\n";
        else if (r == 4)
            buffer += "read 2\n";
        else if (r == 5)
            buffer += "write 2 1\n";
        else
            buffer += "string\n";
        return;
    }

    std::uniform_int_distribution<int> dist(0, 99);
    int p = dist(engine);

    if (scenario == Scenario::Variant5)
    {
        // Variant 5:
        // Field 0: read 9% (0..8), write 1% (9)
        // Field 1: read 40% (10..49), write 10% (50..59)
        // Field 2: read 10% (60..69), write 10% (70..79)
        // string: 20% (80..99)
        if (p < 9)
            buffer += "read 0\n";
        else if (p < 10)
            buffer += "write 0 1\n";
        else if (p < 50)
            buffer += "read 1\n";
        else if (p < 60)
            buffer += "write 1 1\n";
        else if (p < 70)
            buffer += "read 2\n";
        else if (p < 80)
            buffer += "write 2 1\n";
        else
            buffer += "string\n";
    }
    else
    {   
        // Scenario c: strong mismatch and all 7 frequencies are pairwise unequal:
        // string:  50% (0..49)
        // write 0: 30% (50..79)
        // write 2:  9% (80..88)
        // write 1:  5% (89..93)
        // read 2:   3% (94..96)
        // read 0:   2% (97..98)
        // read 1:   1% (99)
        if (p < 50)
            buffer += "string\n";
        else if (p < 80)
            buffer += "write 0 1\n";
        else if (p < 89)
            buffer += "write 2 1\n";
        else if (p < 94)
            buffer += "write 1 1\n";
        else if (p < 97)
            buffer += "read 2\n";
        else if (p < 99)
            buffer += "read 0\n";
        else
            buffer += "read 1\n";
    }
}

void generateActionFile(const std::string &filepath, Scenario scenario, std::size_t operationCount)
{
    std::filesystem::create_directories("data");

    std::random_device rd;
    std::seed_seq seeds{rd(), rd(), rd(), rd(), rd(), rd()};
    std::mt19937 engine(seeds);

    std::ofstream out(filepath, std::ios::binary);
    if (!out)
    {
        throw std::runtime_error("Cannot open file for writing: " + filepath);
    }

    std::string buffer;
    buffer.reserve(65536);

    for (std::size_t i = 0; i < operationCount; ++i)
    {
        appendCommandForScenario(buffer, scenario, engine);
        if (buffer.size() >= 60000)
        {
            out.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
            buffer.clear();
        }
    }
    if (!buffer.empty())
    {
        out.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    }
}

static void ensureScenarioFiles(Scenario scenario, std::size_t totalOperations)
{
    for (int numThreads = 1; numThreads <= 3; ++numThreads)
    {
        std::size_t opsPerThread = totalOperations / static_cast<std::size_t>(numThreads);
        for (int t = 0; t < numThreads; ++t)
        {
            std::string path = buildFilePath(scenario, numThreads, t);
            bool needGenerate = !std::filesystem::exists(path);
            if (!needGenerate)
            {
                std::string existing = loadFileToString(path);
                if (countLinesInString(existing) != opsPerThread)
                {
                    needGenerate = true;
                }
            }
            if (needGenerate)
            {
                std::cout << std::format("  Generating file {} ({} operations)...\n", path, opsPerThread);
                generateActionFile(path, scenario, opsPerThread);
            }
        }
    }
}

void ensureAllActionFiles(std::size_t totalOperations)
{
    ensureScenarioFiles(Scenario::Variant5, totalOperations);
    ensureScenarioFiles(Scenario::Equal, totalOperations);
    ensureScenarioFiles(Scenario::CustomMismatch, totalOperations);
}

std::string loadFileToString(const std::string &filepath)
{
    std::ifstream in(filepath, std::ios::binary | std::ios::ate);
    if (!in)
    {
        throw std::runtime_error("Cannot open file for reading: " + filepath);
    }
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);

    std::string content(static_cast<std::size_t>(size), '\0');
    if (size > 0)
    {
        in.read(content.data(), size);
    }
    return content;
}

std::size_t countLinesInString(const std::string &content)
{
    std::size_t lines = 0;
    for (char ch : content)
    {
        if (ch == '\n')
        {
            ++lines;
        }
    }
    return lines;
}

void executeActionsFromString(const std::string &content, OptimizedData &data)
{
    auto it = content.cbegin();
    auto end = content.cend();

    while (it != end)
    {
        char cmd = *it;
        if (cmd == 'r')
        {
            it += 5;
            std::size_t fieldIdx = static_cast<std::size_t>(*it - '0');
            (void)data.get(fieldIdx);
        }
        else if (cmd == 'w')
        {
            it += 6;
            std::size_t fieldIdx = static_cast<std::size_t>(*it - '0');
            it += 2;
            int val = 0;
            while (it != end && *it >= '0' && *it <= '9')
            {
                val = val * 10 + (*it - '0');
                ++it;
            }
            data.set(fieldIdx, val);
        }
        else if (cmd == 's')
        {
            (void)static_cast<std::string>(data);
        }

        while (it != end && *it != '\n')
        {
            ++it;
        }
        if (it != end && *it == '\n')
        {
            ++it;
        }
    }
}
