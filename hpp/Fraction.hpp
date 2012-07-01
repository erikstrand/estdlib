//==============================================================================
// Fraction.hpp
// Created 9/4/11.
//==============================================================================

#ifndef FRACTION_T
#define FRACTION_T

#include "emath.h"


//==============================================================================
// Class Fraction<T>
//==============================================================================

// Represents a fraction of two numbers.
/*
 * Assumptions:
 * T is fully ordered (required by gcd)
 *
 * Implementation:
 * The fraction is only put into reduced form when necessary (certain comparisons),
 * or when the user calls reduce().
 * Reduced form means the gcd is factored out and the denominator is positive.
 * Nothing has been added to keep track of overflow or division by zero.
 * Hence the user must ensure the denominator will not become zero (or a zero divisor),
 * and that the numerator and denominator do not become too large.
 *
 * Currently the standard is to pass eligible Fraction<T>'s as const&'s, but to pass
 * T's themselves by value.
 * 
 * Theory:
 * Let D be a nonempty subset of a commutative Ring R with the following two properties:
 * 1) D does not contain 0 or any zero divisors
 * 2) D is closed under multiplication
 * Then there is a commutative ring Q containing D as a subring with the following properties:
 * 1) every element of D is a unit in Q (ie for every d in D there exists d^-1 in Q st d.d^-1 = 1)
 * 2) every element of Q is representable in the form r.d^-1 (with r in R, d in D)
 * 3) any other ring containing R in which all elements of D are units also contains Q
 * Note that property 1 of Q implies that if D = R \ {0} then Q is a field.
 * See Dummit and Foote Section 7.5 for more details and a proof.
 */
template <class T>
class Fraction {
private:
   // These are only mutable so that reduce() can be const.
   // All other methods are designated const or not as if these were not mutable.
   // (ex operator+= is certainly not const, even though it now technically could be)
   mutable T _numerator;
   mutable T _denominator;

public:
   // Constructors
   Fraction () {}
   Fraction (T t): _numerator(t), _denominator(1) {}
   Fraction (T numerator, T denominator): _numerator(numerator), _denominator(denominator) {}
   
   // Assignment
   Fraction& operator= (T t) { _numerator = t; _denominator = 1; return *this; }
   Fraction& operator= (Fraction const& f) { _numerator = f._numerator; _denominator = f._denominator; return *this; }
   
   // Basic Access
   T numerator () const { return _numerator; }
   T denominator () const { return _denominator; }

   // Reduction and Inversion
   void reduce () const;
   inline Fraction& invert ();
   inline Fraction inverse () const;

   // Field Operations
   inline Fraction operator+ (Fraction const& f) const;
   inline Fraction operator- (Fraction const& f) const;
   inline Fraction operator* (Fraction const& f) const;
   inline Fraction operator/ (Fraction const& f) const;

   // In-Place Field Operations
   Fraction& operator+= (Fraction const& f) { return *this = *this + f; }
   Fraction& operator-= (Fraction const& f) { return *this = *this - f; }
   Fraction& operator*= (Fraction const& f) { return *this = *this * f; }
   Fraction& operator/= (Fraction const& f) { return *this = *this / f; }

   // Comparisons
   inline bool operator== (Fraction const& f) const;
   inline bool operator!= (Fraction const& f) const { return !(*this == f); }
   bool operator<  (Fraction const& f) const;
   bool operator<= (Fraction const& f) const;
   bool operator>  (Fraction const& f) const;
   bool operator>= (Fraction const& f) const;
   inline T comparisonHelper (Fraction const& f) const;

   // Other
   inline T floor () const;    // greatest whole number less than the fraction
   inline T ceiling () const;  // least whole number greater than the fraction
   inline float castToFloat() const;  // if (float)T works, this works
};


//==============================================================================
// Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
// Reduction and Inversion
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
template <class T>
void Fraction<T>::reduce () const {
   if (_numerator == 0) {
      _denominator = 1;
      return;
   }
   if (_denominator < 0) {
      _numerator *= -1;
      _denominator *= -1;
   }
   T gcd = findgcd(_numerator, _denominator);
   if (gcd != 1) {
      _numerator /= gcd;
      _denominator /= gcd;
   }
}

//------------------------------------------------------------------------------
template <class T>
Fraction<T>& Fraction<T>::invert () {
   T temp = _numerator;
   _numerator = _denominator;
   _denominator = _numerator;
   return *this;
}

//------------------------------------------------------------------------------
template <class T>
Fraction<T> Fraction<T>::inverse () const {
   return Fraction<T>(_denominator, _numerator);
}


//------------------------------------------------------------------------------
// Field Operations
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
template <class T>
Fraction<T> Fraction<T>::operator+ (Fraction const& f) const {
   T lcm = findlcm(_denominator, f._denominator);
   return Fraction<T>(_numerator * (lcm / _denominator) + f._numerator * (lcm / f._denominator), lcm);
}

//------------------------------------------------------------------------------
template <class T>
Fraction<T> Fraction<T>::operator- (Fraction const& f) const {
   T lcm = findlcm(_denominator, f._denominator);
   return Fraction<T>(_numerator * (lcm / _denominator) - f._numerator * (lcm / f._denominator), lcm);
}

//------------------------------------------------------------------------------
template <class T>
Fraction<T> Fraction<T>::operator* (Fraction const& f) const {
   return Fraction<T>(_numerator * f._numerator, _denominator * f._denominator);
}

//------------------------------------------------------------------------------
template <class T>
Fraction<T> Fraction<T>::operator/ (Fraction const& f) const {
   return *this * f.inverse();
}


//------------------------------------------------------------------------------
// Comparisons
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
template <class T>
bool Fraction<T>::operator== (Fraction const& f) const {
   reduce();
   f.reduce();
   return (_numerator == f._numerator and _denominator == f._denominator);
}

//------------------------------------------------------------------------------
template <class T>
bool Fraction<T>::operator< (Fraction const& f) const {
   T lcm = comparisonHelper(f);
   return (_numerator * (lcm / _denominator) < f._numerator * (lcm / f._denominator));
}

//------------------------------------------------------------------------------
template <class T>
bool Fraction<T>::operator<= (Fraction const& f) const {
   T lcm = comparisonHelper(f);
   return (_numerator * (lcm / _denominator) <= f._numerator * (lcm / f._denominator));
}

//------------------------------------------------------------------------------
template <class T>
bool Fraction<T>::operator> (Fraction const& f) const {
   T lcm = comparisonHelper(f);
   return (_numerator * (lcm / _denominator) > f._numerator * (lcm / f._denominator));
}

//------------------------------------------------------------------------------
template <class T>
bool Fraction<T>::operator>= (Fraction const& f) const {
   T lcm = comparisonHelper(f);
   return (_numerator * (lcm / _denominator) >= f._numerator * (lcm / f._denominator));
}

//------------------------------------------------------------------------------
template <class T>
T Fraction<T>::comparisonHelper (Fraction const& f) const {
   // I'm not sure if reduction here is a good idea.
   // Pro: lcm could overflow. Con: pretty slow.
//   reduce();
//   f.reduce();
   return findlcm(_denominator, f._denominator);
}


//------------------------------------------------------------------------------
// Other
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
template <class T>
T Fraction<T>::floor () const {
   T divisor = _numerator / _denominator;
   if (_numerator < 0)
      return divisor -1;
   return divisor;
}

//------------------------------------------------------------------------------
template <class T>
T Fraction<T>::ceiling () const {
   return floor() + 1;
}

//------------------------------------------------------------------------------
// If this were a real typecast regular field operations could be interpreted incorrectly
template <class T>
float Fraction<T>::castToFloat () const {
   return (float)_numerator / (float)_denominator;
}


//==============================================================================
// Global Functions
//==============================================================================

//------------------------------------------------------------------------------
// Field Operations
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
template <class T>
Fraction<T> operator+ (Fraction<T> const& f, T t) {
   return Fraction<T>(f.numerator() + t*f.denominator(), f.denominator());
}

//------------------------------------------------------------------------------
template <class T>
Fraction<T> operator- (Fraction<T> const& f, T t) {
   return Fraction<T>(f.numerator() - t*f.denominator(), f.denominator());
}

//------------------------------------------------------------------------------
template <class T>
Fraction<T> operator* (Fraction<T> const& f, T t) {
   return Fraction<T>(f.numerator() * t, f.denominator());
}

//------------------------------------------------------------------------------
template <class T>
Fraction<T> operator/ (Fraction<T> const& f, T t) {
   return Fraction<T>(f.numerator(), f.denominator() * t);
}

//------------------------------------------------------------------------------
template <class T> Fraction<T> operator+ (T t, Fraction<T> const& f) { return f+t; }
template <class T> Fraction<T> operator- (T t, Fraction<T> const& f) { return f-t; }
template <class T> Fraction<T> operator* (T t, Fraction<T> const& f) { return f*t; }
template <class T> Fraction<T> operator/ (T t, Fraction<T> const& f) { return f/t; }


//------------------------------------------------------------------------------
// Comparisons
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
template <class T>
bool operator== (Fraction<T> const& f, T t) {
   f.reduce();
   return (f.denominator() == 1 and f.numerator() == t);
}

//------------------------------------------------------------------------------
template <class T> bool operator!= (Fraction<T> const& f, T t) { return !(f==t); }

//------------------------------------------------------------------------------
template <class T>
bool operator< (Fraction<T> const& f, T t) {
   return (f.numerator() < t * f.denominator());
}

//------------------------------------------------------------------------------
template <class T>
bool operator<= (Fraction<T> const& f, T t) {
   return (f.numerator() <= t * f.denominator());
}

//------------------------------------------------------------------------------
template <class T>
bool operator> (Fraction<T> const& f, T t) {
   return (f.numerator() > t * f.denominator());
}

//------------------------------------------------------------------------------
template <class T>
bool operator>= (Fraction<T> const& f, T t) {
   return (f.numerator() >= t * f.denominator());
}

//------------------------------------------------------------------------------
template <class T> bool operator== (T t, Fraction<T> const& f) { return (f==t); }
template <class T> bool operator!= (T t, Fraction<T> const& f) { return (f!=t); }
template <class T> bool operator<  (T t, Fraction<T> const& f) { return (f< t); }
template <class T> bool operator<= (T t, Fraction<T> const& f) { return (f<=t); }
template <class T> bool operator>  (T t, Fraction<T> const& f) { return (f> t); }
template <class T> bool operator>= (T t, Fraction<T> const& f) { return (f>=t); }


#endif // FRACTION_T
