#ifndef ATOMIC_HPP
#define ATOMIC_HPP

#include <type_traits>

#include "mutex.hpp"

template <class T,
          class = typename std::enable_if<std::is_fundamental<T>::value>::type>
class Atomic
{
public:
    Atomic(T value = T{}) : value_(value) {}
    ~Atomic() = default;

    T load() const noexcept
    {
        LockGuard lock(mutex_);
        T value = value_;
        return value;
    }
    void store(T value) noexcept
    {
        LockGuard lock(mutex_);
        value_ = value;
    }

    Atomic &operator=(T value) noexcept
    {
        store(value);
        return *this;
    }
    operator T() const noexcept
    {
        return load();
    }

    Atomic &operator++() noexcept
    {
        LockGuard lock(mutex_);
        ++value_;
        return *this;
    }
    T operator++(int) noexcept
    {
        LockGuard lock(mutex_);
        T temp = value_;
        ++value_;
        return temp;
    }

    Atomic &operator--() noexcept
    {
        LockGuard lock(mutex_);
        --value_;
        return *this;
    }
    T operator--(int) noexcept
    {
        LockGuard lock(mutex_);
        T temp = value_;
        --value_;
        return temp;
    }

    Atomic &operator+=(T rhs) noexcept
    {
        LockGuard lock(mutex_);
        value_ += rhs;
        return *this;
    }
    Atomic &operator-=(T rhs) noexcept
    {
        LockGuard lock(mutex_);
        value_ -= rhs;
        return *this;
    }

private:
    T value_;
    mutable Mutex mutex_;
};

#endif // ATOMIC_HPP