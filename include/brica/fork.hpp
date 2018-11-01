#ifndef __BRICA_FORK_HPP__
#define __BRICA_FORK_HPP__

#include "brica/component.hpp"
#include "brica/functor.hpp"
#include "brica/port.hpp"

#include <sstream>

#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <err.h>
#include <errno.h>

#include <iostream>

namespace brica {

class ForkComponent : IComponent {
 public:
  ForkComponent() {
    std::stringstream ss;
    ss << '/' << getpid() << ':' << static_cast<void*>(this);
    name = ss.str();

    pid = fork();
    sem = sem_open(name.c_str(), O_CREAT, 0600, 0);

    if (pid == 0) {
      signal(SIGUSR1, [](int) { exit(0); });
      sem_post(sem);

      for (;;) {
        sem_wait(sem);
        std::cout << "execute" << std::endl;
        sem_post(sem);
      }
    }

    sem_wait(sem);
  }

  ~ForkComponent() {
    kill(pid, SIGUSR1);
    sem_close(sem);
    sem_unlink(name.c_str());
  }

  void collect() {
    for (std::size_t i = 0; i < inputs.size(); ++i) {
      inputs.index(i) = in_ports.index(i)->get();
    }
  }

  void execute() {
    sem_post(sem);
    sem_wait(sem);
  }

  void expose() {
    for (std::size_t i = 0; i < outputs.size(); ++i) {
      out_ports.index(i)->set(outputs.index(i));
    }
  }

 private:
  std::string name;
  sem_t* sem;
  pid_t pid;

  Dict inputs;
  Dict outputs;

  Ports<Buffer> in_ports;
  Ports<Buffer> out_ports;
};

}  // namespace brica

#endif  // __BRICA_FORK_HPP__
