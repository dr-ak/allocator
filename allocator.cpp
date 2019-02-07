#include <iostream>
#include <vector>
#include <map>

template<typename T, std::size_t Size = 1>
struct cast_map_allocator {
    using value_type = T;

    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using void_pointer = void*;

    std::size_t _count = 0;
    pointer _head;

    template<typename U>
    struct rebind {
        using other = cast_map_allocator<U, Size>;
    };

    cast_map_allocator() = default;
    ~cast_map_allocator() = default;

    template<typename U> 
    cast_map_allocator(const cast_map_allocator<U, Size>&) {

    }

    pointer allocate(std::size_t n) {
        ++_count;
        if (_count == 1){
            auto p = std::malloc(Size * sizeof(T));
            if (!p)
                throw std::bad_alloc();
            _head = reinterpret_cast<T *>(p);
            return _head;
        } else {
            return _head + _count - 1;
        }
    }

    void deallocate(pointer p, std::size_t n) {
            std::free(_head);
            _head = nullptr;
    }

    template<typename U, typename ...Args>
    void construct(U *p, Args &&...args) {
        new(p) U(std::forward<Args>(args)...);
    }

    void destroy(T *p) {
        p->~T();
    }
};

template <typename T, typename Allocator = std::allocator<T>>
struct container
{
    struct _node{
        T _val;
        std::size_t _key;
        _node* _next;
    };

    using _alloc_t = typename Allocator:: template rebind<_node>::other;

    _node* _head = nullptr;
    _node* _tail = nullptr;
    std::size_t _count = 0;
    _alloc_t _alloc;

    void push_back(T val)
    {
        _node* tmp = _alloc.allocate(1);
        tmp->_val = val;
        tmp->_key = _count;
        ++_count;
        tmp->_next = nullptr;

        if (_head == nullptr) {
            _head = tmp;
            _tail = tmp;
        } else {
            _tail->_next = tmp;
            _tail = tmp;
        }
    }

    T get(std::size_t key){
        _node* p = _head;

        while (p->_key != key)
            p = p->_next;

        return p->_val;
    }

    ~container(){
        _node* p = _head;
        while (p != nullptr){
            _head = _head->_next;
            _alloc.deallocate(p, 1);
            p = _head;
        }
    }
};

unsigned factorial(unsigned number){
    unsigned out = 1;
    for (unsigned i = 1; i <= number; ++i)
        out = out * i;
    return out;
}

int main(int, char *[]) {

    auto m = std::map<int, int>{};
    for (unsigned i = 0; i < 10; ++i)
        m[i] = factorial(i);

    auto mca = std::map<int, int, std::less<int>, 
         cast_map_allocator<std::pair<const int, int>, 10>>{};
    for (unsigned i = 0; i < 10; ++i)
        mca[i] = factorial(i);

    for (unsigned i = 0; i < 10; ++i)
        std::cout << i << " " << m[i] << std::endl;

    std::cout << std::endl;

    for (unsigned i = 0; i < 10; ++i)
        std::cout << i << " " << mca[i] << std::endl;

    std::cout << std::endl;


    auto c = container<int>{};
    for (unsigned i = 0; i < 10; ++i)
        c.push_back(factorial(i));

    auto cca = container<int, cast_map_allocator<int, 10>>{};
    for (unsigned i = 0; i < 10; ++i)
        cca.push_back(factorial(i));

    for (unsigned i = 0; i < 10; ++i)
        std::cout << i << " " << c.get(i) << std::endl;

    std::cout << std::endl;

    for (unsigned i = 0; i < 10; ++i)
        std::cout << i << " " << cca.get(i) << std::endl;

    std::cout << "\n**** end of main ****\n" << std::endl;

    return 0;
}
