#ifndef analysis_utils_task_scan_params_hpp
#define analysis_utils_task_scan_params_hpp

#include <analysis_utils/task/config.hpp>
#include <analysis_utils/task/parameter_list.hpp>
#include <analysis_utils/progress_bar.hpp>
// stdlib
#include <array>
#include <atomic> 
#include <cstddef> 
#include <iostream> 
#include <stdexcept> 
#include <thread> 
#include <mutex> 

namespace analysis_utils
{
namespace task
{

template<typename F, typename Data> void scan_params(param_list<Data> list, F&& fcn, task::config_t cfg)
{

    const step n_tasks = list.get_n_steps(); 
     if (n_tasks < 1) {
        std::cerr << "in <scan_params<F,Data>: no points to scan!\n"; 
        return; 
    }
    //keeps track of tasks. put this in its own cache register
    alignas(64) std::atomic<step> task_id{0}; 

    if (cfg.n_threads == 0) { cfg.n_threads = std::thread::hardware_concurrency(); }

    std::vector<std::thread> threads; threads.reserve(cfg.n_threads); 

    if (cfg.chunk_size) cfg.chunk_size = std::max<std::size_t>( 1, n_tasks / (20 * cfg.n_threads)); 

    if (cfg.verbosity==1) std::cout << "\nStarting "<< n_tasks<<" tasks...\n"; 

    std::mutex write_mutex; 
    progress_bar pbar{cfg.progress_bar_size}; 
    for (size_t t=0; t<cfg.n_threads; t++) {

        threads.emplace_back([&fcn, &task_id, n_tasks, cfg, t, &list, &pbar, &write_mutex]{

            alignas(64) auto t_list = list; 

            while (1) {
                step start = task_id.fetch_add(cfg.chunk_size, std::memory_order_relaxed); 
                step end   = std::min<step>( n_tasks, start + cfg.chunk_size ); 

                //quit if all tasks have already been scheduled
                if (start >= n_tasks) break;    

                t_list.set_step(start); 

                while (t_list.get_step() < end) {

                    fcn(&t_list.get(), t);
                    ++t_list; 
                }
                
                if (cfg.verbosity == 1) {
                    std::lock_guard<std::mutex> lock(write_mutex); 
                    std::cout << "\r" << pbar(((float)std::min<std::size_t>(task_id,n_tasks))/((float)n_tasks)) << std::flush; 
                }
            }; 
        }); 
    }

    for (auto& thread : threads) thread.join(); 

    if (cfg.verbosity >= 1) std::cout << "\ndone.\n"; 
}

}
}

#endif