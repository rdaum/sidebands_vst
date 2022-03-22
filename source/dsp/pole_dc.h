#pragma once

#include "dsp/oscbuffer.h"

namespace sidebands {


struct PoleZero {
  PoleZero();

  void clear();

  //! Set the b[0] coefficient value.
  void setB0( double b0 ) { b_[0] = b0; };

  //! Set the b[1] coefficient value.
  void setB1( double b1 ) { b_[1] = b1; };

  //! Set the a[1] coefficient value.
  void setA1( double a1 ) { a_[1] = a1; };

  //! Set all filter coefficients.
  void setCoefficients( double b0, double b1, double a1, bool clearState = false );

  // Set the filter for allpass behavior using \e coefficient.
  /*!
    This method uses \e coefficient to create an allpass filter,
    which has unity gain at all frequencies.  Note that the
    \e coefficient magnitude must be less than one to maintain
    filter stability.
  */
  void setAllpass( double coefficient );

  // Create a DC blocking filter with the given pole position in the z-plane.
  /*!
    This method sets the given pole position, together with a zero
    at z=1, to create a DC blocking filter.  The argument magnitude
    should be close to (but less than) one to minimize low-frequency
    attenuation.
  */
  void setBlockZero( double thePole = 0.99 );

  // Input one sample to the filter and return one output.
  double tick( double input );

  OscBuffer a_;
  OscBuffer b_;
  double gain_;
  OscBuffer inputs_;
  OscBuffer outputs_;
  OscBuffer lastFrame_;
};
}  // namespace sidebands
