#pragma once

#include "DataStructure.h"

#include <cstddef>
#include <string>

enum class Scenario
{
    Variant5,       // a) frequencies match variant 5 specification
    Equal,          // b) all 7 operations have equal frequencies (1/7 each)
    CustomMismatch  // c) frequencies strongly mismatch specification and are unequal
};

std::string scenarioPrefix(Scenario scenario);
std::string scenarioTitle(Scenario scenario);
std::string buildFilePath(Scenario scenario, int numThreads, int threadIndex);

void generateActionFile(const std::string &filepath, Scenario scenario, std::size_t operationCount);
void ensureAllActionFiles(std::size_t totalOperations);

std::string loadFileToString(const std::string &filepath);
std::size_t countLinesInString(const std::string &content);

void executeActionsFromString(const std::string &content, OptimizedData &data);
