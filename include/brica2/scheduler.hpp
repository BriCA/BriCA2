#ifndef __BRICA2_SCHEDULER_HPP__
#define __BRICA2_SCHEDULER_HPP__

#include "brica2/component.hpp"
#include "brica2/executors.hpp"

#include <queue>

NAMESPACE_BEGIN(BRICA2_NAMESPACE)

using duration_t = long long int;

struct timing_t {
  duration_t offset;
  duration_t interval;
  duration_t sleep;
};

class single_phase_scheduler {
 public:
  single_phase_scheduler(executor_type& e) : executor(e) {}

  void add(component_type& component) { components.push_back(&component); }

  template <class InputIt> void add(InputIt first, InputIt last) {
    std::for_each(first, last, [&](auto& c) { add(c); });
  }

  void step() {
    std::vector<std::function<void(void)>> fs(components.size());
    for (std::size_t i = 0; i < components.size(); ++i) {
      auto component = components[i];
      fs[i] = [component]() {
        component->collect();
        component->execute();
      };
    }

    executor.post(fs);
    executor.sync();

    for (std::size_t i = 0; i < components.size(); ++i) {
      auto component = components[i];
      fs[i] = [component]() { component->expose(); };
    }

    executor.post(fs);
    executor.sync();
  }

 private:
  std::vector<component_type*> components;
  executor_type& executor;
};

class multi_phase_scheduler {
 public:
  multi_phase_scheduler(executor_type& e) : executor(e) {}

  void add(component_type& component, std::size_t phase = 0) {
    while (phase >= phases.size()) phases.emplace_back(executor);
    phases[phase].add(component);
  }

  template <class InputIt>
  void add(InputIt first, InputIt last, std::size_t phase = 0) {
    std::for_each(first, last, [&](auto& c) { add(c, phase); });
  }

  void step() {
    for (auto phase : phases) phase.step();
  }

 private:
  std::vector<single_phase_scheduler> phases;
  executor_type& executor;
};

class virtual_time_scheduler {
 public:
  virtual_time_scheduler(executor_type& e) : executor(e) {}

  void add(component_type& component, timing_t timing) {
    event_queue.push({&component, timing.offset, timing, false});
  }

  template <class InputIt> void add(InputIt first, InputIt last, timing_t t) {
    std::for_each(first, last, [&](auto& c) { add(c, t); });
  }

  void step() {
    duration_t time = event_queue.top().time;

    std::vector<std::function<void(void)>> awake;
    std::vector<std::function<void(void)>> asleep;

    while (event_queue.top().time == time) {
      auto event = event_queue.top();
      event_queue.pop();

      auto c = event.component;

      if (event.sleep) {
        asleep.push_back([c]() { c->expose(); });
      } else {
        awake.push_back([c]() {
          c->collect();
          c->execute();
        });
      }

      event.time += event.sleep ? event.timing.sleep : event.timing.interval;
      event.sleep = !event.sleep;

      event_queue.push(event);
    }

    executor.post(asleep);
    executor.sync();

    executor.post(awake);
    executor.sync();
  }

 private:
  struct event_t {
    component_type* component;
    duration_t time;
    timing_t timing;
    bool sleep;

    bool operator<(const event_t& e) const { return e.time < time; }
  };

  std::priority_queue<event_t> event_queue;
  executor_type& executor;
};

NAMESPACE_END(BRICA2_NAMESPACE)

#endif  // __BRICA2_SCHEDULER_HPP__
