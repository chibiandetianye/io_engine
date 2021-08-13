#ifndef __ASYNC_INCLUDE_H_
#define __ASYNC_INCLUDE_H_

// __sync__ built in gcc
#define atomic_increase(ptr) __sync_fetch_and_add(ptr, 1)
#define atomic_decrease(ptr) __sync_fetch_and_sub(ptr, 1)
#define compare_and_set(ptr, value) __sync_lock_test_and_set(ptr, value)

#define fence __sync_synchronize()



#endif /** _ASYNC_INCLUDE_H_ */