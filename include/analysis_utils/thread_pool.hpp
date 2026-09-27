#ifndef analysis_utils_thread_pool_hpp
#define analysis_utils_thread_pool_hpp

#include <analysis_utils/thread_local_obj.hpp> 
#include <analysis_utils/progress_bar.hpp> 
// stdlib
#include <array>
#include <atomic> 
#include <cstddef> 
#include <thread> 
#include <mutex> 

namespace analysis_utils
{

template<int N> class thread_pool {
public: 

    thread_pool() = default; 

    thread_pool(const thread_pool&) = delete; 
    thread_pool& operator=(const thread_pool&) = delete; 

    /// @brief Make thread-local copies of the given object. 
    /// @tparam T type of object
    /// @param obj template to make copies of for each thread
    /// @return 'thread_local_obj' object, which gives each thread access to its own object
    template<typename T> thread_local_obj<T,N> MakeThreadLocalObj(const T& obj) {
        static_assert(std::is_copy_assignable_v<T>, "Thread-local object requested is not copy-assignable"); 
        thread_local_obj<T,N> tlocal{}; 
        //make copies of the input object for each thread
        for (int i=0; i<N; i++) tlocal.fObjects[i] = T(obj);
        return tlocal; 
    } 

    /// @brief Make thread-local copies of object (with unique instructions for each thread)
    /// @tparam T type of object to create
    /// @param ctor function which takes thread id as arg, and returns thread-local object
    /// @return 'thread_local_obj' object, which gives each thread access to its own object
    template<typename T> thread_local_obj<T,N> MakeThreadLocalObj(const std::function<T(int)>& ctor) {
        thread_local_obj<T,N> tlocal{}; 
        //make copies of the input object for each thread
        for (int i=0; i<N; i++) tlocal.fObjects[i] = ctor(i); 
        return tlocal; 
    } 

    template<typename F, typename Data> void ParallelFor(std::vector<Data>& data, const F& fcn, std::size_t chunk_size=0)
    {
        const size_t n_tasks = data.size(); 
        if (n_tasks < 1) return; 
        //keeps track of tasks
        alignas(64) std::atomic<std::size_t> task_id{0}; 

        std::array<std::thread, N> threads;

        if (chunk_size<1) chunk_size = std::max<std::size_t>( 1, n_tasks / (32 * N)); 

        if (fVerbosity==1) std::cout << "\nStarting "<< n_tasks<<" tasks...\n"; 

        std::mutex write_mutex; 
        progress_bar pbar{100}; 
        for (size_t t=0; t<N; t++) {

            threads[t] = std::thread([&fcn, &task_id, n_tasks, chunk_size, t, &data, &pbar, &write_mutex, this]{

                while (1) {
                    std::size_t start = task_id.fetch_add(chunk_size, std::memory_order_relaxed); 
                    std::size_t end   = std::min<std::size_t>( n_tasks, start + chunk_size ); 

                    //quit if all tasks have already been scheduled
                    if (start >= n_tasks) break;    

                    for (std::size_t index=start; index<end; index++) { fcn(&data[index], t); }
                    
                    if (fVerbosity == 1) {
                        std::lock_guard<std::mutex> lock(write_mutex); 
                        std::cout << "\r" << pbar(((float)std::min<std::size_t>(task_id,n_tasks))/((float)n_tasks)) << std::flush; 
                    }
                }; 
            }); 
        }

        for (auto& thread : threads) thread.join(); 
        std::cout << "\ndone.\n"; 
    }

private:    

    /// @brief Verbosity with which to print statements. 1=progress bar, 0=quiet. 
    int fVerbosity=1; 

};

}

#endif