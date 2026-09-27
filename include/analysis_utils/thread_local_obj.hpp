#ifndef analysis_utils_thread_local_obj_hpp
#define analysis_utils_thread_local_obj_hpp

#include <array>

namespace analysis_utils
{

template<int N> class thread_pool; 

template<typename T, int N> class thread_local_obj {
private:     

    friend class thread_pool<N>; 
    std::array<T, N> fObjects; 
public: 

    thread_local_obj() { fObjects.fill(T{}); }
    T& operator()(int i) { return fObjects[i]; }

    static_assert(std::is_default_constructible_v<T>); 
};

}

#endif