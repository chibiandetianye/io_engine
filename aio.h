#ifndef _AIO_INCLUDE_H_
#define _AIO_INCLUDE_H_

#include<unistd.h>

#include<thread>
#include<utility>
#include<array>
#include<functional>


#include"spsc_ring.h"
#include"async.h"
#ifdef _SPSC_RING_INCLUDE_H_
using Ring_buffer = spsc_ring;
#endif 

namespace io{

//this version only supports single reader 
class Asynchronous_reader{
public:
    using fd_t = int;
public:
    Asynchronous_Reader(fd_t fd):atomic_flag_(0), fd_(fd){}
    ~Asychronous_Reader(){
        if(this->worker_.joinable()){
            this->worker_.join();
        }
    }

    void aio_read(char* __restrict__ buffer, int capacity);

    bool test_complete();

    void wait();
protected:
    void lock();
    //without check whether the lock in locked, 
    //so must cofirm spin_lock is invoked in the upper layer code
    void ulock();
private:
    volatile int atomic_flag_; //lock 1 unlock 0
    std::thread worker_;
    fd_t fd_;
};

class Asynchronous_writer{
public:
    using Hash = std::function<int(int)>;
    using Release = std::function<void(void*)>;
public:
    using fd_t = int;
    using content_t = std::pair<char*, int>;
    using taskPair =  std::pair<fd_t, content_t>;
public:
    asynchronous_writer(Hash hash, const int queue_num, Release release):hash_func_(hash), release_(release){
        this->buffer_ = new Ring_buffer[queue_num];
        this->hash_func_ = hash;
    };
    ~asynchronous_writer(){
        for(int i = 0; i < this->worker_queue_.size(); ++i){
            if(worker_queue[i].joinable()){
                worker_queue[i].join();
            }
        }
        delete[] this->buffer_;
    }

    void init();

    void terminal_write();

    void aio_write(taskPair& task, int key);

    taskPair* create_taskpair(fd_t fd, content_t content){
        taskPair* tp = new taskPair();
        tp->first = fd;
        tp->second = content;
        return tp;
    }
    
private:
    Ring_buffer<taskPair, 16>* buffer_;
    std::array<std::thread> worker_queue_, 
    Hash hash_func_;
    Release release_;
};


} // namespace io

#endif /** _AIO_INCLUDE_H_ */