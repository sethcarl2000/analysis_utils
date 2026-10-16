#ifndef analysis_utils_parameter_list_hpp
#define analysis_utils_parameter_list_hpp

#include <analysis_utils/task/step.hpp>
// stdlib
#include <vector> 
#include <stdexcept>

namespace analysis_utils
{
namespace task 
{


template<typename TData> class param_list {
public: 

    struct param_t {
        double TData::*field{nullptr}; 
        double min, max, dx; 
        step n_steps; 
    
        double operator()(step i) const { return min + ((double)i)*dx; }
    };

private: 

    step fStep{}; 

    std::vector<param_t> fParams{}; 

    std::vector<step> fParam_indices{};
    std::vector<step> fParam_step_sizes{1}; 

    void compute_param_step_sizes() {
        step st=1; 
        fParam_step_sizes[0] = st; 
        for (std::size_t i=0; i<Size(); i++) { 
            st *= fParams[i].n_steps; 
            fParam_step_sizes[i+1] = st;
        }  
    }

    void compute_param_indices() {
        // For an overview of how this indexing scheme works:   
        // 
        // Suppose we have 4 parameters, each with 10 steps. that would mean there are 10,000 steps total, starting with step '0' and ending with step '9,999'
        // gtttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt (thanks, muon.)
        // We want to assemble a vector of parameters at a certain step, say step '9271'. 
        //
        //  First, we take the list of 'fStepSizes': { 1, 10, 100, 1,000 }
        //  
        //  And we use it to fetch our parameters. Written out explicitly: 
        //  rank = fParams.size()-1; (rank=3)
        //  step = 9721
        //  for (rank=3; rank>=0)
        //    output[3] = fParams[3].get_step(9721 / 1000 = 9); 
        //    step = 9721 % 1000 (step=721);  
        //    --rank (3 -> 2);
        //  } for: (2 >= 0)
        //    output[2] = fParams[2].get_step(721 / 100 = 7); 
        //    step = 721 % 100 (step=21);  
        //    --rank (2 -> 1);
        //  } for: (1 >= 0);   
        //    output[1] = fParams[1].get_step(21 / 10 = 2); 
        //    step = 21 % 10 (step=1);  
        //    --rank (1 -> 0);
        //  } for: (0 >= 0);   
        //    output[0] = fParams[0].get_step(1 / 1 = 1); 
        //    step = 1 % 1 (step=0);  
        //    --rank (0 -> -1);
        //  } for: (-1 >= 0) (break);   
        //std::cout << "step: " << fStep << " \% " << get_n_steps() << " = " << (fStep % get_n_steps()) << " indices:\n"; 
        
        for (int rank=Size()-1; rank>=0; rank--) {
            const auto& par = fParams[rank]; 
            fData.*(par.field) = par(fParam_indices[rank] = fStep / fParam_step_sizes[rank]); 
            fStep = fStep % fParam_step_sizes[rank];
        } 
        /*int r=0; 
        for (const auto& ii : fParam_indices) std::cout << "    i["<<r++<<"]="<<ii;
        //std::cout << "\n"; */ 
    }

    void increment_step() {

        ++fStep; 
        /*int r=0; 
        for (const auto& ii : fParam_indices) std::cout << "    i["<<r++<<"]="<<ii;
        std::cout << "\n";  */ 
        //keep increasing rank, until we find an index which hasn't 'turned over' 
        for (int rank=0; rank<Size(); rank++) { 
            auto& index = fParam_indices[rank]; 
            auto &par = fParams[rank]; 
            if ((++index) < par.n_steps) { 
                //std::cout << "i["<<rank<<"]="<<index<<"\n";
                fData.*(par.field) = par(index); 
                return; 
            } else {
                index =0;
                //std::cout << "i["<<rank<<"]="<<index<<"    ";
                fData.*(par.field) = par(index); 
            }
        }
        // we could only get here if we've 'looped over' to the beginning
        fStep =0; 
    }

    TData fData{}; 

public: 

    param_list() = default; 

    //default copy-ctor
    param_list& operator=(const param_list&) = default; 

    std::vector<std::size_t> get_step_sizes() const { return fParam_step_sizes; } 
    std::vector<std::size_t> get_indices() const { return fParam_indices; } 
    
    param_list& add(double TData::*field, double min, double max, step n_steps) 
    {
        if (n_steps < 2) {
            throw std::invalid_argument("in <param_list<T>::ctor>: parameter must have at least 2 steps");
            return *this; 
        }
        fParams.emplace_back(param_t{
            field, 
            min, max, (max-min)/((double)(n_steps-1)), n_steps
        }); 
        fStep =0; 
        fParam_indices.resize(Size(), 0); 
        fParam_step_sizes.resize(Size()+1, 1);
        compute_param_step_sizes();  
        compute_param_indices(); 
        return *this; 
    }; 

    std::size_t Size() const { return fParams.size(); }

    std::size_t get_n_steps() const { return fParam_step_sizes.empty() ? 0 : fParam_step_sizes.back(); }

    void set_step(step new_step) {
        if (get_n_steps()==0) return;  
        fStep = new_step % get_n_steps(); 
        compute_param_indices(); 
    }

    step get_step() const { return fStep; }

    param_list& operator++() {
        increment_step(); return *this; 
    }

    const TData& get() const { return fData; }
}; 

}
}

#endif