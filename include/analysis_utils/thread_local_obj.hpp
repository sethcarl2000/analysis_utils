#ifndef analysis_utils_thread_local_obj_hpp
#define analysis_utils_thread_local_obj_hpp

#include <cstddef> 
#include <vector>
#include <memory> 
#include <mutex> 
#include <thread> 

namespace analysis_utils
{

class thread_pool; 

template<typename T> class thread_local_obj {
private:        

    static constexpr std::size_t cache_line_size = 64; 

    friend class thread_pool; 

    struct alignas(cache_line_size) Slot {
        T obj; 
        explicit Slot(const T& _val) : obj(_val) {}; 
    };
    std::vector<Slot> objects{}; 

    std::size_t n_threads; 
    
public:

    thread_local_obj(const T& _cpy, std::size_t _n=0) : n_threads{_n} { 
        if (n_threads < 1) n_threads = std::thread::hardware_concurrency(); 
        objects.reserve(n_threads); 
        for (std::size_t t=0; t<n_threads; t++) objects.emplace_back(_cpy); 
    }

    T& operator()(std::size_t i) { return objects[i].obj; }

    static_assert(std::is_copy_constructible_v<T>, "Template paramter 'T' is not copy-constructible"); 
};

}

#endif