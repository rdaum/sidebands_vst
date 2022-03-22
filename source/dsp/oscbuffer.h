#pragma once

#include <valarray>

namespace sidebands {

using OscBuffer = std::valarray<double>;
using OscParam = OscBuffer;

OscBuffer Vcos(const OscBuffer &src);
OscBuffer Vexp(const OscBuffer &src);
OscBuffer Vsin(const OscBuffer &src);

OscBuffer Vmul(const OscBuffer &l, const OscBuffer &r);
OscBuffer Vdiv(const OscBuffer &l, const OscBuffer &r);
OscBuffer Vsub(const OscBuffer &l, const OscBuffer &r);
OscBuffer Vadd(const OscBuffer &l, const OscBuffer &r);

void VaddInplace(OscBuffer &l, const OscBuffer &r);
void VmulInplace(OscBuffer &l, const OscBuffer &r);
void VdivInplace(OscBuffer &l, const OscBuffer &r);
void VsubInplace(OscBuffer &l, const OscBuffer &r);

OscBuffer Vmul(const OscBuffer &l, double r);
OscBuffer Vdiv(const OscBuffer &l, double r);
OscBuffer Vsub(const OscBuffer &l, double r);
OscBuffer Vadd(const OscBuffer &l, double r);

void VaddInplace(OscBuffer &l, double r);
void VmulInplace(OscBuffer &l, double r);
void VdivInplace(OscBuffer &l, double r);
void VsubInplace(OscBuffer &l, double r);

void ToFloat(const OscBuffer &src, float *out);

void linspace(OscBuffer &linspaced, double start, double end, size_t num);

OscBuffer weighted_exp(size_t N, double start, double end, double weight);

inline double zapgremlins(double x) {
  double absx = std::abs(x);
  // very small numbers fail the first test, eliminating denormalized numbers
  //    (zero also fails the first test, but that is OK since it returns zero.)
  // very large numbers fail the second test, eliminating infinities
  // Not-a-Numbers fail both tests and are eliminated.
  return (absx > (double)1e-15 && absx < (double)1e15) ? x : (double)0.;
}

}  // namespace sidebands