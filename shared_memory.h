#pragma once

#include <fcntl.h>     // O_CREAT, O_RDWR
#include <sys/mman.h>  // shm_open, mmap, munmap, shm_unlink
#include <unistd.h>    // ftruncate, close

#include <cstring>  // strerror
#include <iostream>
#include <stdexcept>
#include <string>

class SharedMemory {
 public:
  SharedMemory(const std::string& name, size_t size)
      : shm_name_(name), shm_size_(size), shm_fd_(-1), shm_ptr_(nullptr) {
    // unlink old shared mem
    shm_unlink(shm_name_.c_str());

    if (shm_name_.empty() || shm_name_[0] != '/') {
      throw std::invalid_argument("The shared memory name must start with '/'");
    }

    shm_fd_ = shm_open(shm_name_.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd_ == -1) {
      throw std::runtime_error("shm_open fail: " +
                               std::string(strerror(errno)));
    }

    // set the size of the shared memory object
    if (ftruncate(shm_fd_, shm_size_) == -1) {
      close(shm_fd_);
      shm_unlink(shm_name_.c_str());
      throw std::runtime_error("ftruncate fail: " +
                               std::string(strerror(errno)));
    }

    // map the shared memory object into the process's address space
    shm_ptr_ = mmap(nullptr, shm_size_, PROT_READ | PROT_WRITE, MAP_SHARED,
                    shm_fd_, 0);
    if (shm_ptr_ == MAP_FAILED) {
      close(shm_fd_);
      shm_unlink(shm_name_.c_str());
      throw std::runtime_error("mmap fail: " + std::string(strerror(errno)));
    }
  }

  ~SharedMemory() {
    if (shm_ptr_ && shm_ptr_ != MAP_FAILED) {
      munmap(shm_ptr_, shm_size_);
      shm_ptr_ = nullptr;
    }
    if (shm_fd_ != -1) {
      close(shm_fd_);
      shm_fd_ = -1;
    }
    shm_unlink(shm_name_.c_str());
  }

  // disable copy
  SharedMemory(const SharedMemory&) = delete;
  SharedMemory& operator=(const SharedMemory&) = delete;

  SharedMemory(SharedMemory&& other) noexcept
      : shm_name_(std::move(other.shm_name_)),
        shm_size_(other.shm_size_),
        shm_fd_(other.shm_fd_),
        shm_ptr_(other.shm_ptr_) {
    other.shm_fd_ = -1;
    other.shm_ptr_ = nullptr;
    other.shm_size_ = 0;
  }

  SharedMemory& operator=(SharedMemory&& other) noexcept {
    if (this != &other) {
      if (shm_ptr_ && shm_ptr_ != MAP_FAILED) {
        munmap(shm_ptr_, shm_size_);
      }
      if (shm_fd_ != -1) {
        close(shm_fd_);
        shm_unlink(shm_name_.c_str());
      }

      shm_name_ = std::move(other.shm_name_);
      shm_size_ = other.shm_size_;
      shm_fd_ = other.shm_fd_;
      shm_ptr_ = other.shm_ptr_;

      other.shm_fd_ = -1;
      other.shm_ptr_ = nullptr;
      other.shm_size_ = 0;
    }
    return *this;
  }

  void* data() const { return shm_ptr_; }
  size_t size() const { return shm_size_; }

 private:
  std::string shm_name_;
  size_t shm_size_;
  int shm_fd_;
  void* shm_ptr_;
};
