#ifndef analysis_utils_thread_local_obj_hpp
#define analysis_utils_thread_local_obj_hpp

#include <cstddef> 
#include <vector>
#include <memory> 
#include <thread> 

namespace analysis_utils
{

class thread_pool; 

template<typename T> class thread_local_obj {
private:        

    friend class thread_pool; 

    const T* copy_source; 
    
    alignas(64) std::vector<std::unique_ptr<T>> objects{};

    std::size_t n_threads; 
    
public:

    thread_local_obj(const T& _cpy, std::size_t _n=0) : copy_source{&_cpy}, n_threads{_n} { 
        if (n_threads < 1) n_threads = std::thread::hardware_concurrency(); 
        objects = std::vector<std::unique_ptr<T>>(n_threads); 
    }

    T& operator()(int i) { 
        auto& obj = objects[i]; 
        if (!obj) obj = std::make_unique<T>(*copy_source); 
        return *(obj.get());     
    }

    static_assert(std::is_copy_constructible_v<T>, "Template paramter 'T' is not copy-constructible"); 
};

}

#endif