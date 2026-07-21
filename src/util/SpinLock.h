#pragma once

#include <atomic>

namespace PLAMIOmini
{

// Cross-platform lock for very short critical sections shared between
// worker threads or processor cores. Do not acquire this lock from an ISR.
class SpinLock
{
public:
    SpinLock() = default;
    SpinLock(const SpinLock&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;

    void lock()
    {
        while (flag.test_and_set(std::memory_order_acquire))
        {
        }
    }

    void unlock()
    {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

class SpinLockGuard
{
public:
    explicit SpinLockGuard(SpinLock& lock)
        : lock(lock)
    {
        lock.lock();
    }

    ~SpinLockGuard()
    {
        lock.unlock();
    }

    SpinLockGuard(const SpinLockGuard&) = delete;
    SpinLockGuard& operator=(const SpinLockGuard&) = delete;

private:
    SpinLock& lock;
};

} // namespace PLAMIOmini
