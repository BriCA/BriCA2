#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "brica/brica.hpp"

#include <fcntl.h>
#include <paths.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <sstream>
#include <string>

namespace py = pybind11;

py::object to_object(brica::Buffer buffer) {
  auto pickler = py::module::import("dill");
  auto loads = pickler.attr("loads");

  if (buffer.size() > 0) {
    std::string pickle(buffer.begin(), buffer.end());
    return loads(py::bytes(pickle));
  }

  return py::none();
}

void send_pickle(std::string name, py::object obj) {
  auto pickler = py::module::import("dill");
  auto dumps = pickler.attr("dumps");
  std::string pickle = py::bytes(dumps(obj));

  std::string length_name = name + "length";
  std::string pickle_name = name + "pickle";

  std::size_t size = pickle.size();

  int length_fd = shm_open(length_name.c_str(), O_CREAT | O_RDWR, 0600);
  ftruncate(length_fd, sizeof(std::size_t));
  std::size_t* length_ptr = reinterpret_cast<std::size_t*>(
      mmap(0, sizeof(std::size_t), PROT_READ | PROT_WRITE, MAP_SHARED,
           length_fd, 0));
  *length_ptr = size;

  int pickle_fd = shm_open(pickle_name.c_str(), O_CREAT | O_RDWR, 0600);
  ftruncate(pickle_fd, sizeof(char) * size);
  char* pickle_ptr = reinterpret_cast<char*>(mmap(0, sizeof(char) * size,
                                                  PROT_READ | PROT_WRITE,
                                                  MAP_SHARED, pickle_fd, 0));
  std::copy(pickle.begin(), pickle.end(), pickle_ptr);

  munmap(pickle_ptr, sizeof(char) * size);
  munmap(length_ptr, sizeof(std::size_t));

  close(length_fd);
  close(pickle_fd);
}

py::object recv_pickle(std::string name) {
  std::string length_name = name + "length";
  std::string pickle_name = name + "pickle";

  std::size_t size;

  int length_fd = shm_open(length_name.c_str(), O_CREAT | O_RDWR, 0600);
  ftruncate(length_fd, sizeof(std::size_t));
  std::size_t* length_ptr = reinterpret_cast<std::size_t*>(
      mmap(0, sizeof(std::size_t), PROT_READ | PROT_WRITE, MAP_SHARED,
           length_fd, 0));
  size = *length_ptr;

  int pickle_fd = shm_open(pickle_name.c_str(), O_CREAT | O_RDWR, 0600);
  ftruncate(pickle_fd, sizeof(char) * size);
  char* pickle_ptr = reinterpret_cast<char*>(mmap(0, sizeof(char) * size,
                                                  PROT_READ | PROT_WRITE,
                                                  MAP_SHARED, pickle_fd, 0));
  std::string pickle(pickle_ptr, pickle_ptr + size);

  munmap(pickle_ptr, sizeof(char) * size);
  munmap(length_ptr, sizeof(std::size_t));

  close(length_fd);
  close(pickle_fd);

  shm_unlink(length_name.c_str());
  shm_unlink(pickle_name.c_str());

  auto pickler = py::module::import("dill");
  auto loads = pickler.attr("loads");
  return loads(py::bytes(pickle));
}

void clean_dict(std::string name) {
  std::string stat_name = name + "stat";
  std::string lens_name = name + "lens";
  std::string size_name = name + "size";
  std::string keys_name = name + "keys";
  std::string vals_name = name + "vals";

  shm_unlink(stat_name.c_str());
  shm_unlink(lens_name.c_str());
  shm_unlink(size_name.c_str());
  shm_unlink(keys_name.c_str());
  shm_unlink(vals_name.c_str());
}

void send_dict(std::string name, brica::Dict& dict) {
  std::string stat_name = name + "stat";
  std::string lens_name = name + "lens";
  std::string size_name = name + "size";
  std::string keys_name = name + "keys";
  std::string vals_name = name + "vals";

  int stat_fd = shm_open(stat_name.c_str(), O_CREAT | O_RDWR, 0600);
  ftruncate(stat_fd, sizeof(std::size_t));
  std::size_t* stat_ptr = reinterpret_cast<std::size_t*>(mmap(
      0, sizeof(std::size_t), PROT_READ | PROT_WRITE, MAP_SHARED, stat_fd, 0));
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
    brica::Buffer val = dict.index(i);
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
    brica::Buffer val = dict.index(i);
    std::copy(key.begin(), key.end(), keys_ptr);
    std::copy(val.begin(), val.end(), vals_ptr);
    keys_ptr += key.size();
    vals_ptr += val.size();
  }

  munmap(keys_ptr, sizeof(char) * keys_size);
  munmap(vals_ptr, sizeof(char) * vals_size);
  munmap(size_ptr, sizeof(std::size_t));
  munmap(stat_ptr, sizeof(std::size_t));
  munmap(lens_ptr, sizeof(std::size_t));

  close(stat_fd);
  close(lens_fd);
  close(size_fd);
  close(keys_fd);
  close(vals_fd);
}

void recv_dict(std::string name, brica::Dict& dict) {
  std::string stat_name = name + "stat";
  std::string lens_name = name + "lens";
  std::string size_name = name + "size";
  std::string keys_name = name + "keys";
  std::string vals_name = name + "vals";

  int stat_fd = shm_open(stat_name.c_str(), O_RDWR, 0600);
  ftruncate(stat_fd, sizeof(std::size_t));
  std::size_t* stat_ptr = reinterpret_cast<std::size_t*>(mmap(
      0, sizeof(std::size_t), PROT_READ | PROT_WRITE, MAP_SHARED, stat_fd, 0));
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
    brica::Buffer val(vals_ptr, vals_ptr + size_ptr[i]);
    dict[key] = val;
    keys_ptr += lens_ptr[i];
    vals_ptr += size_ptr[i];
  }

  munmap(keys_ptr, sizeof(char) * keys_size);
  munmap(vals_ptr, sizeof(char) * vals_size);
  munmap(size_ptr, sizeof(std::size_t));
  munmap(stat_ptr, sizeof(std::size_t));
  munmap(lens_ptr, sizeof(std::size_t));

  close(stat_fd);
  close(lens_fd);
  close(size_fd);
  close(keys_fd);
  close(vals_fd);

  clean_dict(name);
}

class Component final : public brica::IComponent {
 public:
  Component(py::object f) {
    std::stringstream ss;
    ss << static_cast<void*>(this);
    name = ss.str();

    sem = sem_open(name.c_str(), O_CREAT, 0600, 0);
    send_pickle(name, f);
    if ((pid = fork()) == 0) {
      std::string cmd = "brica-pyproxy " + name;
      exit(execl(_PATH_BSHELL, "sh", "-c", cmd.c_str(), (char*)NULL));
    }
    sem_wait(sem);
  }

  ~Component() {
    kill(pid, SIGUSR1);
    sem_post(sem);
    sem_close(sem);
    sem_unlink(name.c_str());
    clean_dict(name);
  }

 public:
  void make_in_port(std::string name) {
    inputs.try_emplace(name, brica::Buffer());
    in_ports.try_emplace(name, std::make_shared<brica::Port<brica::Buffer>>());
  }

  std::shared_ptr<brica::Port<brica::Buffer>>& get_in_port(std::string name) {
    return in_ports.at(name);
  }

  void make_out_port(std::string name) {
    outputs.try_emplace(name, brica::Buffer());
    out_ports.try_emplace(name, std::make_shared<brica::Port<brica::Buffer>>());
  }

  std::shared_ptr<brica::Port<brica::Buffer>>& get_out_port(std::string name) {
    return out_ports.at(name);
  }

  brica::Buffer& get_input(std::string name) { return inputs.at(name); }
  brica::Buffer& get_output(std::string name) { return outputs.at(name); }

  py::object get_in_port_value(std::string name) {
    return to_object(in_ports.at(name)->get());
  }

  py::object get_out_port_value(std::string name) {
    return to_object(out_ports.at(name)->get());
  }

  py::object get_input_object(std::string name) {
    return to_object(inputs.at(name));
  }

  py::object get_output_object(std::string name) {
    return to_object(outputs.at(name));
  }

  void connect(Component& target, std::string out, std::string in) {
    in_ports.at(in) = target.out_ports.at(out);
  }

  void collect() {
    for (std::size_t i = 0; i < inputs.size(); ++i) {
      inputs.index(i) = in_ports.index(i)->get();
    }
  }

  void execute() {
    send_dict(name, inputs);
    sem_post(sem);
    sem_wait(sem);
    recv_dict(name, outputs);
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

  brica::Dict inputs;
  brica::Dict outputs;

  brica::Ports<brica::Buffer> in_ports;
  brica::Ports<brica::Buffer> out_ports;
};

static bool running = true;

void handle(int) { running = false; }

void proxy(std::string name) {
  signal(SIGUSR1, handle);

  sem_t* sem = sem_open(name.c_str(), O_CREAT, 0600, 0);

  brica::Dict inputs;
  brica::Dict outputs;

  py::object f = recv_pickle(name);

  sem_post(sem);
  sem_wait(sem);

  auto pickler = py::module::import("dill");
  auto dumps = pickler.attr("dumps");
  auto loads = pickler.attr("loads");

  while (running) {
    recv_dict(name, inputs);

    py::dict args;

    for (auto it = inputs.begin(); it != inputs.end(); ++it) {
      std::string key = it->first;
      brica::Buffer val = it->second;
      if (val.size() > 0) {
        std::string pickle(val.begin(), val.end());
        args[py::str(key)] = loads(py::bytes(pickle));
      } else {
        args[py::str(key)] = py::none();
      }
    }

    py::dict rets = f(args);

    for (auto it = rets.begin(); it != rets.end(); ++it) {
      std::string key = py::str(it->first);
      std::string pickle = py::bytes(dumps(it->second));
      outputs[key] = brica::Buffer(pickle.begin(), pickle.end());
    }

    send_dict(name, outputs);

    sem_post(sem);
    sem_wait(sem);
  }

  sem_close(sem);
}

PYBIND11_MODULE(_brica, m) {
  m.def("proxy", &proxy);

  py::class_<brica::IComponent>(m, "IComponent")
      .def("collect", &brica::IComponent::collect)
      .def("execute", &brica::IComponent::execute)
      .def("expose", &brica::IComponent::expose);

  py::class_<Component, brica::IComponent>(m, "Component")
      .def(py::init<py::object>())
      .def("make_in_port", &Component::make_in_port)
      .def("make_out_port", &Component::make_out_port)
      .def("get_in_port_value", &Component::get_in_port_value)
      .def("get_out_port_value", &Component::get_out_port_value)
      .def("get_input", &Component::get_input_object)
      .def("get_output", &Component::get_output_object);

  m.def("connect", &brica::connect<Component>);

  py::class_<brica::Timing>(m, "Timing")
      .def(py::init<brica::Time, brica::Time, brica::Time>());

  py::class_<brica::VirtualTimePhasedScheduler>(m, "VirtualTimePhasedScheduler")
      .def(py::init<>())
      .def("add_component", &brica::VirtualTimePhasedScheduler::add_component)
      .def("step", &brica::VirtualTimePhasedScheduler::step);

  py::class_<brica::VirtualTimeScheduler>(m, "VirtualTimeScheduler")
      .def(py::init<>())
      .def("add_component", &brica::VirtualTimeScheduler::add_component)
      .def("step", &brica::VirtualTimeScheduler::step);
}
