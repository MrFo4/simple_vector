#pragma once

#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>

// Вспомогательный класс для работы с резервированием памяти
class ReserveProxy {
public:
    explicit ReserveProxy(size_t reserve_size)
        : reserve_size_(reserve_size) {}

    size_t GetReserveSize() const {
        return reserve_size_;
    }

private:
    size_t reserve_size_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) 
    : data_(size), size_(size), capacity_(size) {
    std::generate(data_.Get(), data_.Get() + size, []() { return Type(); });
}


    SimpleVector(size_t size, const Type& value)
        : data_(size), size_(size), capacity_(size) {
        std::fill(data_.Get(), data_.Get() + size, value);
    }

    SimpleVector(std::initializer_list<Type> init_list)
        : data_(init_list.size()), size_(init_list.size()), capacity_(init_list.size()) {
        std::copy(init_list.begin(), init_list.end(), data_.Get());
    }

    SimpleVector(const SimpleVector& other)
        : data_(other.capacity_), size_(other.size_) {
        std::copy(other.begin(), other.end(), data_.Get());
    }

    SimpleVector(SimpleVector&& other) noexcept
        : data_(std::move(other.data_)), size_(other.size_), capacity_(other.capacity_) {
        other.size_ = 0;
        other.capacity_ = 0;
    }

    explicit SimpleVector(ReserveProxy reserve_proxy) {
        Reserve(reserve_proxy.GetReserveSize());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector temp(rhs);
            swap(temp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            data_ = std::move(rhs.data_);
            size_ = rhs.size_;
            capacity_ = rhs.capacity_;
            rhs.size_ = 0;
            rhs.capacity_ = 0;
        }
        return *this;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return data_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index];
    }

    Iterator begin() noexcept {
        return data_.Get();
    }

    Iterator end() noexcept {
        return data_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return data_.Get();
    }

    ConstIterator end() const noexcept {
        return data_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return data_.Get();
    }

    ConstIterator cend() const noexcept {
        return data_.Get() + size_;
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        } else {
            if (new_size > capacity_) {
                Reserve(new_size > capacity_ * 2 ? new_size : capacity_ * 2);
            }
            for (size_t i = size_; i < new_size; ++i) {
                data_[i] = Type{};
            }
            size_ = new_size;
        }
    }

    void PushBack(const Type& item) {
    if (size_ == capacity_) {
        size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
        ArrayPtr<Type> new_data(new_capacity);
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = data_[i];
        }
        std::swap(data_, new_data);
        capacity_ = new_capacity;
    }
    data_[size_++] = item;
}

    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[size_++] = std::move(item);
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());

        size_t offset = pos - begin();
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        std::move_backward(begin() + offset, end(), end() + 1);
        data_[offset] = value;
        ++size_;
        return begin() + offset;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());

        size_t offset = pos - begin();
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        std::move_backward(begin() + offset, end(), end() + 1);
        data_[offset] = std::move(value);
        ++size_;
        return begin() + offset;
    }

    void PopBack() noexcept {
        assert(size_ > 0);
        --size_;
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());

        size_t offset = pos - begin();
        std::move(begin() + offset + 1, end(), begin() + offset);
        --size_;
        return begin() + offset;
    }

    void swap(SimpleVector& other) noexcept {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_data(new_capacity);
            std::move(begin(), end(), new_data.Get());
            data_.swap(new_data);
            capacity_ = new_capacity;
        }
    }

private:
    ArrayPtr<Type> data_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

ReserveProxy Reserve(size_t capacity_to_reserve) {
    return ReserveProxy(capacity_to_reserve);
}

template <typename Type>
bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
