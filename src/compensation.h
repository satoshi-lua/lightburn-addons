#pragma once
#include "request_io.h"
#include <algorithm>

namespace dtw {
inline double compensation(double y,const Parameters& p,bool single) {
 if(single) {
  const double t=std::clamp(y/p.height,0.,1.);
  return 1+t*(p.top/p.bottom-1);
 }
 double t=std::clamp((y-p.upper)/(p.height-p.upper-p.lower),0.,1.);
 double sm=t*t*t*(t*(6*t-15)+10);
 double w=t+p.curve*(sm-t);
 return p.top/(p.top+(p.bottom-p.top)*w);
}
}
