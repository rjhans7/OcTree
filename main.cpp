#include <iostream>
#include <fstream>
#include <vector>
#include "octree.h"


// using namespace cimg_library;
using namespace std;

Cube build_cube (string filename) {
    ifstream fileIn(filename);
	u_short dim_x, dim_y, dim_z;
	fileIn >> dim_x >> dim_y >> dim_z;
    Cube paciente (dim_z,vector<vector<cube_type>>(dim_y,vector <cube_type>(dim_x,0)));
	string fileLine;
    getline(fileIn, fileLine);
	u_short z = 0;
	while(getline(fileIn, fileLine)) {
		CImg<u_char> img(fileLine.c_str());
		CImg<u_char> imgBin(img.width(),img.height());
		//Binarizar
		cimg_forXY(img, x, y) { 
			if ((img(x, y, 0) + img(x, y, 1) +  img(x, y, 2)) / 3) imgBin(x, y) = 1;
			else imgBin(x, y) = 0;
		}
		// imgBin.crop(2, 2, 508, 508);
		// img.display();
		// imgBin.display();

		for (int i = 0; i < dim_x; i++) {
			for(int j = 0; j < dim_y; j++) {
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
    u_short dim_x, dim_y, dim_z;
    fileIn >> dim_x >> dim_y >> dim_z;
    Cube cubo (dim_z,vector<vector<cube_type>>(dim_y,vector <cube_type>(dim_x,0)));
    for (u_short k = 0; k < dim_z; k++) {
        for (u_short j = 0; j < dim_y; j++) {
            for (u_short i = 0; i < dim_x; i++) {
				u_short val;
                fileIn >> val;
				cubo[k][j][i] = val;
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
