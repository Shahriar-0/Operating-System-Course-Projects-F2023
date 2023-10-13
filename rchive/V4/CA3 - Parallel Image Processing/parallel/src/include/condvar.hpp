#ifndef CONDVAR_HPP
#define CONDVAR_HPP

#include <pthread.h>

#include "mutex.hpp"

class CondVar
{
public:
    CondVar()
    {
        pthread_cond_init(&cond_, nullptr);
    }
    ~CondVar()
    {
        pthread_cond_destroy(&cond_);
    }

    void wait(Mutex &mutex)
    {
        pthread_cond_wait(&cond_, &mutex.mutex_);
    }

    void notifyOne()
    {
        pthread_cond_signal(&cond_);
    }
    void notifyAll()
    {
        pthread_cond_broadcast(&cond_);
    }

private:
    pthread_cond_t cond_;
};

#endif // CONDVAR_HPP