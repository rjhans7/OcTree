#include <iostream>
#include <fstream>
#include <vector>
#include "CImg/CImg.h"
#include "octree.h"

#define N_X 8
#define N_Y 8
#define N_Z 8


using namespace cimg_library;
using namespace std;

Cube build_cube (string filename) {
	Cube paciente (N_Z,vector<vector<int>>(N_Y,vector <int>(N_X,0)));

	ifstream fileIn(filename);
	string fileLine;
	unsigned z = 0;
	while(getline(fileIn, fileLine)) {
		CImg<char> img(fileLine.c_str());
		CImg<char> imgBin(img.width(),img.height());
		//Binarizar
		cimg_forXY(img, x, y) { 
			imgBin(x, y) = (img(x, y, 0) + img(x, y, 1) +  img(x, y, 2)) / 3;
		}

		for (int i = 0; i < imgBin.width(); i++) {
			for(int j = 0; j <imgBin.height(); j++) {
				paciente[z][j][i] = imgBin(i, j);
			}
		}
		z++;
	}

	return paciente;

}


int main() {

  auto cubo = build_cube("test1.txt");
  OcTree oct(cubo);
  


}