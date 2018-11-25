/******************************************************************************
 *
 * brica/thread_pool.hpp
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *****************************************************************************/

#ifndef __BRICA_THREAD_POOL_HPP__
#define __BRICA_THREAD_POOL_HPP__

#define ASIO_STANDALONE

#include <asio.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace brica {

class ThreadPool {
 public:
  ThreadPool(std::size_t size = 0)
      : work(std::make_shared<asio::io_service::work>(io_service)) {
    if (size == 0) {
      size = std::thread::hardware_concurrency();
    }

    for (std::size_t i = 0; i < size; ++i) {
      threads.emplace_back([this] { io_service.run(); });
    }
  }

  ~ThreadPool() {
    work.reset();
    for (std::size_t i = 0; i < threads.size(); ++i) {
      threads[i].join();
    }
  }

  void request(std::size_t n) { count = n; }

  void enqueue(std::function<void()>&& f) {
    io_service.post([this, f] {
      f();
      --count;
    });
  }

  void sync() {
    while (count > 0) {
    }
  }

 private:
  asio::io_service io_service;
  std::shared_ptr<asio::io_service::work> work;
  std::vector<std::thread> threads;

  std::atomic<std::size_t> count;
};

}  // namespace brica

#endif  // __BRICA_THREAD_POOL_HPP__
