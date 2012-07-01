//==============================================================================
// emath.h
// Created 9/4/11.
//==============================================================================

#ifndef EMATH
#define EMATH


//==============================================================================
// Function Declarations
//==============================================================================

//------------------------------------------------------------------------------
// Finds the greates common divisor of two numbers.
/*
 * Assumptions:
 * T is fully ordered
 *
 * Implementation:
 * Uses the Euclidean Algorithm. Note that this algorithm can in general be
 * applied to any Euclidean Domain, so our assumption that T is fully ordered
 * is stricter than necessary. It always returns the positive gcd.
 */
template <class T>
T findgcd (T n1, T n2);

//------------------------------------------------------------------------------
// Finds the least common multiple of two numbers.
/* It relies on the gcd function defined above, so T must be a fully ordered set.
 * It always returns the positive lcm.
 */
template <class T>
T findlcm (T n1, T n2) {
   T gcd = findgcd(n1, n2);
   T result = n1 / gcd * n2;
   if (result < 0)
      result *= -1;
   return result;
}


//==============================================================================
// Template Functions Definitions
//==============================================================================

//------------------------------------------------------------------------------
template <class T>
T findgcd (T n1, T n2) {
   if (n1 < 0) {
      n1 *= -1;
   }
   if (n2 < 0) {
      n2 *= -1;
   }
   while (n1 != n2) {
      if (n2 > n1) {
         T temp = n1;
         n1 = n2;
         n2 = temp;
      }
      n1 -= n2;
   }
   return n1;
}


#endif // EMATH
