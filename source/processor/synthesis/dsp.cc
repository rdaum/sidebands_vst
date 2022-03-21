#include "processor/synthesis/dsp.h"

#include <vectorclass.h>
#include <vectormath_exp.h>
#include <vectormath_trig.h>

#include <algorithm>
#include <functional>

namespace sidebands {

namespace {

OscBuffer VapplyUnary(const OscBuffer &src,
                      const std::function<Vec8d(const Vec8d &)> &f) {
  size_t size(src.size());
  OscBuffer dest(size);
  Vec8d src_vec, dst_vec;
  for (int i = 0; i < size; i += 8) {
    src_vec.load(&(src[i]));
    dst_vec = f(src_vec);
    dst_vec.store(&(dest[i]));
  }
  return dest;
}

void VapplyBinaryInplace(
    OscBuffer &l, const OscBuffer &r,
    const std::function<Vec8d(const Vec8d &, const Vec8d &)> &f) {
  size_t size(l.size());
  Vec8d l_vec, r_vec, dst_vec;
  for (int i = 0; i < size; i += 8) {
    l_vec.load(&(l[i]));
    r_vec.load(&(r[i]));
    dst_vec = f(l_vec, r_vec);
    dst_vec.store(&(l[i]));
  }
}

void VapplyBinaryInplace(OscBuffer &l, double r,
                         const std::function<Vec8d(const Vec8d &, double)> &f) {
  size_t size(l.size());
  Vec8d l_vec, dst_vec;
  for (int i = 0; i < size; i += 8) {
    l_vec.load(&(l[i]));
    dst_vec = f(l_vec, r);
    dst_vec.store(&(l[i]));
  }
}

OscBuffer
VapplyBinary(const OscBuffer &l, const OscBuffer &r,
             const std::function<Vec8d(const Vec8d &, const Vec8d &)> &f) {
  size_t size(l.size());
  OscBuffer dest(size);
  Vec8d l_vec, r_vec, dst_vec;
  for (int i = 0; i < size; i += 8) {
    l_vec.load(&(l[i]));
    r_vec.load(&(r[i]));
    dst_vec = f(l_vec, r_vec);
    dst_vec.store(&(dest[i]));
  }
  return dest;
}

OscBuffer VapplyBinary(const OscBuffer &l, double r,
                       const std::function<Vec8d(const Vec8d &, double)> &f) {
  size_t size(l.size());
  OscBuffer dest(size);
  Vec8d l_vec, dst_vec;
  for (int i = 0; i < size; i += 8) {
    l_vec.load(&(l[i]));
    dst_vec = f(l_vec, r);
    dst_vec.store(&(dest[i]));
  }
  return dest;
}
} // namespace

OscBuffer Vsin(const OscBuffer &src) {
  return VapplyUnary(src, [](const Vec8d &v) { return sin(v); });
}

OscBuffer Vcos(const OscBuffer &src) {
  return VapplyUnary(src, [](const Vec8d &v) { return cos(v); });
}

OscBuffer Vexp(const OscBuffer &src) {
  return VapplyUnary(src, [](const Vec8d &v) { return exp(v); });
}

OscBuffer Vmul(const OscBuffer &l, const OscBuffer &r) {
  return VapplyBinary(l, r,
                      [](const Vec8d &l, const Vec8d &r) { return l * r; });
}

OscBuffer Vmul(const OscBuffer &l, double r) {
  return VapplyBinary(l, r, [](const Vec8d &l, double r) { return l * r; });
}

OscBuffer Vsub(const OscBuffer &l, const OscBuffer &r) {
  return VapplyBinary(l, r,
                      [](const Vec8d &l, const Vec8d &r) { return l - r; });
}

OscBuffer Vsub(const OscBuffer &l, double r) {
  return VapplyBinary(l, r, [](const Vec8d &l, double r) { return l - r; });
}

OscBuffer Vdiv(const OscBuffer &l, const OscBuffer &r) {
  return VapplyBinary(l, r,
                      [](const Vec8d &l, const Vec8d &r) { return l / r; });
}

OscBuffer Vdiv(const OscBuffer &l, double r) {
  return VapplyBinary(l, r, [](const Vec8d &l, double r) { return l / r; });
}

OscBuffer Vadd(const OscBuffer &l, const OscBuffer &r) {
  return VapplyBinary(l, r,
                      [](const Vec8d &l, const Vec8d &r) { return l + r; });
}

OscBuffer Vadd(const OscBuffer &l, double r) {
  return VapplyBinary(l, r, [](const Vec8d &l, double r) { return l + r; });
}

void VaddInplace(OscBuffer &l, const OscBuffer &r) {
  VapplyBinaryInplace(l, r,
                      [](const Vec8d &l, const Vec8d &r) { return l + r; });
}

void VmulInplace(OscBuffer &l, const OscBuffer &r) {
  VapplyBinaryInplace(l, r,
                      [](const Vec8d &l, const Vec8d &r) { return l * r; });
}

void VdivInplace(OscBuffer &l, const OscBuffer &r) {
  VapplyBinaryInplace(l, r,
                      [](const Vec8d &l, const Vec8d &r) { return l / r; });
}

void VsubInplace(OscBuffer &l, const OscBuffer &r) {
  VapplyBinaryInplace(l, r,
                      [](const Vec8d &l, const Vec8d &r) { return l - r; });
}

void VaddInplace(OscBuffer &l, double r) {
  VapplyBinaryInplace(l, r,
                      [](const Vec8d &l, const Vec8d &r) { return l + r; });
}

void VmulInplace(OscBuffer &l, double r) {
  VapplyBinaryInplace(l, r,
                      [](const Vec8d &l, const Vec8d &r) { return l * r; });
}

void VdivInplace(OscBuffer &l, double r) {
  VapplyBinaryInplace(l, r,
                      [](const Vec8d &l, const Vec8d &r) { return l / r; });
}

void VsubInplace(OscBuffer &l, double r) {
  VapplyBinaryInplace(l, r,
                      [](const Vec8d &l, const Vec8d &r) { return l - r; });
}

void ToFloat(const OscBuffer &src, float *out_buffer) {
  size_t size(src.size());
  OscBuffer dest(size);
  Vec8d src_vec;
  for (int i = 0; i < size; i += 8) {
    src_vec.load(&(src[i]));
    Vec8f dst_vec = to_float(src_vec);
    dst_vec.store(out_buffer + i);
  }
}

void linspace(OscBuffer &linspaced, double start, double end, size_t num) {
  double delta = (end - start) / (num - 1);
  int x = 0;
  std::generate(std::begin(linspaced), std::end(linspaced),
                [&x, start, delta]() { return (start + delta * x++); });
}

OscBuffer weighted_exp(size_t N, double start, double end, double weight) {
  OscBuffer t(N);
  linspace(t, start, end, N);
  OscBuffer E = exp(t * weight) - 1;
  E /= E.max();
  return E;
}

double CalcSlope(double next, double prev, double slope_factor) {
  return (next - prev) * slope_factor;
}

void LeakDC::Filter(OscBuffer &buf) {
  double b1 = b1_;
  double y1 = y1_;
  double x1 = x1_;

  double slope_factor = 1 / buf.size();
  double b1_slope = CalcSlope(b1_, b1, slope_factor);
  for (int i = 0; i < buf.size(); i++) {
    double x0 = buf[i];
    buf[i] = y1 = x0 - x1 + b1 * y1;
    x1 = x0;
    b1 += b1_slope;
  }
  x1_ = x1;
  y1_ = y1;
}

void Integrator::Filter(OscBuffer &buf, double newB1) {
  double b1 = b1_;
  double y1 = y1_;
  if (b1 == newB1) {
    if (b1 == 1.f) {
      for (int i = 0; i < buf.size(); i++) {
        double y0 = buf[i];
        buf[i] = y1 = y0 + y1;
      }
    } else if (b1 == 0.f) {
      for (int i = 0; i < buf.size(); i++) {
        double y0 = buf[i];
        buf[i] = y1 = y0 + b1 * y1;
      }
    } else {
      for (int i = 0; i < buf.size(); i++) {
        double y0 = buf[i];
        buf[i] = y1 = y0 + b1 * y1;
      }
    }
  } else {
    double slope_factor = 1 / buf.size();
    double b1_slope = CalcSlope(b1_, b1, slope_factor);
    for (int i = 0; i < buf.size(); i++) {
      double y0 = buf[i];
      buf[i] = y1 = y0 + b1 * y1;
      b1 += b1_slope;
    }
    b1_ = newB1;
  }
  y1_ = y1;
}
} // namespace sidebands