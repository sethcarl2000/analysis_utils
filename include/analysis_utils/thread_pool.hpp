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

class thread_pool {
private: 
    std::size_t n_threads; 

public: 

    thread_pool(std::size_t _n_threads=0) : n_threads{_n_threads} {
        if (n_threads < 1) n_threads = std::thread::hardware_concurrency(); 
    }

    void set_verbosity(int v) { fVerbosity=v; }
    
    thread_pool(const thread_pool&) = delete; 
    thread_pool& operator=(const thread_pool&) = delete; 

    /// @brief Make thread-local copies of the given object. 
    /// @tparam T type of object
    /// @param obj template to make copies of for each thread
    /// @return 'thread_local_obj' object, which gives each thread access to its own object
    template<typename T> thread_local_obj<T> MakeThreadLocalObj(const T& obj) {
        static_assert(std::is_copy_assignable_v<T>, "Thread-local object requested is not copy-assignable"); 
        thread_local_obj<T> tlocal(obj, n_threads); 
        //make copies of the input object for each thread
        return tlocal; 
    } 

    template<typename F, typename Data> void ParallelFor(std::vector<Data>& data, F&& fcn, std::size_t chunk_size=0)
    {
        const size_t n_tasks = data.size(); 
        if (n_tasks < 1) return; 
        //keeps track of tasks
        alignas(64) std::atomic<std::size_t> task_id{0}; 

        std::vector<std::thread> threads; threads.reserve(n_threads); 

        if (chunk_size<1) chunk_size = std::max<std::size_t>( 1, n_tasks / (32 * n_threads)); 

        if (fVerbosity==1) std::cout << "\nStarting "<< n_tasks<<" tasks...\n"; 

        std::mutex write_mutex; 
        progress_bar pbar{100}; 
        for (size_t t=0; t<n_threads; t++) {

            threads.emplace_back([fcn, &task_id, n_tasks, chunk_size, t, &data, &pbar, &write_mutex, this]{

                while (1) {
                    std::size_t start = task_id.fetch_add(chunk_size, std::memory_order_relaxed); 
                    std::size_t end   = std::min<std::size_t>( n_tasks, start + chunk_size ); 

                    //quit if all tasks have already been scheduled
                    if (start >= n_tasks) break;    

                    if (fVerbosity >= 2) {
                        std::lock_guard<std::mutex> lock(write_mutex); 
                        std::cout << "thread " << t << " executing task range [" << start << "," << end << "]\n" << std::flush;
                    }
                    //perform the loop
                    for (; start<end; start++) { fcn(&data[start], t); }
                    
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