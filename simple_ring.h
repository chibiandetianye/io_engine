#ifndef _SIMPLE_RING_INCLUDE_H_
#define _SIMPLE_RING_INCLUDE_H_

#include<pthread.h>

#include<queue>
#include<utility>

template<typename type>
class io_buffer{
public:
    io_buffer(){
        pthread_spin_init(&this->spin_lock_, PTHREAD_PROCESS_SHARED);
    }
    ~io_buffer(){
        pthread_spin_destroy(&this->spin_lock_);
    }

    type* consume();

    uint32_t consume_all(type** obj);

    void product(type* obj);

    uint32_t product_n(type** obj, uint32_t n);
private:
    std::queue<type*> queue_;
    pthread_spinlock_t spin_lock_;
};

template<typename type>
type* io_buffer<type>::consume(){
    bool notEmpty = true;
    type* obj;
    while(notEmpty){
        pthread_spin_lock(&this->spin_lock_);
        if(this->queue_.empty()){
            pthread_spin_unlock(&this->spin_lock_);
            continue;
        }
        obj = this->queue_.front();
        this->queue_.pop();
        pthread_spin_unlock(&this->spin_lock_);
        notEmpty = false;
    }
    return obj;
}

template<typename type>
uint32_t io_buffer<type>::consume(type** obj){
    pthread_spin_lock(&this->spin_lock_);
    int size = this->queue_.size();
    for(int i = 0; i < size; ++i){
        obj[i] = this->queue_.front();
        this->queue_.pop();
    }
    pthread_spin_unlock(&this->spin_lock_);
}

template<typename type>
void io_buffer<type>::product(type* obj){
    pthread_spin_lock(&this->spin_lock_);
    this->queue_.push(obj);
    pthread_spin_unlock(&this->spin_lock_);
}

template<typename type>
uint32_t io_buffer<type>::product_n(type** obj, uint32_t n){
    pthread_spin_lock(&this->spin_lock_);
    for(int i = 0; i < n; ++i){
        this->queue_.push(obj[i]);
    }
    pthread_spin_unlock(&this->spin_lock_);
}

#endif /** _SIMPLE_RING_INCLUDE_H_ */