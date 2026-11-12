#include "../src/compensation.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main() {
 dtw::Parameters p{100,0,80,0,200,0,75};
 assert(dtw::compensation(0,p,true)==1);
 assert(dtw::compensation(100,p,true)==1.125);
 assert(dtw::compensation(200,p,true)==1.25);
 assert(dtw::compensation(p.offset,p,true)==1.09375);
 dtw::Parameters d{80,60,70,20,108,.49,0};
 assert(dtw::compensation(0,d,false)==1);
 assert(std::abs(dtw::compensation(74,d,false)-80./75)<1e-12);
 assert(std::abs(dtw::compensation(108,d,false)-80./70)<1e-12);
 std::cout<<"PASS: native single/dual compensation, reference offset\n";
}
