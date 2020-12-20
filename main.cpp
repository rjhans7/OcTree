#include <fstream>
#include <vector>

#include "octree.h"
#include "structs.h"


Cube build_cube (string filename, int dis, int umbral = 50) {
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
			int R = (int)img(x, y, 0);
			int G = (int)img(x, y, 1);
			int B = (int)img(x, y, 2);

			imgBin(x, y) = ((R+G+B)/3 > umbral)? 255: 0;
		}
		// imgBin.crop(2, 2, 508, 508);
		if (dis) {
			img.display();
			imgBin.display();
		}
	
		for(int j = 0; j < dim_y; j++) {
			for (int i = 0; i < dim_x; i++) {
				paciente[z][j][i] = imgBin(i, j);
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

int main(int argc,char **argv) {

	const char *file_i = cimg_option("-i", "paciente1_1.txt", "Input filename");
	const int dis = cimg_option("-s", 0,"Display");
	const int threshold = cimg_option("-t", 100,"Threshold");

	auto cubo = build_cube(file_i, dis, threshold);
 	OcTree oct(cubo);
	//visualizar(cubo, "cubo.txt");
	//OcTree oct2 ("octree.bin");
    // for (int i = 0; i < 8; i++) {
	// 	oct2.rebuildByX(i);
	// 	oct2.rebuildByY(i);
	// 	oct2.rebuildByZ(i);
	// }
	// oct2.rebuildByX(206);
	// oct2.rebuildByY(206);
	// oct2.rebuildByZ(20);
	//oct.make_cut({0, 0, 0},{511, 0, 0},{511, 511, 0}, {0, 511, 0});
	//oct.make_cut({0, 0, 39},{0, 511, 39},{511, 0, 0}, {511, 511, 0});
	//oct.make_cut({0, 0, 0}, {0, 511, 0}, {511, 0, 39}, {511, 511, 39});
	oct.make_cut({0, 0, 10},  {0, 500, 10}, {400, 0, 39}, {400, 500, 39});
	//oct.make_cut({0, 511, 0}, {511, 0, 39}, {511, 511, 39}, {0, 0, 0});

};
