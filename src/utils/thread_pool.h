/**
* @file thread_pool.h
* @date 2016
* @brief Thread pool class
*
*/

#ifndef STEGODISK_UTILS_THREAD_POOL_H_
#define STEGODISK_UTILS_THREAD_POOL_H_

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <algorithm>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include "api_mask.h"
#include "utils/exceptions.h"


namespace stego_disk {

class ThreadPool {
public:
  ThreadPool(size_t);
  template<class F, class... Args>
  auto enqueue(F&& f, Args&&... args)
  -> std::future<typename std::result_of<F(Args...)>::type>;
  ~ThreadPool();
private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()> > tasks;

  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop;
};

inline ThreadPool::ThreadPool(size_t threads)
  :   stop(false)
{

  if (threads == 0)
    threads = std::max(4, static_cast<int>(std::thread::hardware_concurrency()));

  for(size_t i = 0; i < threads; ++i)
    workers.emplace_back(
          [this]
    {
      for(;;)
      {
        std::function<void()> task;

        {
          std::unique_lock<std::mutex> lock(this->queue_mutex);
          this->condition.wait(lock,
                               [this]{ return this->stop || !this->tasks.empty(); });
          if(this->stop && this->tasks.empty())
            return;
          task = std::move(this->tasks.front());
          this->tasks.pop();
        }

        task();
      }
    }
    );
}

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
  using return_type = typename std::result_of<F(Args...)>::type;

  auto task = std::make_shared< std::packaged_task<return_type()> >(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                );

  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(queue_mutex);

    if(stop)
      throw exception::InvalidState{exception::Operation::enqueue,
	  								exception::Component::threadPool,
	  								exception::ComponentState::stopped};

    tasks.emplace([task](){ (*task)(); });
  }
  condition.notify_one();
  return res;
}

inline ThreadPool::~ThreadPool()
{
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
  }
  condition.notify_all();
  for(std::thread &worker: workers)
    worker.join();
}

} //stego_disk

#endif //STEGODISK_UTILS_THREAD_POOL_H_
