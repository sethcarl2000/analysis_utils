#ifndef analysis_utils_root_unique_TObject_copy_hpp
#define analysis_utils_root_unique_TObject_copy_hpp

// ROOT headers
#include <TObject.h>
#include <TClass.h>
#include <TH1.h>
#include <TString.h> 
// stdlib
#include <memory> 
#include <string>
#include <sstream> 

namespace analysis_utils
{
namespace root
{

/// @brief Given an Object of type 'T' which inherits from TObject, return a 'clone' (copy), managed by the user via a unique ptr. 
/// @tparam T Class type 
/// @param source object to copy
/// @param new_name name use. if none is provided then new_name = source->GetName() + "_copy"
/// @return std::unique_ptr<T>, corresponding to copy. 
template<typename T> std::unique_ptr<T> unique_TObject_copy(T* source, std::string new_name="")
{
    static_assert(std::is_base_of<TObject, T>::value, "Input object must inherit from ROOT's TObject parent class"); 

    if (new_name.empty()) {
        new_name = std::string{source->GetName()} + "_copy"; 
    }

    auto cpy = std::unique_ptr<T>(dynamic_cast<T*>(source->Clone(new_name.c_str()))); 

    cpy->SetBit(kMustCleanup); 
    cpy->ResetBit(kCanDelete); 

    if ( TClass::GetClass<T>()->InheritsFrom(TClass::GetClass<TH1>()) ) {
        cpy->SetDirectory(nullptr); 
    }
    
    return cpy; 
}

}
}


#endif