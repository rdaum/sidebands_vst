#include "dsp/dc_block.h"

namespace sidebands {

void DCBlock::Filter(OscBuffer &buf) {
  double b1 = b1_;
  double y1 = y1_;
  double x1 = x1_;

  for (int i = 0; i < buf.size(); i++) {
    double x0 = buf[i];
    buf[i] = y1 = x0 - x1 + b1 * y1;
    x1 = x0;
  }
  x1_ = x1;
  y1_ = zapgremlins(y1);
}

DCBlock2::DCBlock2(int order)
    : delay1_(order),
      iirdelay1_(order),
      iirdelay2_(order),
      iirdelay3_(order),
      iirdelay4_(order),
      ydels_{0, 0, 0, 0},
      dp1_(0),
      dp2_(0),
      scaler_(1.0 / order) {}

void DCBlock2::Filter(OscBuffer &in) {
  {
    size_t del1size = delay1_.size();
    size_t iirdelsize = iirdelay1_.size();
    size_t bufsize = in.size();

    OscBuffer *iirdel[]{&iirdelay1_, &iirdelay2_, &iirdelay3_, &iirdelay4_};
    double x1, x2, y, del;
    int p1 = dp1_;
    int p2 = dp2_;
    for (int i = 0; i < bufsize; i++) {
      /* long delay */
      del = delay1_[p1];
      delay1_[p1] = x1 = in[i];

      /* IIR cascade */
      for (int j = 0; j < 4; j++) {
        x2 = (*iirdel[j])[p2];
        (*iirdel[j])[p2] = x1;
        y = x1 - x2 + ydels_[j];
        ydels_[j] = y;
        x1 = y * scaler_;
      }
      in[i] = (del - x1);

      p1 = (p1 == del1size - 1 ? 0 : p1 + 1);
      p2 = (p2 == iirdelsize - 1 ? 0 : p2 + 1);
    }

    dp1_ = p1;
    dp2_ = p2;
  }
}

}  // namespace sidebands