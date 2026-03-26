// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _MATH_H
#define _MATH_H

typedef float float_t;
typedef double double_t;

#define HUGE_VALF 0.0f
#define HUGE_VAL  0.0
#define HUGE_VALL 0.0l

#define INFINITY  (1.0 / 0.0)

#define NAN       (INFINITY * 0.0)

#define FP_FAST_FMAF true
#define FP_FAST_FMA  true
#define FP_FAST_FMAL true

#define FP_ILOGB0(x)   ilogb(x)
#define FP_ILOGBNAN(x) ilogb(x)

#define MATH_ERRNO     1
#define MATH_ERREXCEPT 2
#define math_errhandling MATH_ERRNO

#define FP_NORMAL    1
#define FP_SUBNORMAL 2
#define FP_ZERO      4
#define FP_INFINITE  8
#define FP_NAN       16

float       fabsf(float arg);
double      fabs (double arg);
long double fabsl(long double arg);

float       fmodf(float x, float y);
double      fmod (double x, double y);
long double fmodl(long double x, long double y);

float       remainderf(float x, float y);
double      remainder (double x, double y);
long double remainderl(long double x, long double y);

float       remquof(float x, float y, int* quo);
double      remquo (double x, double y, int* quo);
long double remquol(long double x, long double y, int* quo);

float       fmaf(float x, float y, float z);
double      fma (double x, double y, double z);
long double fmal(long double x, long double y, long double z);

float       fmaxf(float x, float y);
double      fmax (double x, double y);
long double fmaxl(long double x, long double y);

float       fminf(float x, float y);
double      fmin (double x, double y);
long double fminl(long double x, long double y);

float       fdimf(float x, float y);
double      fdim (double x, double y);
long double fdiml(long double x, long double y);

float       nanf(const char* arg);
double      nan (const char* arg);
long double nanl(const char* arg);

float       expf(float arg);
double      exp (double arg);
long double expl(long double arg);

float       exp2f(float arg);
double      exp2 (double arg);
long double exp2l(long double arg);

float       expm1f(float arg);
double      expm1 (double arg);
long double expm1l(long double arg);

float       logf(float arg);
double      log (double arg);
long double logl(long double arg);

float       log10f(float arg);
double      log10 (double arg);
long double log10l(long double arg);

float       log2f(float arg);
double      log2 (double arg);
long double log2l(long double arg);

float       log1pf(float arg);
double      log1p (double arg);
long double log1pl(long double arg);

float       powf(float base, float exponent);
double      pow (double base, double exponent);
long double powl(long double base, long double exponent);

float       sqrtf(float arg);
double      sqrt (double arg);
long double sqrtl(long double arg);

float       cbrtf(float arg);
double      cbrt (double arg);
long double cbrtl(long double arg);

float       hypotf(float x, float y);
double      hypot (double x, double y);
long double hypotl(long double x, long double y);

float       sinf(float arg);
double      sin (double arg);
long double sinl(long double arg);

float       cosf(float arg);
double      cos (double arg);
long double cosl(long double arg);

float       tanf(float arg);
double      tan (double arg);
long double tanl(long double arg);

float       asinf(float arg);
double      asin (double arg);
long double asinl(long double arg);

float       acosf(float arg);
double      acos (double arg);
long double acosl(long double arg);

float       atanf(float arg);
double      atan (double arg);
long double atanl(long double arg);

float       atan2f(float y, float x);
double      atan2 (float y, float x);
long double atan2l(long double y, long double x);

float       sinhf(float arg);
double      sinh (double arg);
long double sinhl(long double arg);

float       coshf(float arg);
double      cosh (double arg);
long double coshl(long double arg);

float       tanhf(float arg);
double      tanh (double arg);
long double tanhl(long double arg);

float       asinhf(float arg);
double      asinh (double arg);
long double asinhl(long double arg);

float       acoshf(float arg);
double      acosh (double arg);
long double acoshl(long double arg);

float       atanhf(float arg);
double      atanh (double arg);
long double atanhl(long double arg);

float       erff(float arg);
double      erf (double arg);
long double erfl(long double arg);

float       erfcf(float arg);
double      erfc (double arg);
long double erfcl(long double arg);

float       tgammaf(float arg);
double      tgamma (double arg);
long double tgammal(long double arg);

float       lgammaf(float arg);
double      lgamma (double arg);
long double lgammal(long double arg);

float       ceilf(float arg);
double      ceil (double arg);
long double ceill(long double arg);

float       floorf(float arg);
double      floor (double arg);
long double floorl(long double arg);

float       truncf(float arg);
double      trunc (double arg);
long double truncl(long double arg);

float       roundf  (float arg);
double      round   (double arg);
long double roundl  (long double arg);
long        lroundf (float arg);
long        lround  (double arg);
long        lroundl (long double arg);
long long   llroundf(float arg);
long long   llround (double arg);
long long   llroundl(long double arg);

float       nearbyintf(float arg);
double      nearbyint (double arg);
long double nearbyintl(long double arg);

float       rintf  (float arg);
double      rint   (double arg);
long double rintl  (long double arg);
long        lrintf (float arg);
long        lrint  (double arg);
long        lrintl (long double arg);
long long   llrintf(float arg);
long long   llrint (double arg);
long long   llrintl(long double arg);

float       frexpf(float arg, int* exp);
double      frexp (double arg, int* exp);
long double frexpl(long double arg, int* exp);

float       ldexpf(float arg, int exp);
double      ldexp (double arg, int exp);
long double ldexpl(long double arg, int exp);

float       modff(float arg, float* iptr);
double      modf (double arg, double* iptr);
long double modfl(long double arg, long double* iptr);

float       scalbnf(float arg, int exp);
double      scalbn (double arg, int exp);
long double scalbnl(long double arg, int exp);
float       scalblnf(float arg, long exp);
double      scalbln (double arg, long exp);
long double scalblnl(long double arg, long exp);

int ilogbf(float arg);
int ilogb (double arg);
int ilogbl(long double arg);

float       logbf(float arg);
double      logb (double arg);
long double logbl(long double arg);

float       nextafterf (float from, float to);
double      nextafter  (double from, double to);
long double nextafterl (long double from, long double to);
float       nexttowardf(float from, long double to);
double      nexttoward (double from, long double to);
long double nexttowardl(long double from, long double to);

float       copysignf(float x, float y);
double      copysign (double x, double y);
long double copysignl(long double x, long double y);

#define fpclassify(arg)     FP_NORMAL
#define isfinite(arg)       false
#define isinf(arg)          false
#define isnan(arg)          false
#define isnormal(arg)       false
#define signbit(arg)        0
#define isgreater(x,y)      false
#define isgreaterequal(x,y) false
#define isless(x,y)         false
#define islessequal(x,y)    false
#define islessgreater(x,y)  false
#define isunordered(x,y)    false

#endif
