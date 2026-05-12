#pragma once

#include "core/defines.h"
#include "core/logger.h"

#include <vector>
#include <utility>
#include <thread>

namespace pkm {


    #ifdef __cpp_lib_hardware_interference_size
        using std::hardware_destructive_interference_size;
    #else
        // 64 bytes is the standard cache line size for almost all modern CPUs
        constexpr size_t hardware_destructive_interference_size = 64;
    #endif


    template <typename T>
    class SPSCQueue {
    
        public:
            SPSCQueue(size_t capacity) noexcept : m_capacity(capacity), m_queue(capacity), m_head(0), m_tail(0) {}
            ~SPSCQueue() = default;
            

            bool push(T val) noexcept {
                size_t head = m_head.load(std::memory_order_acquire);
                size_t tail = m_tail.load(std::memory_order_relaxed);
                    
                // capacity reached
                if (head == ((tail + 1) % m_capacity)) return false;
                
                m_queue[tail] = std::move(val);
                tail = (tail + 1) % (m_capacity); 
                
                m_tail.store(tail, std::memory_order_release);
                return true;
            }

            bool pop(T& out) noexcept {
                size_t head = m_head.load(std::memory_order_relaxed);
                size_t tail = m_tail.load(std::memory_order_acquire);
                
                // empty
                if (head == tail) return false;
                
                // to avoid abunch of copies we just move 
                out = std::move(m_queue[head]);
                head = (head + 1) % m_capacity;

                m_head.store(head, std::memory_order_release);
                return true;
            }
        
        private:
            size_t m_capacity;
            std::vector<T> m_queue;
            
            alignas(hardware_destructive_interference_size) std::atomic<size_t> m_head;
            alignas(hardware_destructive_interference_size) std::atomic<size_t> m_tail;
    };
    

}
