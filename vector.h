#ifndef VECTOR_H
#define VECTOR_H
#include <vector>
#include <iostream>

namespace tt {
template <typename T>
class Allocator : public std::allocator<T> {
public:
    explicit Allocator() = default;
    [[nodiscard]] T* allocate(size_t count) {
        return static_cast<T*>(::operator new(count * sizeof(T)));
    }

    void deallocate(T *memory, size_t /*n*/) {
        ::operator delete(memory);
    }

    template< typename U, typename...  Args>
    void construct(U *memory, Args&& ... args) {
        new(memory) T(std::forward<Args>(args)...);
    }


    void destroy(T* object) {
        object->~T();
    }
};

template <typename T, typename Alloc = tt::Allocator<T>>
class Vector {

    size_t m_size {0};
    size_t m_capacity {0};
    T* memory {nullptr};
    Alloc allocator;

    using AllocTraits = std::allocator_traits<Alloc>;

    void checkSize() {
        if(m_size >= m_capacity) {
            reserve(m_capacity == 0 ? 2 : m_capacity * 2);
        }
    }

    void clear() {
        for(size_t i = 0; i < m_size; ++i) {
            AllocTraits::destroy(allocator, memory+i);
        }
        AllocTraits::deallocate(allocator, memory, m_size);
    }
public:

    ~Vector() noexcept {
        clear();
    }
    Vector() : allocator(Alloc()) {
    }

    Vector(size_t count) :
        allocator(Alloc()) {
        reserve(count);
    }
    Vector(size_t count, const T& value, const Alloc &alloc = Alloc()) :
        allocator(alloc) {
        assign(count, value);
    }


    void reserve(size_t count) {

        if(count <= 0 || count < m_capacity) {
            return;
        }

        T* mem {nullptr};
        size_t constructed {0};
        try {
            mem = allocator.allocate(count);

            if(m_size) {
                for(constructed = 0; constructed < m_size; ++constructed) {
                    allocator.construct(mem + constructed, *(memory+constructed));
                }
            }

            if(m_size > 0) {
                clear();
            }

            m_capacity = count;
            memory = mem;

        }  catch (...) {

            for(size_t i = 0; i < constructed; ++i) {
                allocator.destroy(memory+i);
            }

            allocator.deallocate(mem, count);
            throw;
        }
    }

    void assign(size_t count, const T& value) {
        if (m_size > 0) {
            clear();
        }

        reserve(count);
        size_t constructed;
        try {
            for(constructed = 0; constructed < count;++constructed) {
                allocator.construct(memory + constructed, value);
            }

            m_size = count;
        }  catch (...) {
            for (size_t i = 0; i < constructed; ++i) {
                allocator.destroy(memory);
            }
            throw;
        }
    }

    size_t size() const {
        return m_size;
    }

    size_t capacity() const {
        return m_capacity;
    }

    T& operator[](size_t index) const {
        return *(memory + index);
    }

    T& at(size_t index) const {
        if(index > m_size - 1) {
            throw std::out_of_range("index out of range.");
        }
        return *(memory + index);
    }

    void push_back(const T& value) {
        emplace_back(value);
    }

    void push_back(T&& value) {
        emplace_back(std::move(value));
    }

    template<typename... Args>
    void emplace_back(Args&& ... args) {
        checkSize();

        try {
            allocator.construct(memory + m_size++, std::forward<Args>(args)...);
        }  catch (...) {
            allocator.destroy(memory + m_size);
            throw;
        }
    }
};


}
#endif


