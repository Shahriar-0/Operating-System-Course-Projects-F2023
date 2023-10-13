#ifndef MUTEX_HPP
#define MUTEX_HPP

#include <pthread.h>

class Mutex
{
public:
    Mutex()
    {
        pthread_mutex_init(&mutex_, nullptr);
    }
    ~Mutex()
    {
        pthread_mutex_destroy(&mutex_);
    }

    void lock()
    {
        pthread_mutex_lock(&mutex_);
    }
    void unlock()
    {
        pthread_mutex_unlock(&mutex_);
    }

private:
    pthread_mutex_t mutex_;
    friend class CondVar;
};

class LockGuard
{
public:
    LockGuard(Mutex &mutex) : mutex_(mutex) {}
    ~LockGuard() { unlock(); }

    void lock() { mutex_.lock(); }
    void unlock() { mutex_.unlock(); }

private:
    Mutex &mutex_;
};

#endif // MUTEX_HPP