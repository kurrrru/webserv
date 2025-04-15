// Copyright 2025 Ideal Broccoli
// Description: This file is a mimic of SharedPtr in C++11.
// std::SharedPtr cannot be used in C++98, so I mimic it.

#pragma once

#include <algorithm>
#include <utility>

namespace toolbox {
template <typename T>
class SharedPtr {
 public:
    SharedPtr() : _ptr(NULL), _count(new int(0)) {}

    explicit SharedPtr(T* ptr) : _ptr(ptr), _count(new int(1)) {
        if (ptr == NULL) {
            *_count = 0;
        }
    }
    SharedPtr(const SharedPtr& other)
        : _ptr(other._ptr), _count(other._count) {
        if (other._ptr != NULL) {
            ++(*_count);
        } else {
            _count = new int(0);
        }
    }

    // if other._ptr == NULL allocate new memory and assign 0
    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            int *tmp_count = other._count;
            if (other._ptr != NULL) {
                ++(*tmp_count);
            } else {
                tmp_count = new int(0);
            }
            if (--(*_count) <= 0) {
                delete _ptr;
                delete _count;
            }
            _ptr = other._ptr;
            _count = tmp_count;
        }
        return *this;
    }

    void swap(SharedPtr& other) {
        std::swap(_ptr, other._ptr);
        std::swap(_count, other._count);
    }

    ~SharedPtr() {
        if (*_count == 0) {
            delete _count;
            return;
        }
        if (--(*_count) == 0) {
            delete _ptr;
            delete _count;
        }
    }

    T* get() const {
        return _ptr;
    }

    int use_count() const {
        return *_count;
    }

    void reset(T* ptr = NULL) {
        int* new_count = new int(1);
        if (ptr == NULL) {
            *new_count = 0;
        }
        if (--(*_count) == 0) {
            delete _ptr;
            delete _count;
        }
        _ptr = ptr;
        _count = new_count;
    }

    T& operator*() const {
        return *_ptr;
    }

    T* operator->() const {
        return _ptr;
    }

    operator bool() const {
        return _ptr != NULL;
    }

 private:
    T* _ptr;
    int* _count;
};

template <typename T>
void swap(SharedPtr<T>& lhs, SharedPtr<T>& rhs) {
    lhs.swap(rhs);
}

}  // namespace toolbox
