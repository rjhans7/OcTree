#include<iostream> 
#include<tuple>
#include <vector>
#include "CImg/CImg.h"

#define N_IMAGES 8

using namespace cimg_library;
using namespace std;

typedef tuple <int, int, int> Point;
typedef vector<vector<vector<int>>> Cube;
class OcTree {
private:

    enum n_type {full, empty, middle};
    struct Node {
        long id;
        Point p_start, p_end;
        n_type type; // color
        long children[8];

        Node(){};

        Node(Point p_start, Point p_end){
            this->p_start = p_start;
            this->p_end = p_end;
            type = middle;
            id = -1;
            for (int i = 0; i < 8; i++) {
                children[i] = -1;
            }
        }

        void write(fstream &fileout, long n) {
            id = n;
            fileout.seekp(n*sizeof(Node), ios::beg);
            fileout.write((char *) this, sizeof(Node));
        }

        void read(fstream &fileIn, long n) {
            fileIn.seekg(n*sizeof(Node), ios::beg);
            fileIn.read((char *) this, sizeof(Node));
        }
    };

    long nNodes = 0;

public:
    OcTree (string filename) {
        fstream file(filename.c_str(), ios::binary | ios::in); 
        Node root;
        root.read(file, 0);
        CImg<char> images[N_IMAGES];
        for (int i = 0; i < N_IMAGES; i++) {
            images[i] = CImg (8, 8);
        }
        rebuild (root, images, file);
        for (int i = 0; i < N_IMAGES; i++) {
            images[i].display();
        }
    }

    void rebuild(Node root, CImg<char> *images, fstream &file){
        if (root.type == full || root.type == empty) {
            cout << "nodo pintado" << endl;
            for (int z = get<2>(root.p_start); z <= get<2>(root.p_end); z++)
                rebuild_img (root, z, images);
        }
        else {
            for (int i = 0; i < 8; i++) {
                cout << root.children[i] << endl;
                if (root.children[i] != -1) {
                    Node child;
                    child.read (file, root.children[i]);
                    rebuild (child, images, file);
                }
            }
        }
    }

    void rebuild_img (Node node, int z, CImg<char> *images) {
        for (int y = get<1>(node.p_start); y <= get<1>(node.p_end); y++) {
            for (int x = get<0>(node.p_start); x <= get<0>(node.p_end); x++) {
                images[z] (y, x) = node.type;
            }
        }
    }

    OcTree(Cube &img) {
        fstream file("octree.bin", ios::trunc | ios::binary | ios::in | ios::out);
        int size_x = img[0][0].size() - 1;
        int size_y = img[0].size() - 1;
        int size_z = img.size() - 1;
        Node root ({0, 0, 0}, {size_x, size_y, size_z});
        root.write(file, nNodes);
        nNodes++;

        build (0, 0, 0, size_x, size_y, size_z, root, img, file);
        file.close();
    }

    void build(int x_min, int y_min, int z_min, int x_max, int y_max, int z_max, Node &root, Cube &img, fstream &file) {
        if (check(x_min, y_min, z_min, x_max, y_max, z_max, img)) {
            root.type = img[z_min][y_min][x_min] == 0 ? full : empty;
            root.write(file, root.id);
            return;
        }

        int x_m = (x_max + x_min) / 2;
        int y_m = (y_max + y_min) / 2;
        int z_m = (z_max + z_min) / 2;

        Node child_0 ({x_min, y_min, z_min}, {x_m, y_m, z_m});
        child_0.write(file, nNodes);
        root.children[0] = nNodes;
        
        nNodes++;
        Node child_1 ({x_min, y_min, z_m + 1}, {x_m, y_m, z_max});
        child_1.write(file, nNodes);
        root.children[1] = nNodes;
        
        nNodes++;
        Node child_2 ({x_m + 1, y_min, z_m+1}, {x_max, y_m, z_max});
        child_2.write(file, nNodes);
        root.children[2] = nNodes;
        
        nNodes++;
        Node child_3 ({x_m + 1, y_min, z_min}, {x_max, y_m, z_m});
        child_3.write(file, nNodes);
        root.children[3] = nNodes; 
        
        nNodes++;
        Node child_4({x_min, y_m, z_min}, {x_m, y_max, z_m});
        child_4.write(file, nNodes);
        root.children[4] = nNodes;
        
        nNodes++;
        Node child_5({x_min, y_min, z_m + 1}, {x_m, y_max, z_max});
        child_5.write(file, nNodes);
        root.children[5] = nNodes;
        
        nNodes++;
        Node child_6({x_m + 1, y_m + 1, z_m + 1}, {x_max, y_max, z_max});
        child_6.write(file, nNodes);
        root.children[6] = nNodes;
        
        nNodes++;
        Node child_7 ({x_m + 1, y_m + 1, z_min}, {x_max, y_max, z_m});
        child_7.write(file, nNodes);
        root.children[7] = nNodes; 
        
        nNodes++;
        root.write(file, root.id);

        build (x_min, y_min, z_min, x_m, y_m, z_m, child_0, img, file);
        build (x_min, y_min, z_m + 1, x_m, y_m, z_max,child_1, img, file);
        build (x_m+1, y_min, z_m+1, x_max, y_m, z_max, child_2, img, file);
        build (x_m + 1, y_min, z_min, x_max, y_m, z_m, child_3, img, file);
        build (x_min, y_m, z_min, x_m, y_max, z_m, child_4, img, file);
        build (x_min, y_min, z_m + 1, x_m, y_max, z_max, child_5, img, file);
        build (x_m + 1, y_m + 1, z_m + 1, x_max, y_max, z_max, child_6, img, file);
        build (x_m + 1, y_m + 1, z_min, x_max, y_max, z_m, child_7, img, file);
    }


    bool check (int x_min, int y_min, int z_min, int x_max, int y_max, int z_max, Cube &img) {
        bool c = img[z_min][y_min][x_min];

        for (int z = z_min; z <= z_max; ++z) {
            for (int y = y_min; y <= y_max; ++y) {
                for (int x = x_min; x <= x_max; ++x) {
                    if (img[z][y][x] != c) return false;
                }
            }
        }

        return true;
    }


    ~OcTree(){}
};
