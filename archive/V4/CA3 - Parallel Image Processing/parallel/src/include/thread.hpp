#ifndef THREAD_HPP
#define THREAD_HPP

#include <pthread.h>

#include <queue>

#include "atomic.hpp"
#include "condvar.hpp"
#include "mutex.hpp"

namespace thread
{
    class Thread
    {
    public:
        Thread() = default;
        virtual ~Thread() = default;

        void start();
        void join();

    protected:
        virtual void run() = 0;

    private:
        pthread_t tid_ = 0;

        static void *entry(void *arg);
    };

    class Task
    {
    public:
        Task() = default;
        virtual ~Task() = default;
        virtual void run() = 0;
    };

    class PoolQueue
    {
    public:
        PoolQueue() = default;
        ~PoolQueue() = default;

        Task *next();
        void add(Task *task);
        void waitToFinish(int threadCount);

    private:
        std::queue<Task *> queueTasks_;
        Mutex queueMutex_;
        CondVar queueCond_;

        Atomic<int> waitingThreads{0};
        Mutex waitingThreadsMutex_;
        CondVar waitingThreadsCond_;
    };

    class PoolThread final : public Thread
    {
    public:
        PoolThread(PoolQueue &workQueue);

    protected:
        void run() override;

    private:
        PoolQueue *workQueue_;
    };

    class Pool
    {
    public:
        Pool(int num);
        ~Pool();

        void add(Task *task);
        void waitForTasks();
        unsigned count() const;

    private:
        std::vector<PoolThread> threads_;
        PoolQueue workQueue_;
    };

} // namespace thread

#endif // THREAD_HPP