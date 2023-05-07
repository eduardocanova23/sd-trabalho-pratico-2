#include <atomic>

class Spinlock {
    public: 
        Spinlock();
        void acquire();
        void release();
    private:
        std::atomic_flag held; 

};