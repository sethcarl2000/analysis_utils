#ifndef analysis_utils_fft_computer_hpp
#define analysis_utils_fft_computer_hpp

#include <cmath> 
#include <vector> 
#include <memory> 
#include <cstddef> 
#include <complex> 
#include <cstdio> 

// I based this implemenetation on the description of Radix-2 from: 
//      https://www.phys.uconn.edu/~rozman/Courses/m3511_19s/downloads/radix2fft.pdf
//
// - Seth H (21 Oct 2026) 

namespace analysis_utils
{

/// @brief This is a Discrete Fourier Transform (DFT) computuer, applicable for a finite set of evenly-spaced points. It uses the Radix-2 Fast Fourier Transform algorithm to expediate calculations.
class fft_computer {
private: 

    
    std::size_t N; 

    std::unique_ptr<fft_computer> child{nullptr}; 
    std::vector<std::complex<double>> coeffs{}, conj_coeffs{}; 

    std::vector<std::complex<double>> buffer_E{}, buffer_O{}; 

    std::complex<double> step_size; 

    bool is_lowest_child{false}; 


    void compute_fft(const std::complex<double>* Xn, std::complex<double>* Xk, std::size_t start, std::size_t stride) {
        
        if (is_lowest_child) {

            // if we're the lowest child, then compute the FFT the normal way
            for (std::size_t k=0; k<N; k++) {
                auto& xk = Xk[k]; xk =0.; 
                for (std::size_t m=0; m<N; m++) {
                    xk += Xn[m*stride + start] * coeffs[k*N + m]; 
                }
            }

        } else {    

            // if not, do it recursively 
            child->compute_fft(Xn, buffer_E.data(), start,          stride*2); 
            child->compute_fft(Xn, buffer_O.data(), start + stride, stride*2); 

            for (std::size_t k=0; k<N; k++) {
                std::size_t k_mod_N2 = k % (N/2); 
                Xk[k] = buffer_E[k_mod_N2] + std::exp( +step_size*((double)k) )*buffer_O[k_mod_N2]; 
            }
        }
    }; 


    void compute_fft_inverse(const std::complex<double>* Xk, std::complex<double>* Xn, std::size_t start, std::size_t stride) {
        if (is_lowest_child) {

            // if we're the lowest child, then compute the FFT the normal way
            for (std::size_t k=0; k<N; k++) {
                auto& xn = Xn[k]; xn =0.; 
                for (std::size_t m=0; m<N; m++) {
                    xn += Xk[m*stride + start] * conj_coeffs[k*N + m]; 
                }
            }

        } else {    

            // if not, do it recursively 
            child->compute_fft_inverse(Xk, buffer_E.data(), start,          stride*2); 
            child->compute_fft_inverse(Xk, buffer_O.data(), start + stride, stride*2); 

            for (std::size_t k=0; k<N; k++) {
                std::size_t k_mod_N2 = k % (N/2); 
                Xn[k] = buffer_E[k_mod_N2] + std::exp( -step_size*((double)k) )*buffer_O[k_mod_N2]; 
            }
        }
    }; 

public: 

    /// @brief Compute a DFT with evenly-spaced points using the Radix-2 algorithm. Basically, it splits the total number of points in half as many times as possible, and computes the DFT recursively. 
    /// @param n Number of points. 
    fft_computer(std::size_t n) : N{n} {
        
        using namespace std::complex_literals;   

        step_size = -1i * 2.*3.1415926536/((double)N); 
        
        std::printf("creating FFT cptr with N=%zu. is lowest child? %s\n",
            N, (N % 2 != 0 || N <= 2) ? "yes" : "no"
        );

        // if we're the bottom-most child (we can't divide by any more powers of 2)
        if (N <= 2 || N % 2 != 0) {

            is_lowest_child = true; 
            //compute the coefficients
            coeffs.reserve(N*N); conj_coeffs.reserve(N*N); 
            for (std::size_t k=0; k<N; k++) {
                for (std::size_t n=0; n<N; n++) {
                    coeffs     .emplace_back( std::exp( +step_size*((double)(n*k)) ) ); 
                    conj_coeffs.emplace_back( std::exp( -step_size*((double)(n*k)) ) ); 
                }
            }
        
        // if we can split this FFT into smaller pieces, let's do that. 
        } else {

            is_lowest_child = false; 
            child = std::make_unique<fft_computer>(N/2); 

            buffer_E = std::vector<std::complex<double>>(N/2, 0.); 
            buffer_O = std::vector<std::complex<double>>(N/2, 0.); 
        }   
    }; 

    //copy ctor
    explicit fft_computer(const fft_computer& src) : fft_computer(src.N) {};  
    
    /// @brief Compute the Discrete Fourier Transform (position space -> reciprocal space)
    /// @param Xn evenly-spaced position-space points. 
    /// @param Xk evenly-spaced reciprocal-space points. 
    void compute(const std::complex<double>* Xn, std::complex<double>* Xk) {

        compute_fft(Xn, Xk, 0, 1); 
    }; 

    /// @brief Compute the inverse Discrete Fourier Transform (reciprocal space -> position space)
    /// @param Xk evenly-spaced reciprocal-space points.
    /// @param Xn evenly-spaced position-space points.  
    void compute_inverse(const std::complex<double>* Xk, std::complex<double>* Xn) {

        compute_fft_inverse(Xk, Xn, 0, 1); 
        double norm = 1./((double)N);
        for (std::size_t i=0; i<N; i++) Xn[i] *= norm; 
    }; 
};


};

#endif