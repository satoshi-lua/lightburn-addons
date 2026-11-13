#pragma once
#include "compensation.h"
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace dtw {
struct PreviewPoint { int kind; double x,y; };
using PreviewPath=std::vector<PreviewPoint>;
struct PreviewTheme {
 std::string background="#ffffff",text="#333333",accent="#1674c7";
};
inline std::string previewSvg(const Parameters& p,bool single,double designWidth,
                            const std::vector<PreviewPath>& paths,int width,int height,
                            bool original,bool zones,bool english,double unitScale,
                            const std::string& unit,const PreviewTheme& theme) {
 const double W=std::max(designWidth*std::max(1.,p.top/p.bottom)*1.16,45.);
 const double largest=std::max(p.top,p.bottom);
 const double k=std::min((width-185.)/(W+largest),(height-105.)/p.height);
 const double x=40,y=42+(height-105.-p.height*k)/2, rw=W*k,rh=p.height*k;
 const double pc=x+rw+25+largest*k/2,pr=pc+largest*k/2;
 std::ostringstream out;out.imbue(std::locale::classic());out<<std::setprecision(9);
 auto text=[&](double tx,double ty,const std::string& value,const std::string& anchor="start") {
  out<<"<text x='"<<tx<<"' y='"<<ty<<"' text-anchor='"<<anchor<<"' fill='"<<theme.text<<"'>"<<value<<"</text>";
 };
 auto number=[&](double value){std::ostringstream s;s.imbue(std::locale::classic());s<<std::setprecision(5)<<value*unitScale;return s.str()+" "+unit;};
 auto line=[&](double ax,double ay,double bx,double by,const std::string& extra="") {
  out<<"<path d='M"<<ax<<","<<ay<<" L"<<bx<<","<<by<<"' fill='none' stroke='"<<theme.text<<"' "<<extra<<"/>";
 };
 auto dimension=[&](double ax,double ay,double bx,double by) {
  line(ax,ay,bx,by);double len=std::hypot(bx-ax,by-ay);if(len<2)return;
  double ux=(bx-ax)/len,uy=(by-ay)/len,a=std::min(5.,len/4);
  for(int end=0;end<2;end++)for(int side:{-1,1}) {
   double px=end?bx:ax,py=end?by:ay,d=end?-1.:1.;
   line(px,py,px+d*ux*a-side*uy*a*.4,py+d*uy*a+side*ux*a*.4);
  }
 };
 out<<"<svg xmlns='http://www.w3.org/2000/svg' width='"<<width<<"' height='"<<height<<"' viewBox='0 0 "<<width<<" "<<height<<"'>";
 out<<"<rect width='"<<width<<"' height='"<<height<<"' fill='"<<theme.background<<"'/><g font-family='sans-serif' font-size='11'>";
 const double edges[]={0,single?0:p.upper,single?p.height:p.height-p.lower,p.height};
 if(zones) {
  const char* colors[]={"#4d9de0","#e3af36","#5da65d"};
  for(int i=0;i<3;i++)if(edges[i+1]>edges[i])
   out<<"<rect x='"<<x<<"' y='"<<y+edges[i]*k<<"' width='"<<rw<<"' height='"<<(edges[i+1]-edges[i])*k<<"' fill='"<<colors[i]<<"' fill-opacity='.12'/>";
 }
 out<<"<rect x='"<<x<<"' y='"<<y<<"' width='"<<rw<<"' height='"<<rh<<"' fill='none' stroke='"<<theme.text<<"'/>";
 line(x+rw/2,y-8,x+rw/2,y+rh+8,"stroke-dasharray='4 4' opacity='.4'");
 out<<"<g fill='none' stroke='"<<(original?theme.text:theme.accent)<<"' stroke-width='1.25' stroke-linejoin='round'>";
 for(const auto& path:paths) {
  out<<"<path d='";
  for(const auto& point:path) {
   double dx=point.x*(original?1:compensation(point.y+p.offset,p,single));
   out<<(point.kind==0?'M':'L')<<x+rw/2+dx*k<<","<<y+(point.y+p.offset)*k<<" ";
  }
  out<<"'/>";
 }
 out<<"</g>";
 // Vessel profile uses the same millimetre scale on both axes.
 out<<"<path d='";
 for(int side:{-1,1}) for(int j=0;j<=120;j++) {
  double yy=(side==-1?j:120-j)*p.height/120;
  double t=single?yy/p.height:std::clamp((yy-p.upper)/(p.height-p.upper-p.lower),0.,1.);
  if(!single)t+=p.curve*(t*t*t*(t*(6*t-15)+10)-t);
  double diameter=p.top+(p.bottom-p.top)*t;
  out<<((side==-1&&j==0)?'M':'L')<<pc+side*diameter*k/2<<","<<y+yy*k<<" ";
 }
 out<<"Z' fill='none' stroke='"<<theme.text<<"' stroke-width='1.4'/>";
 for(int i=1;i<3;i++)if(!single&&zones)line(x,y+edges[i]*k,pr+18,y+edges[i]*k,"stroke-dasharray='4 4' opacity='.4'");
 for(int bottom=0;bottom<2;bottom++) {
  double diameter=bottom?p.bottom:p.top,yy=bottom?y+rh:y,dy=bottom?14:-14;
  for(int side:{-1,1})line(pc+side*diameter*k/2,yy,pc+side*diameter*k/2,yy+dy+(bottom?5:-5));
  dimension(pc-diameter*k/2,yy+dy,pc+diameter*k/2,yy+dy);
  text(pc,yy+dy+(bottom?16:-6),number(diameter),"middle");
 }
 double labelBottom=y-5;
 for(int i=0;i<3;i++)if(edges[i+1]>edges[i]) {
  double ya=y+edges[i]*k,yb=y+edges[i+1]*k,dx=pr+12;
  line(pr+4,ya,dx+5,ya);line(pr+4,yb,dx+5,yb);dimension(dx,ya,dx,yb);
  double ly=std::max((ya+yb)/2-6,labelBottom+18);
  std::string label=i==0?(english?"Upper straight":"Recta superior"):i==2?(english?"Lower straight":"Recta inferior"):(single?(english?"Tapered section":"Tramo cónico"):(english?"Transition":"Transición"));
  text(dx+7,ly,label);text(dx+7,ly+15,number(edges[i+1]-edges[i]));labelBottom=ly+15;
 }
 out<<"<g transform='translate("<<x-15<<","<<y+rh/2<<") rotate(-90)'>";
 text(0,0,number(p.height),"middle");out<<"</g>";
 text(width/2.,height-12,original?(english?"Original":"Original"):(english?"Compensated design":"Diseño compensado"),"middle");
 out<<"</g></svg>";return out.str();
}
}
