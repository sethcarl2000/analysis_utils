#ifndef analysis_utils_task_config_hpp
#define analysis_utils_task_config_hpp

#include <cstddef> 

namespace analysis_utils
{
namespace task
{

struct config_t {

    /// @brief Number of threads to use. 0==hardware concurrency
    std::size_t n_threads{0};

    /// @brief Number of 'tasks' for each thread to digest at a time. 
    std::size_t chunk_size{32}; 

    /// @brief Verbosity. 1==print progress bar 
    int verbosity=1; 

    /// @brief Progress bar size. 
    int progress_bar_size=100; 
};

}
}



#endif