#include <iostream>
#include <fstream>
#include <vector>
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
			if ((img(x, y, 0) + img(x, y, 1) +  img(x, y, 2)) / 3) imgBin(x, y) = 0;
			else imgBin(x, y) = 1;
		}

		for (int i = 0; i < imgBin.width(); i++) {
			for(int j = 0; j <imgBin.height(); j++) {
				paciente[z][j][i] = imgBin(j, i);
			}
		}
		z++;
	}

	return paciente;

}

void visualizar(Cube cubo, ofstream &fileout) { 
	for (size_t k = 0; k < cubo.size(); k++) {
		for (size_t j = 0; j < cubo[0].size(); j++) {
			for (size_t i = 0; i < cubo[0][0].size(); i++) {
				fileout << cubo[k][j][i] << " ";
			}
			fileout << endl;
		}
		fileout << endl << endl;
		
	}
	
}


int main() {

  	auto cubo = build_cube("test1.txt");
 	//visualizar(cubo);
  	OcTree oct(cubo);
	OcTree oct2 ("octree.bin");

  cout << endl;
  


}
