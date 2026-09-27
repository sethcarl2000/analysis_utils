#ifndef analysis_utils_progress_bar_hpp
#define analysis_utils_progress_bar_hpp

#include <chrono> 
#include <string> 
#include <mutex> 
#include <sstream> 
#include <cmath>
#include <cstdio> 
#include <iostream> 

namespace analysis_utils
{

class progress_bar {
private: 
    std::chrono::time_point<std::chrono::system_clock> fStart; 
    int fN_points{100}; 

public: 
    
    progress_bar(int n_pts=100) : fN_points{n_pts}, fStart{std::chrono::system_clock::now()} {}; 

    std::string operator()(float fraction) {

        using duration = std::chrono::duration<double, std::milli>; 
        
        //get the current duration 
        auto now = std::chrono::system_clock::now(); 
        
        auto seconds = duration{ now - fStart }.count() / 1.e3;

        std::ostringstream oss; 
        oss  << "["; 
        double n_steps_d = (double)fN_points;
        int n_ticks_full = ((double)fN_points)*std::min<float>( 1., fraction );

        for (int i=0; i<fN_points; i++) { oss << (i<=n_ticks_full ? "=" : " "); }

        char buff[100]; std::sprintf(buff, "%4.1f%%", fraction*100.); 
        oss << "]    " << buff; 
        
        int minutes = std::floor( seconds / 60. ); 
        seconds = seconds - minutes*60; 
        std::sprintf(buff, "   elapsed: (%02i:%02.0f)", minutes, seconds); 
        oss << buff; 

        return oss.str(); 
    }

};

}


#endif