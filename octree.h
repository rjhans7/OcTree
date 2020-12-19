#include<iostream> 
#include<tuple>
#include <vector>
#include "CImg/CImg.h"
#include<fstream>


using namespace cimg_library;
using namespace std;
int ns = 0;

typedef unsigned char u_char;
typedef bool cube_type;
typedef tuple <int, int, int> Point;
typedef vector<vector<vector<cube_type>>> Cube;


struct Punto {
    int x, y, z;

    Punto(){}

    Punto (int x, int y, int z) : x (x), y (y), z (z) {}

    /* Producto Escalar */
    int operator*(const Punto &p2) {
        return x * p2.x + y * p2.y + z * p2.z;
    }

    /* Producto Vectorial */
    const Punto operator%(const Punto &p2) const {
        return Punto(y * p2.z - z * p2.y, x * p2.z - z * p2.x, x * p2.y - y * p2.x);
    }

    const Punto operator+ (const Punto &p2) const {
        return Punto (x + p2.x, y + p2.y, z + p2.z);
    }

    const Punto operator-(Punto &p2) const{
        return Punto (x - p2.x, y - p2.y, z - p2.z);
    }

    
};

ostream& operator<< (ostream &out, const Punto &p) {
        out << "x: " << p.x << " y: " << p.y << " z: " << p.z;
        return out;
    }
struct Plane {
    Punto normal;
    Punto p1, p2, p3, p4;
    int d;
    
    Plane(Punto p1, Punto p2, Punto p3, Punto p4): p1(p1), p2(p2), p3(p3), p4(p4) {
        Punto v1 = p2 - p1;
        Punto v2 = p3 - p1;
        normal = v1 % v2;
        d = 0 - normal * p1;
    }
    bool checker(Punto k) {
        int r = d + normal * k;
        return (r == 0);
    }

    int distance (Punto k) {
        return (d + normal * k) / sqrt((normal.x << 1) + (normal.y << 1) + (normal.z << 1));
    }
};

class OcTree {
private:

    enum n_type {full, empty, middle};
    struct Node {
        long id;
        Punto p_start, p_end;
        n_type type; // color
        long children[8];

        Node(){};

        Node(Punto p_start, Punto p_end){
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
    string filename;
public:
    OcTree (string filename) {
        this->filename = filename;
    }

    bool intersect (Plane plano, Node root) {
        float diagonal = sqrt(((root.p_end.x - root.p_start.x) << 1) + ((root.p_end.y - root.p_start.y) << 1) + ((root.p_end.z - root.p_start.z) << 1));
        float distance = plano.distance(root.p_end);
        
        if (distance <= diagonal) return true;

        return false;
    }


    vector<Node> make_cut(Punto p1, Punto p2, Punto p3, Punto p4){
        vector<Node> nodos;
        fstream file(filename.c_str(), ios::binary | ios::in);
        Node root; root.read(file, 0);

        Plane plane(p1, p2, p3, p4);

        // TODO: pthreads 
        make_cut(plane, root, nodos, file);
        return nodos;
    }

    void make_cut (Plane plane,  Node root, vector<Node> &nodos, fstream &file) {
        if (intersect(plane, root)) {
            if (root.type != middle) {
                nodos.push_back(root);
                return;
            }

            Node curr;
            for (int i = 0; i < 8; i++) {
                if (root.children[i] != -1) {
                    curr.read(file, root.children[i]);
                    make_cut(plane,curr, nodos, file);
                }
            }
        }
        return;
    }


    void rebuildByX (int x) {
        fstream file(filename.c_str(), ios::binary | ios::in);
        Node root;
        root.read(file, 0);
        CImg<char> image (root.p_end.y + 1, root.p_end.z + 1);
        int x_m = (root.p_end.x + root.p_start.x)/2;

        vector<int> c_ids;
        if (x <= x_m) c_ids = {0, 1, 5, 4};
        else c_ids = {3, 2, 6, 7};        
        for (size_t i = 0; i < 4; i++) {
            Node temp;
            temp.read(file, root.children[c_ids[i]]);
            rebuildByX(x, temp, image, file);
        }
        
        image.display();

    }

    void rebuildByX (int x, Node root, CImg<char> &image, fstream &file) {
        if (root.type != middle ) {
            if (root.p_start.x <= x && root.p_end.x >= x) {
                for (int k = root.p_start.z; k <= root.p_end.z; k++) {
                    for (int j = root.p_start.y; j <= root.p_end.y; j++) {
                        image(j, k) = root.type;
                    }
                }
            }
            
        } else {
            int x_m = (root.p_end.x + root.p_start.x)/2;

            vector<int> c_ids;
            if (x <= x_m) c_ids = {0, 1, 5, 4};
            else c_ids = {3, 2, 6, 7};
            for (size_t i = 0; i < 4; i++) {
                if (root.children[c_ids[i]] != -1) {
                    Node temp;
                    temp.read(file, root.children[c_ids[i]]);
                    rebuildByX(x, temp, image, file);
                }
            }
        }

    }

    void rebuildByY (int y) {
        fstream file(filename.c_str(), ios::binary | ios::in); 
        Node root;
        root.read(file, 0);
        CImg<char> image (root.p_end.x + 1, root.p_end.z + 1);
        int y_m = (root.p_end.y + root.p_start.y)/2;

        size_t i = (y <= y_m)? 0 : 4;
        size_t i_e = (y <= y_m)? 4 : 8;
        for (; i < i_e; i++) {
            Node temp;
            temp.read(file, root.children[i]);
            rebuildByY(y, temp, image, file);
        }
        image.display();
    }

    void rebuildByY (int y, Node root, CImg<char> &image, fstream &file) {
        if (root.type != middle ) {
            if (root.p_start.y <= y && root.p_end.y >= y) {
                for (int k = root.p_start.z; k <= root.p_end.z; k++) {
                    for (int i = root.p_start.x; i <= root.p_end.x; i++) {
                        image(i, k) = root.type;
                    }
                }
            }
            
        } else {
            int y_m = (root.p_end.y + root.p_start.y)/2;
            
            size_t i = (y <= y_m)? 0 : 4;
            size_t i_e = (y <= y_m)? 4 : 8;
            for (; i < i_e; i++) {
                if (root.children[i] != -1) {
                    Node temp;
                    temp.read(file, root.children[i]);
                    rebuildByY(y, temp, image, file);
                }
            }
        }

    }

    void rebuildByZ (int z) {
        fstream file(filename.c_str(), ios::binary | ios::in); 
        Node root;
        root.read(file, 0);
        CImg<char> image (root.p_end.x + 1, root.p_end.y + 1);
        int z_m = (root.p_end.z + root.p_start.z)/2;

        vector<int> c_ids;
        if (z <= z_m) c_ids = {0, 4, 7, 3};
        else c_ids = {1, 5, 6, 2};
        for (size_t i = 0; i < 4; i++) {
            Node temp;
            temp.read(file, root.children[c_ids[i]]);
            rebuildByZ(z, temp, image, file);
        }
        image.display();
    }

    void rebuildByZ (int z, Node root, CImg<char> &image, fstream &file) {
        if (root.type != middle ) {
            if (root.p_start.z <= z && root.p_end.z >= z) {
                for (int j = root.p_start.y; j <= root.p_end.y; j++) {
                    for (int i = root.p_start.x; i <= root.p_end.x; i++) {
                        image(i, j) = root.type;
                    }
                }
            }
            
        } else {
            int z_m = (root.p_end.z + root.p_start.z)/2;

            vector<int> c_ids;
            if (z <= z_m) c_ids = {0, 4, 7, 3};
            else c_ids = {1, 5, 6, 2};
            for (size_t i = 0; i < 4; i++) {
                if (root.children[c_ids[i]] != -1) {
                    Node temp;
                    temp.read(file, root.children[c_ids[i]]);
                    rebuildByZ(z, temp, image, file);
                }
            }
        }

    }


    void rebuildAll() {
        fstream file(filename.c_str(), ios::binary | ios::in); 
        Node root;
        root.read(file, 0);
        int dim_z = root.p_end.z + 1;
        int dim_y = root.p_end.y + 1;
        int dim_x = root.p_end.x + 1;
        CImg<char> images[dim_z];
        for (int i = 0; i < dim_z; i++) {
            images[i] = CImg<char> (dim_x, dim_y);
        }
        rebuildAll (root, images, file);
        for (int i = 0; i < dim_z; i++) {
            images[i].display();
        }

    }

    void rebuildAll(Node root, CImg<char> *images, fstream &file){
        if (root.type == full || root.type == empty) {
            //cout << "nodo pintado" << endl;
            for (int z = root.p_start.z; z <= root.p_end.z; z++)
                rebuild_img (root, z, images);
        }
        else {
            for (int i = 0; i < 8; i++) {
                //cout << root.children[i] << endl;
                if (root.children[i] != -1) {
                    Node child;
                    child.read (file, root.children[i]);
                    rebuildAll (child, images, file);
                }
            }
        }
    }

    void rebuild_img (Node node, int z, CImg<char> *images) {
        for (int y = node.p_start.y; y <= node.p_end.y; y++) {
            for (int x =node.p_start.x; x <= node.p_end.x; x++) {
                images[z] (y, x) = node.type;
            }
        }
    }

    OcTree(Cube &img) {
        filename = "octree.bin";
        fstream file(filename, ios::trunc | ios::binary | ios::in | ios::out);
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

        if ((x_min <= x_m) && (y_min <= y_m) && (z_min <= z_m)) {
            Node child_0({x_min, y_min, z_min}, {x_m, y_m, z_m});
            child_0.write(file, nNodes);
            root.children[0] = nNodes;
            nNodes++;
            build(x_min, y_min, z_min, x_m, y_m, z_m, child_0, img, file);

        }

        if ((x_min <= x_m) && (y_min <= y_m) && ((z_m + 1) <= z_max)) {
            Node child_1({x_min, y_min, z_m + 1}, {x_m, y_m, z_max});
            child_1.write(file, nNodes);
            root.children[1] = nNodes;
            nNodes++;
            build(x_min, y_min, z_m + 1, x_m, y_m, z_max, child_1, img, file);
        }

        if (((x_m + 1) <= x_max) && (y_min <= y_m) && ((z_m + 1) <= z_max)) {
            Node child_2({x_m + 1, y_min, z_m + 1}, {x_max, y_m, z_max});
            child_2.write(file, nNodes);
            root.children[2] = nNodes;
            nNodes++;
            build(x_m + 1, y_min, z_m + 1, x_max, y_m, z_max, child_2, img, file);
        }

        if (((x_m + 1) <= x_max) && (y_min <= y_m) && (z_min <= z_m)) {
            Node child_3({x_m + 1, y_min, z_min}, {x_max, y_m, z_m});
            child_3.write(file, nNodes);
            root.children[3] = nNodes;
            nNodes++;
            build(x_m + 1, y_min, z_min, x_max, y_m, z_m, child_3, img, file);
        }

        if ((x_min <= x_m) && ((y_m + 1) <= y_max) && (z_min <= z_m)) {
            Node child_4({x_min, y_m + 1, z_min}, {x_m, y_max, z_m});
            child_4.write(file, nNodes);
            root.children[4] = nNodes;
            nNodes++;
            build(x_min, y_m + 1, z_min, x_m, y_max, z_m, child_4, img, file);
        }

        if ((x_min <= x_m) && ((y_m + 1) <= y_max) && ((z_m + 1) <= z_max)) {
            Node child_5({x_min, y_m + 1, z_m + 1}, {x_m, y_max, z_max});
            child_5.write(file, nNodes);
            root.children[5] = nNodes;
            nNodes++;
            build(x_min, y_m + 1, z_m + 1, x_m, y_max, z_max, child_5, img, file);
        }

        if (((x_m + 1) <= x_max) && ((y_m + 1) <= y_max) && ((z_m + 1) <= z_max)) {
            Node child_6({x_m + 1, y_m + 1, z_m + 1}, {x_max, y_max, z_max});
            child_6.write(file, nNodes);
            root.children[6] = nNodes;
            nNodes++;
            build(x_m + 1, y_m + 1, z_m + 1, x_max, y_max, z_max, child_6, img, file);
        }

        if (((x_m + 1) <= x_max) && ((y_m + 1) <= y_max) && (z_min <= z_m)) {
            Node child_7({x_m + 1, y_m + 1, z_min}, {x_max, y_max, z_m});
            child_7.write(file, nNodes);
            root.children[7] = nNodes;
            nNodes++;
            build(x_m + 1, y_m + 1, z_min, x_max, y_max, z_m, child_7, img, file);
        }
        root.write(file, root.id);
    }


    bool check (int &x_min, int &y_min, int &z_min, int &x_max, int &y_max, int &z_max, Cube &img) {
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
