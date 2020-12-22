#include <fstream>
#include "CImg/CImg.h"
using namespace cimg_library;
using namespace std;
#ifndef cimg_imagepath
#define cimg_imagepath "img/"
#endif

// Main procedure


int main() {
  //Colors
  const unsigned char white[3]={ 255, 255, 255 };
  const unsigned char orange[] = { 255,128,64 };

  std::fprintf(stderr," - Create 3D Scene.\n");
  CImgList<unsigned int> plane_prims, scene_prims;

  // const CImg<float> plane_pts = CImg<>::plane3d(plane_prims,512,512);
  // plane_prims.insert(plane_prims.get_reverse_object3d());
  
  // // Define objects colors and textures.
  // const CImgList<unsigned char>
  // plane_cols = CImgList<unsigned char>(plane_prims.size(),CImg<unsigned char>::vector(60,120,180));

  // Append all object in a single 3D scene.
  CImg<float> scene_pts; //= CImg<float>().append_object3d(scene_prims,plane_pts,plane_prims);
  CImgList<unsigned char> scene_cols;// = (plane_cols);


  ifstream fileIn("paths/paciente1_1.txt");
	string fileLine;
  getline(fileIn, fileLine);
	u_short z = 0;
	while(getline(fileIn, fileLine)) {
		CImg<u_char> img2(fileLine.c_str());
    CImg<u_char> img = img2.crop(47, 28, 464,480);
    //img.display();
		CImg<u_char> imgBin(img.width(),img.height());
    
		//Binarizar
		cimg_forXY(img, x, y) { 
			int R = (int)img(x, y, 0);
			int G = (int)img(x, y, 1);
			int B = (int)img(x, y, 2);

			imgBin(x, y) = ((R+G+B)/3 > 100)? 255: 0;
		}

    CImgList<unsigned int> primitives;
    CImgList<unsigned char> colors;
    CImg<unsigned char> elevation(imgBin.width(),imgBin.height(),1,1,z*10);
    const CImg<> points = imgBin.get_elevation3d(primitives, colors, elevation);
		scene_pts.append_object3d(scene_prims, points, primitives);
    scene_cols.push_back(colors);
		z++;
	}

  

  // Display object3D in a user-interacted window and get final position matrix.
  std::fprintf(stderr," - Display 3D Scene.\n");
  CImg<float> view_matrix = CImg<>::identity_matrix(4);
  const char *const title = "Image viewed as a surface";
  CImgDisplay disp(800,600,title,0);
  CImg<unsigned char> visu(disp.width(),disp.height(),1,3,0);
  visu.draw_text(10,10,"%s",white,0,1,24,"Octree Visualization");
  visu.display_object3d(disp,scene_pts,scene_prims,scene_cols,true,0,-1,true,
                          500.0f,0,0,-5000,0.5f,0.1f,true,view_matrix.data());

  //  visu.display_object3d(disp,points,primitives,colors,first_time,rtype,-1,true,
  //                                500.0f,0.0f,0.0f,-5000.0f,0.0f,0.0f,true,pose.data());

  // visu.display_object3d(disp,points,primitives,colors,first_time,rtype,-1,true,
  //                                500.0f,0.0f,0.0f,-5000.0f,0.0f,0.0f,true,pose.data());

  // Exit.
  std::fprintf(stderr," - Exit.\n");
  return 0;
}