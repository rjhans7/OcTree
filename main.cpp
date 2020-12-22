#include <fstream>
#include <vector>

#include "octree.h"
#include "structs.h"
#include <stdlib.h>

Cube build_cube (string filename, bool display, cube_type bin_umbral = 50, bool to_binary = true) {
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
			
			imgBin(x, y) = (to_binary)? (((R+G+B)/3 > bin_umbral)? 255: 0): R;
		}

		// imgBin.crop(2, 2, 508, 508);
		if (display) {
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

static bool comparey(Point a, Point b) {return (a.y < b.y);}
static bool comparex(Point a, Point b) {return (a.x < b.x);}

int ram = 0;

void naive_cut (Cube cube, Point p1, Point p2, Point p3, Point p4) {
	vector<Point> points = {p1, p2, p3, p4};

	sort(points.begin(), points.end(), comparey);
	sort(points.begin(), points.begin() + 2 + 1, comparex);
	sort(points.begin() + 2, points.end(), comparex);

	Plane plane(points[0], points[1], points[2], points[3]);
	vector<Point> result;

	int y_max = 0;
	int y_min = 0;

	for (int z = 0; z < 40; z++) {
		for (int y = 0; y < 512; y++) {
			for (int x = 0; x < 512; x++) {
				Point p (x, y, z);
				if (plane.distance(p) < 5) {
					result.push_back(p);
					y_max = p.y > y_max ? p.y : y_max;
					y_min = p.y < y_min ? p.y : y_min;
				}
			}
		}
	}

	ram += cube.size () * cube[0].size () * cube[0][0].size () + result.size (); 
	CImg<u_char> img (sqrt(pow(points[0].z - points[2].z, 2) + pow(points[0].x - points[2].x, 2)) + 1, y_max - y_min + 1);

	for (auto point : result) {
		int pitagoraso = sqrt (pow (abs (points[2].x - point.x), 2) + pow (abs (points[2].z - point.z) , 2));
		img(pitagoraso, point.y) = cube[point.z][point.y][point.x] == 0 ? 0 : 255;
	}

	img.display();
}

int main(int argc,char **argv) {

	// Usar el flag -i para indicarle el archivo que contiene el paths con todas las imágenes 
	const char *file_i = cimg_option("-i", "paths/paciente1_1.txt", "Input filename");
	const bool dis = cimg_option("-d", 0,"Display");
	const cube_type threshold_bin = cimg_option("-b", 100,"Threshold Binarization");
	const cube_type threshold_gray_scale = cimg_option("-g", 10,"Threshold Gray Scale");
	const bool is_binary = cimg_option("-u", 1,"Use Binary Image");

	auto cubo = build_cube(file_i, dis, threshold_bin, is_binary);
 	OcTree oct(cubo, is_binary, threshold_gray_scale);
	vector<tuple<Point, Point, Point, Point>> cuts;
	
	tuple<Point, Point, Point, Point> cut;
	Point p1, p2, p3, p4;
	int y, z;
	/* Generar cortes en eje y */
	for (int i = 0; i < 7; i++) {
		z = rand () % 39;
		p1 = Point (0, 0, 39 - z);
		p2 = Point (0, 511, 39 - z);
		p3 = Point (511, 0, z);
		p4 = Point (511, 511, z);
		cut = {p1, p2, p3, p4};
		cuts.push_back (cut);
	}
	/* Generar cortes en eje x */
	for (int i = 0; i < 7; i++) {
		z = rand () % 39;
		p1 = Point (0, 0, 39 - z);
		p2 = Point (511, 0, 39 - z);
		p3 = Point (0, 511, z);
		p4 = Point (511, 511, z);
		cut = {p1, p2, p3, p4};
		cuts.push_back (cut);
	}
	/* Generar cortes en eje z */
	for (int i = 0; i < 6; i++) {
		y = rand () % 511;
		p1 = Point (0, y, 0);
		p2 = Point (0, y, 39);
		p3 = Point (511, 511 - y, 0);
		p4 = Point (511, 511 - y, 39);
		cut = {p1, p2, p3, p4};
		cuts.push_back (cut);
	}
	for (unsigned i = 0; i < cuts.size (); ++i) {
		cout << "corte: " << i << endl;
		cout << get<0>(cuts[i]).x << "," << get<0>(cuts[i]).y << "," << get<0>(cuts[i]).z << " "; 
		cout << get<1>(cuts[i]).x << "," << get<1>(cuts[i]).y << "," << get<1>(cuts[i]).z << " "; 
		cout << get<2>(cuts[i]).x << "," << get<2>(cuts[i]).y << "," << get<2>(cuts[i]).z << " "; 
		cout << get<3>(cuts[i]).x << "," << get<3>(cuts[i]).y << "," << get<3>(cuts[i]).z << endl; 
		oct.make_cut (get<0>(cuts[i]), get<1> (cuts[i]), get<2> (cuts[i]), get<3> (cuts[i]));
	}
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
	/* Cortes rectos */
	//oct.make_cut({0, 0, 19}, {0, 511, 19}, {511, 0, 19}, {511, 511, 19});
	//oct.make_cut({255, 0, 0}, {255, 0, 39}, {255, 511, 0}, {255, 511, 39});
	//oct.make_cut({0, 255, 0}, {0, 255, 39}, {511, 255, 0}, {511, 255, 39});
	/* -----------*/
	//oct.make_cut({0, 0, 0}, {511, 0, 0}, {0, 511, 39}, {511, 511, 39});
	//oct.make_cut({0, 20, 0}, {0, 20, 39}, {511, 491, 0}, {511, 491, 39});
	//clock_t start, end;
	//start = clock ();
	//naive_cut (cubo, {0, 255, 0}, {0, 255, 39}, {51, 255, 0}, {511, 255, 39});
	//end = clock ();
	//double time_taken = (double) (end - start) / CLOCKS_PER_SEC;
	//cout << "tiempo de ejecucion en oct: " << oct_result.first << " tiempo de ejecucion en bf: " << time_taken << endl;
	//cout << "ram utilizada en oct: " << oct_result.second << " ram utilizada en bf: " << ram << endl;
	//oct.make_cut({0, 0, 10},  {0, 500, 10}, {400, 0, 39}, {400, 500, 39});
	//oct.make_cut({0, 511, 0}, {511, 0, 39}, {511, 511, 39}, {0, 0, 0});
	//oct.make_cut({100, 250, 10},  {400, 250, 39}, {400, 0, 39}, {100, 0, 10});
	//cout << "tiempo de ejecucion en oct: " << oct_result.first << " tiempo de ejecucion en bf: " << time_taken << endl;
	//cout << "ram utilizada en oct: " << oct_result.second << " ram utilizada en bf: " << ram << endl;
	//oct.make_cut({0, 0, 10},  {0, 500, 10}, {400, 0, 39}, {400, 500, 39});
	//oct.make_cut({0, 511, 0}, {511, 0, 39}, {511, 511, 39}, {0, 0, 0});
	//oct.make_cut({100, 250, 10},  {400, 250, 39}, {400, 0, 39}, {100, 0, 10});
};