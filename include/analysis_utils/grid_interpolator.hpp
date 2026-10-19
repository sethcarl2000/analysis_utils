#ifndef analysis_utils_grid_interpolator_hpp
#define analysis_utils_grid_interpolator_hpp

#include <cmath> 

namespace analysis_utils
{

/*
*   Multilinear grid interpolation. 
*   Basically, given a number of dimensions 'N', consider a N-dimensional hypercube. 
*   As it turns out, if you know the value for some function at each of the vertices of the hypercube, and you want to  
*   Linearly interpolate in the interior of the cube, you can just perorm recursive interpolations to lower and lower dimensions
*   Until you get to 1D. 
*
*   Example:  let x,y,z \in [0,1].  
*
*   z0 ---|---- z1 
*         ^ <- x 
*   let z(x) ~= F(z0,z1;x) = z1 + (z1 - z0)*x 
*
*
*   z10 ------- z11
*    |           |
*    |           |
*    |  + <-x,y  | 
*    |           |  
*   z00 ------- z01
*   
*  then: 
*  z(x,y) ~= F( F(z00,z01,x), F(z10,z11,x), y )
*                ^ recursion! 
*
*  And so forth for 3D. 
*
*/

template<int N> int grid_interpolator_stride_size()
{
    static_assert(N >= 1, "Dimension of grid must be >= 1"); 

    if constexpr (N == 1) { return 1; }
    else { return grid_interpolator_stride_size<N-1>() * 2; }
}

/// @brief Perform basic grid interpolation. See code body for documentation. 
/// @tparam N Dimension of grid. 1=1D linear interpolation, 2=2D grid, etc. 
/// @param Z Function values to interpolate between (vertices of the hypercube). Array is indexed as binary, i.e., for a 3D cube, Z[0] is the x=y=z=0 point, Z[1] is x=1 & y=z=0, Z[3] is x=y=1 & z=0, and Z[7] is x=y=z=1.
/// @param X normalized 'arg' for each axis (coordinate within hypercube, where each side-length is 1). For a 3D cube, z = X[0], and x = X[2] in above convention. x=0 is the Z[0] point, and x=1 is the Z[1] point.  
/// @return 
template<int N> double grid_interpolate(const double* Z, const double* X)
{
    static_assert(N >= 1, "Dimension of grid must be >= 1"); 

    if constexpr (N==1) {
        return Z[0] + (Z[1] - Z[0])*X[0];
    } else {
        int stride = grid_interpolator_stride_size<N>(); 
        
        //perform recursive interpolation for the lower-rank dimensions. 
        // this function is called recursively, until we reach the lowest-level 1d interpolation. 
        double z1 = grid_interpolate<N-1>(Z+stride, X+1); 
        double z0 = grid_interpolate<N-1>(Z+0,      X+1); 

        return z0 + (z1 - z0)*X[0]; 
    }
}

};

#endif