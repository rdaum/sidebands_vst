#pragma once

#include <complex>
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

struct LeakDC  {
  explicit LeakDC(double b = 0.999) : b1_(b) {}
  void Filter(OscBuffer &buf);
  double b1_;
  double x1_ = 0.0;
  double y1_ = 0.0;
};

struct Integrator {
  explicit Integrator(double b = 0.999) : b1_(b) {}
  void Filter(OscBuffer &buf, double b = 0.999);
  double b1_ = 0.0;
  double y1_ = 0.f;
};


}  // namespace sidebands