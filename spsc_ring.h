#ifndef _SPSC_RING_INCLUDE_H_
#define _SPSC_RING_INCLUDE_H_

#include<stdint.h>

#include<algorithm>

#include"common.h"


template<typename type, unsigned size>
class spsc_ring{
private:
    struct cons_header{
        volatile uint32_t head;
        uint32_t size;
        uint32_t mask;
    };

    struct prod_header{
        volatile uint32_t head;
        uint32_t size;
        uint32_t mask;
    };

public:
    spsc_ring(){
        static_assert(size & size - 1 == 0, "must be pow of 2");
        this->cons_.head = 0;
        this->prod_.head = 0;
        this->cons_.size = size;
        this->cons_.mask = ~(size - 1);
        this->prod_.size = size;
        this->prod_.mask = ~(size - 1);
    }
    ~spsc_ring(){}

    type* consume();

    uint32_t consume_all(type** obj);

    void product(type* obj);

    uint32_t product_n(type** obj, int n);

private:
    type* ring_[size] cacheline_align;
    cons_header cacheline_align cons_;
    prod_header cacheline_align prod_;
};

template<typename type, unsigned size>
type* spsc_ring<type, size>::consume(){
    type* obj;
    while(true){
        uint32_t head = this->cons_.head;
        __sync_synchronize();
        uint32_t tail = this->prod_.head;
        int free_entries = tail - head;
        if(unlikely(free_entries <= 0)){
            continue;
        }
        uint32_t size =  this->cons_.size;
        obj = this->ring_[head % size];
        this->cons_.head++;
        __sync_synchronize();
        break;
    }
    return obj;
}

template<typename type, unsigned size>
uint32_t spsc_ring<type, size>::consume_all(type** start_obj){
    int start;
    uint32_t count;
    uint32_t size = this->prod_.size;
    uint32_t head = this->cons_.head;
    fence;
    uint32_t tail = this->prod_.head;
    int free_entries = tail - head;
    start = head;
    count = free_entries;
    start = start % size;
    for(int i = start, j = 0; i < count; ++i, ++j){
        start_obj[j] = this->ring_[i];
    }
    this->cons_.head += count;
    __sync_synchronize();
    return count;
}


template<typename type, unsigned size>
void spsc_ring<type, size>::product(type* obj){
    uint32_t start;
    while(true){
        uint32_t tail = this->prod_.head;
        __sync_synchronize();
        uint32_t head = this->cons_.head;
        uint32_t mask = this->prod_.mask;
        int free_entries = head + mask - tail;
        if(unlikely(free_entries < 0)){
            continue;
        }
        start = tail;
    }
    this->ring_[start] = obj;
    this->prod_.head++;`
    __sync_synchronize();
}

template<typename type, unsigned size>
uint32_t spsc_ring<type, size>::product_n(type** obj, int n){
    uint32_t start;
    uint32_t count;
    uint32_t tail = this->prod_.head;
    __sync_synchronize();
    uint32_t head = this->cons_.head;
    uint32_t mask = this->prod_.mask;
    uint32_t size = this->prod_.size;
    int free_entries = head + mask - tail;
    count = std::min(free_entries, n);
    start = tail % size;
    for(int i = start, j = 0; i < count; ++i){
        this->ring_[i] = obj[j];
    }
    this->prod_.head += count;
    __sync_synchronize();
    return count;
}


#endif /** _SPSC_RING_INCLUDE_H_ */