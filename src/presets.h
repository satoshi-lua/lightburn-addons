#pragma once

#include "compensation.h"
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace dtw {
struct Point { double x, y; };
enum class Rotation { None=0, Clockwise=90, HalfTurn=180, Counterclockwise=-90 };
struct Placement { Point center; Rotation rotation=Rotation::None; bool geometryReference=false; };

inline bool validPosition(Point p) {
 return std::isfinite(p.x) && std::isfinite(p.y) && std::abs(p.x)<=1e7 && std::abs(p.y)<=1e7;
}

inline bool validGeometry(const Parameters& p) {
 return std::isfinite(p.top) && std::isfinite(p.bottom) && std::isfinite(p.height)
     && std::isfinite(p.upper) && std::isfinite(p.lower) && std::isfinite(p.curve)
     && p.top>0 && p.bottom>0 && p.top/p.bottom>=.1 && p.top/p.bottom<=10
     && p.upper>=0 && p.lower>=0 && p.height-p.upper-p.lower>0 && p.curve>=0 && p.curve<=1;
}

struct GeometryPreset { std::string name; Parameters geometry; };
struct PositionPreset { std::string name; Placement placement; };

inline std::ostream& operator<<(std::ostream& out,const GeometryPreset& entry) {
 const auto& p=entry.geometry;
 return out<<std::quoted(entry.name)<<' '<<p.top<<' '<<p.upper<<' '<<p.bottom<<' '<<p.lower<<' '<<p.height<<' '<<p.curve;
}
inline std::istream& operator>>(std::istream& in,GeometryPreset& entry) {
 auto& p=entry.geometry;
 return in>>std::quoted(entry.name)>>p.top>>p.upper>>p.bottom>>p.lower>>p.height>>p.curve;
}
inline std::ostream& operator<<(std::ostream& out,const PositionPreset& entry) {
 const auto& p=entry.placement;
 return out<<std::quoted(entry.name)<<' '<<p.center.x<<' '<<p.center.y<<' '<<static_cast<int>(p.rotation)<<' '<<p.geometryReference;
}
inline std::istream& operator>>(std::istream& in,PositionPreset& entry) {
 auto& p=entry.placement;
 if(!(in>>std::quoted(entry.name)>>p.center.x>>p.center.y))return in;
 p.rotation=Rotation::None;
 if(in.eof()||(in>>std::ws).eof())return in; // Profiles saved before rotation was added.
 int degrees=0;
 if(in>>degrees) {
  if(degrees==0||degrees==90||degrees==180||degrees==-90)p.rotation=static_cast<Rotation>(degrees);
  else {in.setstate(std::ios::failbit);return in;}
  if(!in.eof()&&!(in>>std::ws).eof())in>>p.geometryReference;
 }
 return in;
}

template<class Preset>
std::string encodePresets(const std::vector<Preset>& entries) {
 std::ostringstream out;
 out.imbue(std::locale::classic());
 out<<std::setprecision(std::numeric_limits<double>::max_digits10);
 for(const auto& entry:entries)out<<entry<<'\n';
 return out.str();
}

template<class Preset,class Valid>
std::vector<Preset> decodePresets(const std::string& text,Valid valid) {
 std::vector<Preset> entries;
 std::istringstream input(text);
 for(std::string line;std::getline(input,line);) {
  std::istringstream record(line);
  record.imbue(std::locale::classic());
  Preset entry{};
  if(record>>entry && (record>>std::ws).eof() && !entry.name.empty() && valid(entry))entries.push_back(entry);
 }
 return entries;
}

inline double warpedX(double x,double center,double fromTop,const Parameters& p,bool single) {
 return center+(x-center)*compensation(fromTop+p.offset,p,single);
}

inline Point placePoint(Point point,Point warpedCenter,Placement target,bool yUp) {
 const double dx=point.x-warpedCenter.x;
 const double dy=point.y-warpedCenter.y;
 const double clockwise=yUp?-1.:1.;
 switch(target.rotation) {
  case Rotation::Clockwise: return {target.center.x-clockwise*dy,target.center.y+clockwise*dx};
  case Rotation::Counterclockwise: return {target.center.x+clockwise*dy,target.center.y-clockwise*dx};
  case Rotation::HalfTurn: return {target.center.x-dx,target.center.y-dy};
  case Rotation::None: return {target.center.x+dx,target.center.y+dy};
 }
 return point;
}

inline Point referenceAnchor(double centerX,double topY,const Parameters& p,bool yUp) {
 const double fromDesignTop=p.height/2-p.offset;
 return {centerX,topY+(yUp?-fromDesignTop:fromDesignTop)};
}

inline Point referenceSize(double height,Rotation turn) {
 return turn==Rotation::Clockwise||turn==Rotation::Counterclockwise?Point{height,30}:Point{30,height};
}

inline std::optional<Rotation> referenceRotation(Point localYAxis,Rotation initial,bool yUp) {
 if(!std::isfinite(localYAxis.x)||!std::isfinite(localYAxis.y)||
    std::hypot(localYAxis.x,localYAxis.y)<1e-6)return std::nullopt;
 constexpr double degreesPerRadian=57.29577951308232;
 const double delta=std::atan2(yUp?localYAxis.x:-localYAxis.x,localYAxis.y)*degreesPerRadian;
 const double quarters=std::round(delta/90.);
 // Allow for the rounding of a native rotation, without snapping diagonal guides.
 if(std::abs(delta-quarters*90.)>.05)return std::nullopt;
 const int degrees=(static_cast<int>(initial)+int(quarters)*90+360)%360;
 return static_cast<Rotation>(degrees==270?-90:degrees);
}
}
