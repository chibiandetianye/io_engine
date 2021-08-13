#include"aio.h"

namespace io{
void Asynchronous_reader::lock(){
    while(compare_and_set(&this->atomic_flag_, 1) == 0){
        while(this->atomic_flag_);
    }
}

void Asynchronous_reader:ulock(){
    atomic_decrease(&this->aotmic_flag_);
    fence;
}

void Asynchronous_reader::aio_read(char* __restrict__ buffer, int start, int capacity){
    this->worker_ = std::thread(
        [&]{
            //spin lock
            this->lock();
            read(this->fd_, buffer, capacity);
            //unlock
            this->ulock();
        }
    );
}

bool Asynchronous_reader::test_complete(){
    fence;
    return this->atomic_flag_ == 0;
}

void wait(){
    while(true){
        if(!this->test_complete()){
            break;
        }
    }
}

void Asynchronous_writer::init(){
    for(int i = 0; i < this->worker_queue_.size(); ++i){
        this->woker_queue_[i] = std::thread(
            [&]{
                bool running = true;
                taskPair* task_n[16];
                while(running){
                    int n = this->buffer_[i].consume_all(task_n);
                    if(n == 0)continue;
                    for(int i = 0; i < n; ++i){
                        int fd = task_n[i]->first;
                        content_t content = task_n[i]->second;
                        if(fd == 0) {
                            running = false;
                            break;
                        }
                        char* buf = content.first;
                        int* length = content.second;
                        write(fd, buf, length);
                        release(buf);
                        delete content;
                    }
                }
            }
        );
    }
}

void Asynchronous_writer::aio_write(taskPair& task, int key){
    const int queue_no = this->hash_func(key);
    this->buffer_[queue_no].product(&task);
}

void Asynchronous_writer::terminal_write(){
    for(int i = 0; i < 16; ++i){
        content_t content{nullptr, 0};
        
        taskPair* tp = this->create_taskpair(0, content);
        this->buffer_[i].product(tp);
    }
}

} // namespace io