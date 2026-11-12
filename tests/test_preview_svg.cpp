#include "../src/preview_svg.h"
#include <cassert>
#include <iostream>
int main() {
 dtw::Parameters p{87,0,73,0,143,0,20};
 std::vector<dtw::PreviewPath> paths{{{0,-20,0},{1,20,0},{1,20,100},{1,-20,100},{1,-20,0}}};
 auto original=dtw::previewSvg(p,true,40,paths,700,540,true,true,false,1,"mm",{});
 auto warped=dtw::previewSvg(p,true,40,paths,700,540,false,true,false,1,"mm",{});
 assert(original!=warped);
 assert(warped.find("87 mm")!=std::string::npos&&warped.find("73 mm")!=std::string::npos);
 assert(warped.find("Tramo cónico")!=std::string::npos);
 auto inch=dtw::previewSvg(p,true,40,paths,700,540,false,true,true,1/25.4,"in",{});
 assert(inch.find("Tapered section")!=std::string::npos&&inch.find("in")!=std::string::npos);
 dtw::Parameters dual{80,60,70,20,108,.49,4};
 auto zoned=dtw::previewSvg(dual,false,40,paths,700,540,false,true,false,1,"mm",{});
 assert(zoned.find("Transición")!=std::string::npos&&zoned.find("28 mm")!=std::string::npos);
 std::cout<<"PASS: preview profiles, original comparison, dimensions and units\n";
}
