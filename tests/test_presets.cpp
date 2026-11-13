#include "../src/presets.h"
#include <cassert>
#include <iostream>

int main() {
 using namespace dtw;
 const Parameters geometry{80,60,70,20,108,.49,4};
 const std::vector<GeometryPreset> geometryPresets{{"Termo \"cónico\"",geometry}};
 auto storedGeometry=decodePresets<GeometryPreset>(encodePresets(geometryPresets)+"corrupt row\n",
  [](const auto& entry){return validGeometry(entry.geometry);});
 assert(storedGeometry.size()==1 && storedGeometry.front().name==geometryPresets.front().name);
 const auto& saved=storedGeometry.front().geometry;
 assert(saved.top==80 && saved.upper==60 && saved.bottom==70 && saved.lower==20);
 assert(saved.height==108 && saved.curve==.49 && saved.offset==0);

 const std::vector<PositionPreset> positions{
  {"Rotary setup",{{135.123456789,-100},Rotation::Clockwise}},
  {"Second fixture",{{0,0},Rotation::Counterclockwise,true}}
 };
 const auto storedPositions=decodePresets<PositionPreset>(
  encodePresets(positions)+"\"Legacy fixture\" 20 30\n\"Legacy turn\" 20 30 90\n\"out of range\" 10000001 0\n\"bad angle\" 20 30 45\n",
  [](const auto& entry){return validPosition(entry.placement.center);});
 assert(storedPositions.size()==4 && storedPositions.front().name=="Rotary setup");
 assert(storedPositions[0].placement.center.x==positions[0].placement.center.x && storedPositions[0].placement.center.y==-100);
 assert(storedPositions[0].placement.rotation==Rotation::Clockwise);
 assert(storedPositions[1].placement.rotation==Rotation::Counterclockwise);
 assert(storedPositions[2].placement.rotation==Rotation::None);
 assert(storedPositions[1].placement.geometryReference);
 assert(!storedPositions[0].placement.geometryReference && !storedPositions[2].placement.geometryReference);
 assert(storedPositions[3].placement.rotation==Rotation::Clockwise && !storedPositions[3].placement.geometryReference);
 const auto upright=referenceSize(108,Rotation::None),sideways=referenceSize(108,Rotation::Clockwise);
 assert(upright.x==30 && upright.y==108 && sideways.x==108 && sideways.y==30);
 const double left=warpedX(-10,0,0,geometry,false),right=warpedX(10,0,100,geometry,false);
 assert(right>-left);
 const Point warpedCenter{(left+right)/2,50},target{135,100};
 const auto a=placePoint({left,0},warpedCenter,{target,Rotation::None},false);
 const auto b=placePoint({right,100},warpedCenter,{target,Rotation::None},false);
 assert(std::abs((a.x+b.x)/2-135)<1e-12 && a.y==50 && b.y==150);
 for(bool yUp:{false,true}) {
  const double clockwiseX=yUp?1.:-1.;
  assert(referenceRotation({clockwiseX,0},Rotation::None,yUp)==Rotation::Clockwise);
  assert(referenceRotation({-clockwiseX,0},Rotation::None,yUp)==Rotation::Counterclockwise);
  assert(referenceRotation({clockwiseX,0},Rotation::Clockwise,yUp)==Rotation::HalfTurn);
  assert(referenceRotation({-clockwiseX,0},Rotation::Clockwise,yUp)==Rotation::None);
  assert(referenceRotation({0,-1},Rotation::None,yUp)==Rotation::HalfTurn);
  assert(!referenceRotation({1,1},Rotation::None,yUp));
  const Point top{warpedCenter.x,yUp?100.:0.};
  const auto cw=placePoint(top,warpedCenter,{target,Rotation::Clockwise},yUp);
  const auto ccw=placePoint(top,warpedCenter,{target,Rotation::Counterclockwise},yUp);
  assert(cw.x==185 && cw.y==100 && ccw.x==85 && ccw.y==100);
  const auto placedCenter=placePoint(warpedCenter,warpedCenter,{target,Rotation::Clockwise},yUp);
  assert(placedCenter.x==target.x && placedCenter.y==target.y);
  const auto anchor=referenceAnchor(warpedCenter.x,top.y,geometry,yUp);
  const auto placedTop=placePoint(top,anchor,{target,Rotation::Clockwise,true},yUp);
  assert(placedTop.x==target.x+geometry.height/2-geometry.offset && placedTop.y==target.y);
 }
 std::cout<<"Preset and engraving placement tests passed\n";
}
