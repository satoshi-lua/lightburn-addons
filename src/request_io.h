#pragma once
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace dtw {
struct Parameters { double top,upper,bottom,lower,height,curve,offset; };

inline Parameters readAndClaim(const std::filesystem::path& file) {
 Parameters p{};
 {
  std::ifstream input(file);
  input>>p.top>>p.upper>>p.bottom>>p.lower>>p.height>>p.curve>>p.offset;
  if(!input) throw std::runtime_error("Incomplete request.");
 } // Release the Windows file handle before renaming the request.
 std::filesystem::rename(file,file.parent_path()/L"applied.txt");
 return p;
}
}
