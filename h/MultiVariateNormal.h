//==============================================================================
// MultiVariateNormal.h
// Created 4/28/12.
//==============================================================================

#ifndef ESTDLIB_MULTI_VARIATE_NORMAL
#define ESTDLIB_MULTI_VARIATE_NORMAL

#include "Random.h"
#include "SMatrix.hpp"


//==============================================================================
// Class MVNGen
//==============================================================================

template <class Gen>
class MVNGen {
   Gen* _gen;
   SMatrix<double> _covariance;
};

//------------------------------------------------------------------------------
// Inline Method Definitions
//------------------------------------------------------------------------------


#endif // ESTDLIB_MULTI_VARIATE_NORMAL
