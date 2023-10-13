#include "thread.hpp"

namespace thread
{
    // Thread

    void Thread::start()
    {
        pthread_create(&tid_, nullptr, entry, this);
    }
    void Thread::join()
    {
        pthread_join(tid_, nullptr);
    }
    void *Thread::entry(void *arg)
    {
        Thread *thread = static_cast<Thread *>(arg);
        thread->run();
        return nullptr;
    }

    // PoolQueue

    Task *PoolQueue::next()
    {
        queueMutex_.lock();
        while (queueTasks_.empty())
        {
            waitingThreads = waitingThreads + 1;

            waitingThreadsMutex_.lock();
            waitingThreadsCond_.notifyOne();
            waitingThreadsMutex_.unlock();

            queueCond_.wait(queueMutex_);
            waitingThreads = waitingThreads - 1;
        }
        Task *task = queueTasks_.front();
        queueTasks_.pop();
        queueMutex_.unlock();
        return task;
    }
    void PoolQueue::add(Task *task)
    {
        queueMutex_.lock();
        queueTasks_.push(task);
        queueCond_.notifyOne();
        queueMutex_.unlock();
    }
    void PoolQueue::waitToFinish(int threadCount)
    {
        waitingThreadsMutex_.lock();
        while (waitingThreads != threadCount || !queueTasks_.empty())
        {
            waitingThreadsCond_.wait(waitingThreadsMutex_);
        }
        waitingThreadsMutex_.unlock();
    }

    // PoolThread

    PoolThread::PoolThread(PoolQueue &workQueue) : workQueue_(&workQueue) {}
    void PoolThread::run()
    {
        while (true)
        {
            Task *task = workQueue_->next();
            if (task == nullptr)
                break;
            task->run();
            delete task;
        }
    }

    // Pool

    Pool::Pool(int num)
    {
        threads_.assign(num, PoolThread(workQueue_));
        for (auto &thread : threads_)
        {
            thread.start();
        }
    }
    Pool::~Pool()
    {
        for (unsigned i = 0; i < threads_.size(); ++i)
        {
            workQueue_.add(nullptr);
        }
        for (auto &thread : threads_)
        {
            thread.join();
        }
    }
    void Pool::add(Task *task) { workQueue_.add(task); }
    void Pool::waitForTasks() { workQueue_.waitToFinish(threads_.size()); }
    unsigned Pool::count() const { return threads_.size(); }

} // namespace thread