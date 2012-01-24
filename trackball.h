#ifndef IGL_TRACKBALL_H
#define IGL_TRACKBALL_H

namespace igl
{
  // Applies a trackball drag to a given rotation
  // Inputs:
  //   w  width of the trackball context
  //   h  height of the trackball context
  //   speed_factor  controls how fast the trackball feels, 1 is normal
  //   down_quat  rotation at mouse down, i.e. the rotation we're applying the
  //     trackball motion to (as quaternion)
  //   down_mouse_x  x position of mouse down
  //   down_mouse_y  y position of mouse down
  //   mouse_x  current x position of mouse
  //   mouse_y  current y position of mouse
  // Outputs:
  //   quat  the resulting rotation (as quaternion)
  template <typename Q_type>
  inline void trackball(
    const int w,
    const int h,
    const Q_type speed_factor,
    const Q_type * down_quat,
    const int down_mouse_x,
    const int down_mouse_y,
    const int mouse_x,
    const int mouse_y,
    Q_type * quat);
}

// Implementation

#include "EPS.h"
#include "dot.h"
#include "cross.h"
#include "axis_angle_to_quat.h"
#include "quat_mult.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

// Utility inline functions
template <typename Q_type>
static inline Q_type _QuatD(int w, int h)
{
  return (Q_type)(abs(w) < abs(h) ? abs(w) : abs(h)) - 4;
}
template <typename Q_type>
static inline Q_type _QuatIX(int x, int w, int h)
{
  return (2.0f*(Q_type)x - (Q_type)w - 1.0f)/_QuatD<Q_type>(w, h);
}
template <typename Q_type>
static inline Q_type _QuatIY(int y, int w, int h)
{
  return (-2.0f*(Q_type)y + (Q_type)h - 1.0f)/_QuatD<Q_type>(w, h);
}

// This is largely the trackball as implemented in AntTweakbar. Much of the
// code is straight from its source in TwMgr.cpp
// http://www.antisphere.com/Wiki/tools:anttweakbar
template <typename Q_type>
inline void igl::trackball(
  const int w,
  const int h,
  const Q_type speed_factor,
  const Q_type * down_quat,
  const int down_mouse_x,
  const int down_mouse_y,
  const int mouse_x,
  const int mouse_y,
  Q_type * quat)
{
  assert(speed_factor > 0);

  double original_x = 
    _QuatIX<Q_type>(speed_factor*(down_mouse_x-w/2)+w/2, w, h);
  double original_y = 
    _QuatIY<Q_type>(speed_factor*(down_mouse_y-h/2)+h/2, w, h);

  double x = _QuatIX<Q_type>(speed_factor*(mouse_x-w/2)+w/2, w, h);
  double y = _QuatIY<Q_type>(speed_factor*(mouse_y-h/2)+h/2, w, h);

  double z = 1;
  double n0 = sqrt(original_x*original_x + original_y*original_y + z*z);
  double n1 = sqrt(x*x + y*y + z*z);
  if(n0>igl::DOUBLE_EPS && n1>igl::DOUBLE_EPS)
  {
    double v0[] = { original_x/n0, original_y/n0, z/n0 };
    double v1[] = { x/n1, y/n1, z/n1 };
    double axis[3];
    cross(v0,v1,axis);
    double sa = sqrt(dot(axis, axis));
    double ca = dot(v0, v1);
    double angle = atan2(sa, ca);
    if( x*x+y*y>1.0 )
    {
      angle *= 1.0 + 0.2f*(sqrt(x*x+y*y)-1.0);
    }
    double qrot[4], qres[4], qorig[4];
    axis_angle_to_quat(axis,angle,qrot);

    double nqorig =
      sqrt(down_quat[0]*down_quat[0]+
      down_quat[1]*down_quat[1]+
      down_quat[2]*down_quat[2]+
      down_quat[3]*down_quat[3]);

    if( fabs(nqorig)>igl::DOUBLE_EPS_SQ )
    {
        qorig[0] = down_quat[0]/nqorig;
        qorig[1] = down_quat[1]/nqorig;
        qorig[2] = down_quat[2]/nqorig;
        qorig[3] = down_quat[3]/nqorig;
        igl::quat_mult<double>(qrot,qorig,qres);
        quat[0] = qres[0];
        quat[1] = qres[1];
        quat[2] = qres[2];
        quat[3] = qres[3];
    }
    else
    {
        quat[0] = qrot[0];
        quat[1] = qrot[1];
        quat[2] = qrot[2];
        quat[3] = qrot[3];
    }
  }
}
#endif
