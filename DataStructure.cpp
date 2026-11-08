#include "DataStructure.h"

#include <format>
#include <mutex>
#include <stdexcept>

int OptimizedData::get(std::size_t index) const
{
    switch (index)
    {
    case 0:
    {
        std::shared_lock<std::shared_mutex> lk(mtx0_);
        return fields_[0];
    }
    case 1:
    {
        std::shared_lock<std::shared_mutex> lk(mtx1_);
        return fields_[1];
    }
    case 2:
    {
        std::shared_lock<std::shared_mutex> lk(mtx2_);
        return fields_[2];
    }
    default:
        throw std::out_of_range("Field index out of range in OptimizedData::get");
    }
}

void OptimizedData::set(std::size_t index, int value)
{
    switch (index)
    {
    case 0:
    {
        std::lock_guard<std::shared_mutex> lk(mtx0_);
        fields_[0] = value;
        break;
    }
    case 1:
    {
        std::lock_guard<std::shared_mutex> lk(mtx1_);
        fields_[1] = value;
        break;
    }
    case 2:
    {
        std::lock_guard<std::shared_mutex> lk(mtx2_);
        fields_[2] = value;
        break;
    }
    default:
        throw std::out_of_range("Field index out of range in OptimizedData::set");
    }
}

OptimizedData::operator std::string() const
{
    int v0 = 0;
    int v1 = 0;
    int v2 = 0;
    {
        std::shared_lock<std::shared_mutex> lk0(mtx0_);
        std::shared_lock<std::shared_mutex> lk1(mtx1_);
        std::shared_lock<std::shared_mutex> lk2(mtx2_);
        v0 = fields_[0];
        v1 = fields_[1];
        v2 = fields_[2];
    }
    return std::format("({}, {}, {})", v0, v1, v2);
}
