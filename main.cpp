#include <iostream>
#include <fstream>
#include <vector>
#include "octree.h"


// using namespace cimg_library;
using namespace std;

Cube build_cube (string filename) {
    ifstream fileIn(filename);
	int dim_x, dim_y, dim_z;
	fileIn >> dim_x >> dim_y >> dim_z;
    Cube paciente (dim_z,vector<vector<int>>(dim_y,vector <int>(dim_z,0)));
	string fileLine;
    getline(fileIn, fileLine);
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

void visualizar(Cube cubo, string filename) { 
	ofstream fileout (filename.c_str());
	fileout << cubo[0][0].size() << " ";
	fileout << cubo[0].size() << " ";
	fileout << cubo.size() << endl;
	for (size_t k = 0; k < cubo.size(); k++) {
		for (size_t j = 0; j < cubo[0].size(); j++) {
			for (size_t i = 0; i < cubo[0][0].size(); i++) {
				fileout << cubo[k][j][i] << " ";
			}
			fileout << endl;
		}
		fileout << endl;
	}
}


Cube read_cube (string filename) {
	ifstream fileIn (filename.c_str());
    int dim_x, dim_y, dim_z;
    fileIn >> dim_x >> dim_y >> dim_z;
    Cube cubo (dim_z,vector<vector<int>>(dim_y,vector <int>(dim_x,0)));
    for (size_t k = 0; k < dim_z; k++) {
        for (size_t j = 0; j < dim_y; j++) {
            for (size_t i = 0; i < dim_x; i++) {
                fileIn >> cubo[k][j][i];
            }
        }
    }
	return cubo;
}

int main() {

	auto cubo = build_cube("paciente1_1.txt");
  	OcTree oct(cubo);
	OcTree oct2 ("octree.bin");
}
