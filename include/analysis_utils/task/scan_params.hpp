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
    const bool has_parameters = (list.get_n_steps() > 0); 

    const step n_tasks = has_parameters ? list.get_n_steps() * cfg.n_scans : cfg.n_scans; 
     if (n_tasks < 1) {
        std::cerr << "in <scan_params<F,Data>: no points to scan!\n"; 
        return; 
    }
    //keeps track of tasks. put this in its own cache register
    alignas(64) std::atomic<step> task_id{0}; 

    if (cfg.n_threads == 0) { cfg.n_threads = std::thread::hardware_concurrency(); }

    std::vector<std::thread> threads; threads.reserve(cfg.n_threads); 

    if (cfg.chunk_size < 1) cfg.chunk_size = std::max<std::size_t>( 1, n_tasks / (20 * cfg.n_threads)); 
    
    std::mutex write_mutex;
    
    progress_bar pbar{cfg.progress_bar_size}; 
    if (cfg.verbosity == 1) {
        if (cfg.verbosity==1) std::cout << "\nStarting "<< n_tasks<<" tasks...\n"; 
        std::cout << "\r" << pbar(0.) << std::flush; 
    }


    for (size_t t=0; t<cfg.n_threads; t++) {

        threads.emplace_back([fcn, &task_id, n_tasks, cfg, t, &list, &pbar, &write_mutex, has_parameters]{

            alignas(64) auto t_list = list; 

            Data default_dat{}; 
            while (1) { 
                step start = task_id.fetch_add(cfg.chunk_size, std::memory_order_relaxed); 
                step end   = std::min<step>( n_tasks, start + cfg.chunk_size ); 

                //quit if all tasks have already been scheduled
                if (start >= n_tasks) break;    

                t_list.set_step(start); 

                if (cfg.verbosity >= 2) {
                    write_mutex.lock(); 
                    std::cout << "thread " << t << " executing task range [" << start << "," << end << "]\n" << std::flush; 
                    write_mutex.unlock(); 
                } 

                do {
                    if (has_parameters) {
                        fcn(&t_list.get(), t);
                        ++t_list; 
                    } else {
                        fcn(&default_dat, t);
                    }
                } while (++start < end); 
                
                if (cfg.verbosity == 1) {
                    write_mutex.lock(); 
                    std::cout << "\r" << pbar(((float)std::min<std::size_t>(task_id,n_tasks))/((float)n_tasks)) << std::flush; 
                    write_mutex.unlock(); 
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