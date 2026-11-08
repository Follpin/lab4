#pragma once

#include <cstddef>
#include <shared_mutex>
#include <string>

class OptimizedData
{
public:
    static constexpr std::size_t M = 3;

    OptimizedData() = default;
    OptimizedData(const OptimizedData &) = delete;
    OptimizedData &operator=(const OptimizedData &) = delete;
    OptimizedData(OptimizedData &&) = delete;
    OptimizedData &operator=(OptimizedData &&) = delete;
    ~OptimizedData() = default;

    int get(std::size_t index) const;
    void set(std::size_t index, int value);
    operator std::string() const;

private:
    alignas(64) int fields_[M]{0, 0, 0};
    alignas(64) mutable std::shared_mutex mtx0_;
    alignas(64) mutable std::shared_mutex mtx1_;
    alignas(64) mutable std::shared_mutex mtx2_;
};
