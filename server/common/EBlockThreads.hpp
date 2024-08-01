//
// Created by 20075 on 2024/6/18.
//

#ifndef EBLOCK_EBLOCKTHREADS_H
#define EBLOCK_EBLOCKTHREADS_H


#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <future>
#include <functional>
#include <mutex>
#include <condition_variable>

class EBlockThreads {
        public:
        EBlockThreads(size_t threads);
        ~EBlockThreads();

        template<class F, class... Args>
        auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

        private:
        // Worker threads
        std::vector<std::thread> workers;
        // Task queue
        std::queue<std::function<void()>> tasks;

        // Synchronization
        std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop;
};

inline EBlockThreads::EBlockThreads(size_t threads)
: stop(false)
        {
                for (size_t i = 0; i < threads; ++i)
                workers.emplace_back(
                [this] {
                    for (;;) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock,
                            [this] { return this->stop || !this->tasks.empty(); });
                            if (this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                }
                );
        }

inline EBlockThreads::~EBlockThreads()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers)
        worker.join();
}

template<class F, class... Args>
auto EBlockThreads::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // Don't allow enqueueing after stopping the pool
        if (stop)
        throw std::runtime_error("enqueue on stopped EBlockThreads");

        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}


#endif //EBLOCK_EBLOCKTHREADS_H
