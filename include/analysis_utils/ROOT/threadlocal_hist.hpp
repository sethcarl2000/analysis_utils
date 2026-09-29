#ifndef analysis_utils_root_threadlocal_hist_hpp
#define analysis_utils_root_threadlocal_hist_hpp

#include <analysis_utils/ROOT/unique_TObject_copy.hpp>
// ROOT 
#include <TAxis.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h> 
#include <TClass.h> 
// stdlib
#include <memory> 
#include <vector> 

namespace analysis_utils
{
namespace root
{

template<typename Hist> class threadlocal_hist
{
    static_assert(std::is_base_of<TH1, Hist>::value, "Histogram type must derive from ROOT's TH1 histogram parent class"); 

private: 
    
    //template to make copies of 
    Hist* fTemplate{nullptr}; 

    // histograms for each thread 
    alignas(64) std::vector<std::unique_ptr<Hist>> fHistograms; 
    
    void combine1D(TH1* src, TH1* dest) {
        auto xax = src->GetXaxis(); 
        const size_t xbins = xax->GetNbins(); 
        for (int ix=1; ix<=xbins; ix++) {
            double x = xax->GetBinCenter(ix);  
            dest->Fill( x, src->GetBinContent(ix) ); 
        }
    } 

    void combine2D(TH1* _src, TH1* _dest) {
        auto src = dynamic_cast<TH2*>(_src); 
        auto dest = dynamic_cast<TH2*>(_dest); 
        auto xax = src->GetXaxis(); 
        auto yax = src->GetYaxis(); 
        const size_t xbins = xax->GetNbins(); 
        const size_t ybins = yax->GetNbins(); 
        for (int ix=1; ix<=xbins; ix++) {
            double x = xax->GetBinCenter(ix);  
            for (int iy=1; iy<=ybins; iy++) {
                double y = yax->GetBinCenter(iy); 
                dest->Fill( x,y, src->GetBinContent(ix,iy) ); 
            }
        }
    } 

    void combine3D(TH1* _src, TH1* _dest) {
        auto src = dynamic_cast<TH3*>(_src); 
        auto dest = dynamic_cast<TH3*>(_dest); 
        auto xax = src->GetXaxis(); 
        auto yax = src->GetYaxis(); 
        auto zax = src->GetZaxis(); 
        const size_t xbins = xax->GetNbins(); 
        for (int ix=1; ix<=xbins; ix++) {
            double x = xax->GetBinCenter(ix);  
            const size_t ybins = yax->GetNbins(); 
            for (int iy=1; iy<=ybins; iy++) {
                double y = yax->GetBinCenter(iy); 
                const size_t zbins = zax->GetNbins(); 
                for (int iz=1; iz<=zbins; iz++) {
                    double z = zax->GetBinCenter(iz); 
                    dest->Fill( x,y,z, src->GetBinContent(ix,iy,iz) ); 
                }
            }
        }
    } 

public: 

    threadlocal_hist(Hist* _h=nullptr, std::size_t _n_threads=1) : fTemplate{_h} { 
        fHistograms = std::vector<std::unique_ptr<Hist>>(_n_threads); 
    }

    Hist* operator()(std::size_t i) { 
        auto& hist = fHistograms[i]; 
        if (!hist) {
            hist.reset(); 
            hist = root::unique_TObject_copy<Hist>(fTemplate); 
        }
        return hist.get(); 
    }

    Hist* aggregate() 
    {
        //the 0-th histogram will take data from all others
        auto hist = fTemplate; 

        if ( TClass::GetClass<Hist>()->InheritsFrom(TClass::GetClass<TH3>()) ) {
            for (auto& h : fHistograms) { if (h) combine3D(h.get(), hist); }
        } else {
            if ( TClass::GetClass<Hist>()->InheritsFrom(TClass::GetClass<TH2>()) ) {
                for (auto& h : fHistograms) { if (h) combine2D(h.get(), hist); }
            } else {
                for (auto& h : fHistograms) { if (h) combine1D(h.get(), hist); }
            }
        }
        //delete histograms 
        fHistograms.clear();

        return hist; 
    }
};

}
}

#endif