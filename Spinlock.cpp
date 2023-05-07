#include "Spinlock.h"

Spinlock::Spinlock() : held(false){}

void Spinlock::acquire(){
    while(std::atomic_flag_test_and_set_explicit(&this->held, std::memory_order_acquire));
}

void Spinlock::release(){
    std::atomic_flag_clear_explicit(&this->held, std::memory_order_release);
}

