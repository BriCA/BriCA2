#ifndef __BRICA_FORK_HPP__
#define __BRICA_FORK_HPP__

#include "brica/component.hpp"
#include "brica/functor.hpp"
#include "brica/port.hpp"

#include <algorithm>
#include <sstream>
#include <string>

#include <err.h>
#include <errno.h>
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

class ForkComponent final : public IComponent {
 public:
  ForkComponent(std::string id) {
    std::stringstream ss;
    ss << '/' << getpid() << ':' << id;
    name = ss.str();

    pid = fork();
    sem = sem_open(name.c_str(), O_CREAT, 0600, 0);

    if (pid == 0) {
      signal(SIGUSR1, [](int) { exit(0); });
      sem_post(sem);

      for (;;) {
        sem_wait(sem);
        recv_dict(outputs);
        sem_post(sem);
      }
    }

    sem_wait(sem);
  }

  ~ForkComponent() {
    kill(pid, SIGUSR1);
    sem_close(sem);
    sem_unlink(name.c_str());
    clean_shm();
  }

 private:
  void send_dict(Dict& dict) {
    std::string stat_name = name + ":stat";
    std::string lens_name = name + ":lens";
    std::string size_name = name + ":size";
    std::string keys_name = name + ":keys";
    std::string vals_name = name + ":vals";

    int stat_fd = shm_open(stat_name.c_str(), O_CREAT | O_RDWR, 0600);
    ftruncate(stat_fd, sizeof(std::size_t));
    std::size_t* stat_ptr = reinterpret_cast<std::size_t*>(
        mmap(0, sizeof(std::size_t), PROT_READ | PROT_WRITE, MAP_SHARED,
             stat_fd, 0));
    *stat_ptr = dict.size();

    int lens_fd = shm_open(lens_name.c_str(), O_CREAT | O_RDWR, 0600);
    ftruncate(lens_fd, sizeof(std::size_t) * dict.size());
    std::size_t* lens_ptr = reinterpret_cast<std::size_t*>(
        mmap(0, sizeof(std::size_t) * dict.size(), PROT_READ | PROT_WRITE,
             MAP_SHARED, lens_fd, 0));

    int size_fd = shm_open(size_name.c_str(), O_CREAT | O_RDWR, 0600);
    ftruncate(size_fd, sizeof(std::size_t) * dict.size());
    std::size_t* size_ptr = reinterpret_cast<std::size_t*>(
        mmap(0, sizeof(std::size_t) * dict.size(), PROT_READ | PROT_WRITE,
             MAP_SHARED, size_fd, 0));

    std::size_t keys_size = 0;
    std::size_t vals_size = 0;

    for (std::size_t i = 0; i < dict.size(); ++i) {
      std::string key = dict.key(i);
      Buffer val = dict.index(i);
      keys_size += lens_ptr[i] = key.size();
      vals_size += size_ptr[i] = val.size();
    }

    int keys_fd = shm_open(keys_name.c_str(), O_CREAT | O_RDWR, 0600);
    ftruncate(keys_fd, sizeof(char) * keys_size);
    char* keys_ptr = reinterpret_cast<char*>(mmap(0, sizeof(char) * keys_size,
                                                  PROT_READ | PROT_WRITE,
                                                  MAP_SHARED, keys_fd, 0));

    int vals_fd = shm_open(vals_name.c_str(), O_CREAT | O_RDWR, 0600);
    ftruncate(vals_fd, sizeof(char) * vals_size);
    char* vals_ptr = reinterpret_cast<char*>(mmap(0, sizeof(char) * vals_size,
                                                  PROT_READ | PROT_WRITE,
                                                  MAP_SHARED, vals_fd, 0));

    for (std::size_t i = 0; i < dict.size(); ++i) {
      std::string key = dict.key(i);
      Buffer val = dict.index(i);
      std::copy(key.begin(), key.end(), keys_ptr);
      std::copy(val.begin(), val.end(), vals_ptr);
      keys_ptr += key.size();
      vals_ptr += val.size();
    }

    close(stat_fd);
    close(lens_fd);
    close(size_fd);
    close(keys_fd);
    close(vals_fd);
  }

  void recv_dict(Dict& dict) {
    std::string stat_name = name + ":stat";
    std::string lens_name = name + ":lens";
    std::string size_name = name + ":size";
    std::string keys_name = name + ":keys";
    std::string vals_name = name + ":vals";

    int stat_fd = shm_open(stat_name.c_str(), O_RDWR);
    ftruncate(stat_fd, sizeof(std::size_t));
    std::size_t* stat_ptr = reinterpret_cast<std::size_t*>(
        mmap(0, sizeof(std::size_t), PROT_READ | PROT_WRITE, MAP_SHARED,
             stat_fd, 0));
    std::size_t size = *stat_ptr;

    int lens_fd = shm_open(lens_name.c_str(), O_CREAT | O_RDWR, 0600);
    ftruncate(lens_fd, sizeof(std::size_t) * size);
    std::size_t* lens_ptr = reinterpret_cast<std::size_t*>(
        mmap(0, sizeof(std::size_t) * size, PROT_READ | PROT_WRITE, MAP_SHARED,
             lens_fd, 0));

    int size_fd = shm_open(size_name.c_str(), O_CREAT | O_RDWR, 0600);
    ftruncate(size_fd, sizeof(std::size_t) * size);
    std::size_t* size_ptr = reinterpret_cast<std::size_t*>(
        mmap(0, sizeof(std::size_t) * size, PROT_READ | PROT_WRITE, MAP_SHARED,
             size_fd, 0));

    std::size_t keys_size = 0;
    std::size_t vals_size = 0;

    for (std::size_t i = 0; i < size; ++i) {
      keys_size += lens_ptr[i];
      vals_size += size_ptr[i];
    }

    int keys_fd = shm_open(keys_name.c_str(), O_CREAT | O_RDWR, 0600);
    ftruncate(keys_fd, sizeof(char) * keys_size);
    char* keys_ptr = reinterpret_cast<char*>(mmap(0, sizeof(char) * keys_size,
                                                  PROT_READ | PROT_WRITE,
                                                  MAP_SHARED, keys_fd, 0));

    int vals_fd = shm_open(vals_name.c_str(), O_CREAT | O_RDWR, 0600);
    ftruncate(vals_fd, sizeof(char) * vals_size);
    char* vals_ptr = reinterpret_cast<char*>(mmap(0, sizeof(char) * vals_size,
                                                  PROT_READ | PROT_WRITE,
                                                  MAP_SHARED, vals_fd, 0));

    for (std::size_t i = 0; i < size; ++i) {
      std::string key(keys_ptr, keys_ptr + lens_ptr[i]);
      Buffer val(vals_ptr, vals_ptr + size_ptr[i]);
      dict[key] = val;
      keys_ptr += lens_ptr[i];
      vals_ptr += size_ptr[i];
    }

    close(stat_fd);
    close(lens_fd);
    close(size_fd);
    close(keys_fd);
    close(vals_fd);
  }

  void clean_shm() {
    std::string stat_name = name + ":stat";
    std::string lens_name = name + ":lens";
    std::string size_name = name + ":size";
    std::string keys_name = name + ":keys";
    std::string vals_name = name + ":vals";

    shm_unlink(stat_name.c_str());
    shm_unlink(lens_name.c_str());
    shm_unlink(size_name.c_str());
    shm_unlink(keys_name.c_str());
    shm_unlink(vals_name.c_str());
  }

 public:
  void collect() {
    for (std::size_t i = 0; i < inputs.size(); ++i) {
      inputs.index(i) = in_ports.index(i)->get();
    }
  }

  void execute() {
    send_dict(inputs);
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
