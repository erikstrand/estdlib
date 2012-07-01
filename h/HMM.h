//==============================================================================
// HMM.h
// Created 12/5/22
//==============================================================================

#ifndef ESTDLIB_HMM
#define ESTDLIB_HMM

#include "Matrix.hpp"
#include "BitField.h"


//==============================================================================
// Class HMM
//==============================================================================

typedef float prob;

class HMM {
public:
   struct Message {
      prob* _m;
      Message (): _m(0) {}
      void resize (unsigned s) { if (_m) delete[] _m; _m = new prob[s]; }
      ~Message () { if (_m) delete[] _m; }
      prob& operator[] (unsigned i) { return _m[i]; }
      prob const& operator[] (unsigned i) const { return _m[i]; }
      void zero (unsigned s) { memset(_m, 0, sizeof(prob) * s); }
   };

private:
   static const prob _BIG  = 1e7;
   static const prob _BIGI = 1e-7;
   unsigned _n;      // length of chain
   unsigned _s;      // latent variables take values in [0, ..., _s)
   unsigned _o;      // observables take values in [0, ..., _o)
   Matrix<prob> _t;  // _t[i][j] = p( z_{n+1} = s_j | z_n = s_i ) for all n
   Matrix<prob> _e;  // _e[i][j] = p( x_n = o_j | z_n = s_i ) for all n
   prob* _p;         // prior (marginal) distribution for z_0

   BitField _obsC;   // which observables are conditioned on
   unsigned* _obs;   // observations of conditioned observables
   BitField _stateC; // which states are conditioned on
   unsigned* _state; // states of conditioned states

   Message* _f;      // _n - 1 forward Messages, _f[i] is Message from z_i to z_{i+1}
   Message* _b;      // _n - 1 backward Messages, _b[i] is Message from z_{i+1} to z_i
   int* _fn;         // normalization of forward messages
   int* _bn;         // normalization of backward messages
   prob _normalize;  // normalization constant
   unsigned _npow;   // normalization constant includes _BIG raised to this power

public:
   inline HMM ();
   inline ~HMM ();
   void resize (unsigned n, unsigned s, unsigned o);

   inline void conditionObservable (unsigned n, unsigned o);
   inline void conditionState (unsigned n, unsigned o);
   void unConditionObservable (unsigned n) { _obsC.unset(n); }
   void unConditionState (unsigned n) { _stateC.unset(n); }

   void calculateMessages ();
   // returns the marginal distribution on latent variable n
   void unNormalizedMarginal (unsigned n, prob* p);
   void marginal (unsigned n, prob* p);
   // calculates the single most likely path given a complete set of observations
   prob viterbiPath (unsigned* bestPath);

   unsigned n () const { return _n; }
   unsigned s () const { return _s; }
   unsigned o () const { return _o; }
   Matrix<prob>& t () { return _t; }
   Matrix<prob>& e () { return _e; }
   prob* p () { return _p; }
   unsigned const* observations () const { return _obs; }
   unsigned const* states () const { return _state; }
   prob normalizationConstant () const { return _normalize; }

   void printMessages () const;
   void printConditions () const;

   void calculateLogMessages ();
   void unNormalizedLogMarginal (unsigned n, prob* p);
   void logMarginal (unsigned n, prob* p);
   prob logSum (prob x, prob y);
};


//==============================================================================
// Inline Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
HMM::HMM ()
: _n(0), _s(0), _o(0), _t(), _e(), _p(0),
_obsC(), _obs(0), _stateC(), _state(0),
_f(0), _b(0), _fn(0), _bn(0), _normalize(1.0), _npow(0)
{}

//------------------------------------------------------------------------------
HMM::~HMM () {
   if (_f) delete[] _f;
   if (_b) delete[] _b;
   // should delete more!
}

//------------------------------------------------------------------------------
void HMM::conditionObservable (unsigned n, unsigned o) {
   _obsC.set(n);
   _obs[n] = o;
}

//------------------------------------------------------------------------------
void HMM::conditionState (unsigned n, unsigned o) {
   _stateC.set(n);
   _state[n] = o;
}


#endif // ESTDLIB_HMM
