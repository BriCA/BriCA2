#ifndef __BRICA_FORK_HPP__
#define __BRICA_FORK_HPP__

#include "brica/component.hpp"
#include "brica/functor.hpp"
#include "brica/port.hpp"

#include <algorithm>
#include <sstream>
#include <string>

#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <err.h>
#include <errno.h>

#include <iostream>

namespace brica {

void bufprint(Buffer buffer) {
  std::cout << getpid() << ": ";
  for (std::size_t i = 0; i < buffer.size(); ++i) {
    if (i > 0) {
      std::cout << " ";
    }
    std::cout << static_cast<int>(buffer[i]);
  }
  std::cout << std::endl;
}

class ForkComponent final : public IComponent {
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
        for (std::size_t i = 0; i < inputs.size(); ++i) {
          recv_input(inputs.key(i));
        }
        sem_post(sem);
      }
    }

    sem_wait(sem);

    inputs["default"] = Buffer({1, 2, 3});
  }

  ~ForkComponent() {
    kill(pid, SIGUSR1);
    sem_close(sem);
    sem_unlink(name.c_str());
  }

 private:
  void send_input(std::string key, Buffer buffer) {
    bufprint(buffer);
    std::string port_size_name = name + ":in:" + key + ":size";
    std::string port_data_name = name + ":in:" + key + ":data";

    std::size_t size = buffer.size();
    int size_fd = shm_open(port_size_name.c_str(), O_CREAT | O_RDWR, 0600);
    ftruncate(size_fd, sizeof(std::size_t));
    void* size_ptr = mmap(0, sizeof(std::size_t), PROT_READ | PROT_WRITE,
                          MAP_SHARED, size_fd, 0);
    std::cout << "send" << std::endl;
    std::copy(&size, &size + sizeof(std::size_t), size_ptr);
    std::cout << "send" << std::endl;

    char* data = buffer.data();
    int data_fd = shm_open(port_data_name.c_str(), O_CREAT | O_RDWR, 0600);
    ftruncate(data_fd, sizeof(char) * size);
    void* data_ptr = mmap(0, sizeof(char) * size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, size_fd, 0);
    std::copy(data, data + size, data_ptr);
  }

  Buffer recv_input(std::string key) {
    std::cout << "recv" << std::endl;
    std::string port_size_name = name + ":in:" + key + ":size";
    std::string port_data_name = name + ":in:" + key + ":data";

    int size_fd = shm_open(port_size_name.c_str(), O_CREAT | O_RDWR, 0600);
    ftruncate(size_fd, sizeof(std::size_t));
    std::size_t* size_ptr = static_cast<std::size_t*>(
        mmap(0, sizeof(std::size_t), PROT_READ | PROT_WRITE, MAP_SHARED,
             size_fd, 0));
    std::size_t size = *size_ptr;

    Buffer buffer(size);

    int data_fd = shm_open(port_data_name.c_str(), O_CREAT | O_RDWR, 0600);
    ftruncate(data_fd, sizeof(char) * size);
    char* data_ptr =
        static_cast<char*>(mmap(0, sizeof(char) * size, PROT_READ | PROT_WRITE,
                                MAP_SHARED, size_fd, 0));
    std::copy(data_ptr, data_ptr + size, buffer.data());

    bufprint(buffer);

    return buffer;
  }

 public:
  void collect() {
    for (std::size_t i = 0; i < inputs.size(); ++i) {
      inputs.index(i) = in_ports.index(i)->get();
    }
  }

  void execute() {
    for (std::size_t i = 0; i < inputs.size(); ++i) {
      send_input(inputs.key(i), inputs.index(i));
    }
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
