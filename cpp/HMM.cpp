//==============================================================================
// HMM.cpp
// Created 12/5/22
//==============================================================================

#include "HMM.h"
#include <iostream>
#include <cmath>

using namespace std;


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
void HMM::resize (unsigned n, unsigned s, unsigned o) {
   _n = n;
   _s = s;
   _o = o;
   _t.resize(_s, _s);
   _e.resize(_s, _o);
   if (_p) delete[] _p;
   _p = new prob[_s];

   if (_f) { delete[] _f; delete[] _fn; }
   if (_b) { delete[] _b; delete[] _bn; }
   _f = new Message[_n - 1];
   _b = new Message[_n - 1];
   _fn = new int[_n - 1];
   _bn = new int[_n - 1];
   for (unsigned n=0; n<_n-1; ++n) {
      _f[n].resize(_s);
      _b[n].resize(_s);
   }

   _obsC.resize(_n);
   _stateC.resize(_n);
   if (_obs) delete[] _obs;
   if (_state) delete[] _state;
   _obs = new unsigned[_n];
   _state = new unsigned[_n];
}

//------------------------------------------------------------------------------
void HMM::calculateMessages () {
   // Zero all messages
   for (unsigned n=0; n<_n-1; ++n) {
      _f[n].zero(_s);
      _b[n].zero(_s);
   }

   prob messageSum;

   // Calculate backward message from the last node
   if (_stateC.get(_n-1)) {
      unsigned s = _state[_n-1];
      if (_obsC.get(_n-1)) {
         unsigned o = _obs[_n-1];
         for (unsigned i=0; i<_s; ++i) {
            _b[_n-2][i] = _t[i][s] * _e[s][o];
         }
      } else {
         for (unsigned i=0; i<_s; ++i) {
            _b[_n-2][i] = _t[i][s];
         }
      }
   } else {
      if (_obsC.get(_n-1)) {
         unsigned o = _obs[_n-1];
         for (unsigned i=0; i<_s; ++i) {
            for (unsigned j=0; j<_s; ++j) {
               _b[_n-2][i] += _t[i][j] * _e[j][o];
            }
         }
      } else {
         for (unsigned i=0; i<_s; ++i) {
            _b[_n-2][i] = 1.0;
         }
      }
   }
   _bn[_n-2] = 0;  // we haven't started normalizing yet

   // Calculate other backward messages
   // (each loop calculates message from z_n to z_{n-1})
   for (unsigned n=_n-2; n>0; --n) {
      if (_stateC.get(n)) {
         unsigned s = _state[n];
         if (_obsC.get(n)) {
            unsigned o = _obs[n];
            for (unsigned i=0; i<_s; ++i) {
               _b[n-1][i] = _t[i][s] * _b[n][s] * _e[s][o];
            }
         } else {
            for (unsigned i=0; i<_s; ++i) {
               _b[n-1][i] = _t[i][s] * _b[n][s];
            }
         }
      } else {
         if (_obsC.get(n)) {
            unsigned o = _obs[n];
            for (unsigned i=0; i<_s; ++i) {
               for (unsigned j=0; j<_s; ++j) {
                  _b[n-1][i] += _t[i][j] * _b[n][j] * _e[j][o];
               }
            }
         } else {
            for (unsigned i=0; i<_s; ++i) {
               for (unsigned j=0; j<_s; ++j) {
                  _b[n-1][i] += _t[i][j] * _b[n][j];
               }
            }
         }
      }

      // check normalization
      messageSum = 0;
      _bn[n-1] = _bn[n];
      for (unsigned i=0; i<_s; ++i)
         messageSum += _b[n-1][i];
      if (messageSum < _BIGI) {
         ++_bn[n-1];
         for (unsigned i=0; i<_s; ++i)
            _b[n-1][i] *= _BIG;
      }
   }

   // Calculate forward message from root
   if (_stateC.get(0)) {
      unsigned s = _state[0];
      if (_obsC.get(0)) {
         unsigned o = _obs[0];
         for (unsigned i=0; i<_s; ++i) {
            _f[0][i] = _t[s][i] * _p[s] * _e[s][o];
         }
      } else {
         for (unsigned i=0; i<_s; ++i) {
            _f[0][i] = _t[s][i] * _p[s];
         }
      }
   } else {
      if (_obsC.get(0)) {
         unsigned o = _obs[0];
         for (unsigned i=0; i<_s; ++i) {
            for (unsigned j=0; j<_s; ++j) {
               _f[0][i] += _t[j][i] * _p[j] * _e[j][o];
            }
         }
      } else {
         for (unsigned i=0; i<_s; ++i) {
            for (unsigned j=0; j<_s; ++j) {
               _f[0][i] += _t[j][i] * _p[j];
            }
         }
      }
   }
   _fn[0] = 0;  // we haven't started normalizing yet

   // Calculate other forward messages
   for (unsigned n=1; n<_n-1; ++n) {
      if (_stateC.get(n)) {
         unsigned s = _state[n];
         if (_obsC.get(n)) {
            unsigned o = _obs[n];
            for (unsigned i=0; i<_s; ++i) {
               _f[n][i] = _t[s][i] * _f[n-1][s] * _e[s][o];
            }
         } else {
            for (unsigned i=0; i<_s; ++i) {
               _f[n][i] = _t[s][i] * _f[n-1][s];
            }
         }
      } else {
         if (_obsC.get(n)) {
            unsigned o = _obs[n];
            for (unsigned i=0; i<_s; ++i) {
               for (unsigned j=0; j<_s; ++j) {
                  _f[n][i] += _t[j][i] * _f[n-1][j] * _e[j][o];
               }
            }
         } else {
            for (unsigned i=0; i<_s; ++i) {
               for (unsigned j=0; j<_s; ++j) {
                  _f[n][i] += _t[j][i] * _f[n-1][j];
               }
            }
         }
      }

      // check normalization
      messageSum = 0;
      _fn[n] = _fn[n-1];
      for (unsigned i=0; i<_s; ++i)
         messageSum += _f[n][i];
      if (messageSum < _BIGI) {
         ++_fn[n];
         for (unsigned i=0; i<_s; ++i)
            _f[n][i] *= _BIG;
      }
   }

   // calculate the normalization constant
   prob p[_s];
   unsigned nindex = 0;
   // we have to find the marginal distribution for a variable that is not
   // conditioned on.
   for (unsigned i=0; i<_n; ++i) {
      if (!_stateC.get(i)) {
         nindex = i;
         unNormalizedMarginal(i, p);
         break;
      }
   }
   prob total = 0.0;
   for (unsigned i=0; i<_s; ++i)
      total += p[i];
   cout << "total prob = " << total << " (for variable " << nindex << ")" << '\n';
   _normalize = 1.0 / total;

   if (nindex == 0) _npow = _bn[0];
   else if (nindex == _n - 1) _npow = _fn[_n-2];
   else _npow = _fn[nindex-1] + _bn[nindex];
}

//------------------------------------------------------------------------------
void HMM::unNormalizedMarginal (unsigned n, prob* p) {
   // Note: we're not checking if z_n has been conditioned on.
   // This method is used for determining the normalization constant
   // and should not be called on conditioned variables.

   if (n == 0) {
      for (unsigned i=0; i<_s; ++i) {
         p[i] = _p[i] * _b[0][i];
      }
   } else if (n == _n-1) {
      for (unsigned i=0; i<_s; ++i) {
         p[i] = _f[_n-2][i];
      }
   } else {
      for (unsigned i=0; i<_s; ++i) {
         p[i] = _f[n-1][i] * _b[n][i];
      }
   }

   // Include emission factors if we have conditioned on x_n
   if (_obsC.get(n)) {
      unsigned o = _obs[n];
      for (unsigned i=0; i<_s; ++i) {
         p[i] *= _e[i][o];
      }
   }
}

//------------------------------------------------------------------------------
void HMM::marginal (unsigned n, prob* p) {
   // if we've conditioned z_n our job is easy
   if (_stateC.get(n)) {
      unsigned s = _state[n];
      for (unsigned i=0; i<_s; ++i)
         p[i] = 0.0;
      p[s] = 1.0;
      return;
   }

   // otherwise calculate the marginal
   unNormalizedMarginal(n, p);

   // normalize
   int bigPower;
   if (n == 0) bigPower = _bn[0];
   else if (n == _n - 1) bigPower = _fn[_n-2];
   else bigPower = _fn[n-1] + _bn[n];
   bigPower -= _npow;

   float renorm = _normalize;
   if      (bigPower > 0) renorm *= pow(_BIGI, bigPower);
   else if (bigPower < 0) renorm *= pow(_BIG, -bigPower);
   for (unsigned i=0; i<_s; ++i)
      p[i] *= renorm;
}

//------------------------------------------------------------------------------
// Assumes all observables are conditioned
prob HMM::viterbiPath (unsigned* bestPath) {
   Matrix<prob> probs(_n, _s);         // we actually store log probs
   Matrix<unsigned> paths(_n-1, _s);

   // calculate initial probabilities
   cout << _obs[0] << '\n';
   for (unsigned j=0; j<_s; ++j) {
      probs[0][j] = log(_e[j][_obs[0]]) + log(_p[j]);
      cout << probs[0][j] << '\n';
   }

   // iterate forward through states
   prob temp;
   for (unsigned i=1; i<_n; ++i) {
      for (unsigned j=0; j<_s; ++j) {
         // maximize over final states
         paths[i-1][j] = 0;
         probs[i][j] = log(_t[0][j]) + probs[i-1][0];
         for (unsigned s=1; s<_s; ++s) {
            temp = log(_t[s][j]) + probs[i-1][s];
            if (temp > probs[i][j]) {
               paths[i-1][j] = s;
               probs[i][j] = temp;
            }
         }
         probs[i][j] += log(_e[j][_obs[i]]);
      }
      if (i % 10000 == 0) {
         for (unsigned j=0; j<_s; ++j) {
            cout << "probs[" << i << "][" << j << "] = " << probs[i][j] << ", ";
         }
         cout << '\n';
      }
   }

   // debug printing...
   /*
   for (unsigned i=0; i<_n; ++i) {
      for (unsigned j=0; j<_s; ++j) {
         cout << "probs[" << i << "][" << j << "] = " << probs[i][j] << ", ";
      }
      cout << '\n';
   }
   */

   // backtrack to find Viterbi Path
   bestPath[_n-1] = 0;
   prob bestProb = probs[_n-1][0];
   // find best final state
   for (unsigned s=1; s<_s; ++s) {
      temp = probs[_n-1][s];
      if (temp > bestProb) {
         bestPath[_n-1] = s;
         bestProb = temp;
      }
   }
   // backtrack
   for (unsigned i=_n-2; true; --i) {
      bestPath[i] = paths[i][bestPath[i+1]];
      if (i == 0) break;
   }
   return bestProb;
}

//------------------------------------------------------------------------------
void HMM::printMessages () const {
   for (unsigned n=0; n<_n-1; ++n) {
      cout << "_f[" << n << "] : ";
      for (unsigned i=0; i<_s; ++i) {
         cout << _f[n][i] << ", ";
      }
      cout << '\n';
      cout << "_b[" << n << "] : ";
      for (unsigned i=0; i<_s; ++i) {
         cout << _b[n][i] << ", ";
      }
      cout << '\n';
   }
}

//------------------------------------------------------------------------------
void HMM::printConditions () const {
   cout << "Conditioned Latent Variables:\n";
   for (BitField::ConstIterator itr(_stateC, true); itr.valid(); itr.nextSet())
         cout << itr.i() << " = " << _state[itr.i()] << '\n';
   cout << "Conditioned Observables:\n";
   for (BitField::ConstIterator itr(_obsC, true); itr.valid(); itr.nextSet())
         cout << itr.i() << " = " << _obs[itr.i()] << '\n';
}

//------------------------------------------------------------------------------
void HMM::calculateLogMessages () {
   prob logZero = log(0);

   // Calculate backward message from the last node
   if (_stateC.get(_n-1)) {
      unsigned s = _state[_n-1];
      if (_obsC.get(_n-1)) {
         unsigned o = _obs[_n-1];
         for (unsigned i=0; i<_s; ++i) {
            _b[_n-2][i] = log(_t[i][s]) + log(_e[s][o]);
         }
      } else {
         for (unsigned i=0; i<_s; ++i) {
            _b[_n-2][i] = log(_t[i][s]);
         }
      }
   } else {
      if (_obsC.get(_n-1)) {
         unsigned o = _obs[_n-1];
         for (unsigned i=0; i<_s; ++i) {
            _b[_n-2][i] = logZero;
            for (unsigned j=0; j<_s; ++j) {
               _b[_n-2][i] = logSum( _b[_n-2][i], log(_t[i][j]) + log(_e[j][o]) );
            }
         }
      } else {
         for (unsigned i=0; i<_s; ++i) {
            _b[_n-2][i] = 0.0;
         }
      }
   }

   // Calculate other backward messages
   // (each loop calculates message from z_n to z_{n-1})
   for (unsigned n=_n-2; n>0; --n) {
      if (_stateC.get(n)) {
         unsigned s = _state[n];
         if (_obsC.get(n)) {
            unsigned o = _obs[n];
            for (unsigned i=0; i<_s; ++i) {
               _b[n-1][i] = _b[n][s] + log(_t[i][s]) + log(_e[s][o]);
            }
         } else {
            for (unsigned i=0; i<_s; ++i) {
               _b[n-1][i] = _b[n][s] + log(_t[i][s]);
            }
         }
      } else {
         if (_obsC.get(n)) {
            unsigned o = _obs[n];
            for (unsigned i=0; i<_s; ++i) {
               _b[n-1][i] = logZero;
               for (unsigned j=0; j<_s; ++j) {
                  _b[n-1][i] = logSum( _b[n-1][i], _b[n][j] + log(_t[i][j]) + log(_e[j][o]) );
               }
            }
         } else {
            for (unsigned i=0; i<_s; ++i) {
               _b[n-1][i] = logZero;
               for (unsigned j=0; j<_s; ++j) {
                  _b[n-1][i] = logSum( _b[n-1][i], _b[n][j] + log(_t[i][j]) );
               }
            }
         }
      }
   }

   // Calculate forward message from root
   if (_stateC.get(0)) {
      unsigned s = _state[0];
      if (_obsC.get(0)) {
         unsigned o = _obs[0];
         for (unsigned i=0; i<_s; ++i) {
            _f[0][i] = log(_t[s][i]) + log(_p[s]) + log(_e[s][o]);
         }
      } else {
         for (unsigned i=0; i<_s; ++i) {
            _f[0][i] = log(_t[s][i]) + log(_p[s]);
         }
      }
   } else {
      if (_obsC.get(0)) {
         unsigned o = _obs[0];
         for (unsigned i=0; i<_s; ++i) {
            _f[0][i] = logZero;
            for (unsigned j=0; j<_s; ++j) {
               _f[0][i] = logSum( _f[0][i], log(_t[j][i]) + log(_p[j]) + log(_e[j][o]) );
            }
         }
      } else {
         for (unsigned i=0; i<_s; ++i) {
            _f[0][i] = logZero;
            for (unsigned j=0; j<_s; ++j) {
               _f[0][i] = logSum( _f[0][i], log(_t[j][i]) + log(_p[j]) );
            }
         }
      }
   }
   _fn[0] = 0;  // we haven't started normalizing yet

   // Calculate other forward messages
   for (unsigned n=1; n<_n-1; ++n) {
      if (_stateC.get(n)) {
         unsigned s = _state[n];
         if (_obsC.get(n)) {
            unsigned o = _obs[n];
            for (unsigned i=0; i<_s; ++i) {
               _f[n][i] = log(_t[s][i]) + _f[n-1][s] + log(_e[s][o]);
            }
         } else {
            for (unsigned i=0; i<_s; ++i) {
               _f[n][i] = log(_t[s][i]) + log(_f[n-1][s]);
            }
         }
      } else {
         if (_obsC.get(n)) {
            unsigned o = _obs[n];
            for (unsigned i=0; i<_s; ++i) {
               _f[n][i] = logZero;
               for (unsigned j=0; j<_s; ++j) {
                  _f[n][i] = logSum( _f[n][i], log(_t[j][i]) + _f[n-1][j] + log(_e[j][o]) );
               }
            }
         } else {
            for (unsigned i=0; i<_s; ++i) {
               _f[n][i] = logZero;
               for (unsigned j=0; j<_s; ++j) {
                  _f[n][i] = log(_t[j][i]) + _f[n-1][j];
               }
            }
         }
      }
   }

   // calculate the normalization constant
   prob p[_s];
   unsigned nindex = 0;
   // we have to find the marginal distribution for a variable that is not
   // conditioned on.
   for (unsigned i=0; i<_n; ++i) {
      if (!_stateC.get(i)) {
         nindex = i;
         unNormalizedLogMarginal(i, p);
         break;
      }
   }
   prob total = logZero;
   for (unsigned i=0; i<_s; ++i)
      total = logSum(total, p[i]);
   cout << "total log prob = " << total << " (for variable " << nindex << ")" << '\n';
   _normalize = -total;
}

//------------------------------------------------------------------------------
void HMM::unNormalizedLogMarginal (unsigned n, prob* p) {
   if (n == 0) {
      for (unsigned i=0; i<_s; ++i) {
         p[i] = log(_p[i]) + _b[0][i];
      }
   } else if (n == _n-1) {
      for (unsigned i=0; i<_s; ++i) {
         p[i] = _f[_n-2][i];
      }
   } else {
      for (unsigned i=0; i<_s; ++i) {
         p[i] = _f[n-1][i] + _b[n][i];
      }
   }

   // Include emission factors if we have conditioned on x_n
   if (_obsC.get(n)) {
      unsigned o = _obs[n];
      for (unsigned i=0; i<_s; ++i) {
         p[i] += log(_e[i][o]);
      }
   }
}

//------------------------------------------------------------------------------
void HMM::logMarginal (unsigned n, prob* p) {
   // if we've conditioned z_n our job is easy
   if (_stateC.get(n)) {
      unsigned s = _state[n];
      for (unsigned i=0; i<_s; ++i)
         p[i] = 0.0;
      p[s] = 1.0;
      return;
   }

   unNormalizedLogMarginal(n, p);

   for (unsigned i=0; i<_s; ++i) {
      p[i] = exp( p[i] + _normalize );
   }
}

//------------------------------------------------------------------------------
// assumes x and y have already been logged
// ie logSum( log(x), log(y) ) == log(x + y)
prob HMM::logSum (prob x, prob y) {
   if (x >= y)
      return x + log(1 + exp(y - x) );
   return y + log(1 + exp(x - y) );
}
