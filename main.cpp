#include <iostream>
#include "CImg/CImg.h"

using namespace cimg_library;int main() {// Load 3d object from a .off file.

CImgList<unsigned int>primitives;
CImgList<unsigned char>colors;
const CImg<float> points=CImg<>::load_off(primitives,colors,"tomo.h5"); // Display 3d object in interactive window.
CImg<unsigned char>(800,600,1,3,128).display_object3d("Objetc",points,primitives,colors);

}