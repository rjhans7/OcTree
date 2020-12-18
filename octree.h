#include<iostream> 
#include<tuple>
#include <vector>
#include "CImg/CImg.h"



using namespace cimg_library;
using namespace std;
int ns = 0;

typedef unsigned short u_short;
typedef unsigned char u_char;
typedef bool cube_type;
typedef tuple <u_short, u_short, u_short> Point;
typedef vector<vector<vector<cube_type>>> Cube;


struct Punto {
    short x, y, z;

    Punto (u_short x, u_short y, u_short z) : x (x), y (y), z (z) {}

    /* Producto Escalar */
    u_short operator*(const Punto &p2) {
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
    Point normal;
    Point p1;
    Point p2;
    u_short d;
    Plane(Point normal, Point p1, Point p2): normal(normal), p1(p1), p2(p2){
        d = 0 - (get<0>(normal)*get<0>(p1) +
                 get<1>(normal)*get<1>(p1) +
                 get<2>(normal)*get<2>(p1));
    }
    bool checker(Point k) {
        u_short r = d + (get<0>(normal)*get<0>(k) +
            get<1>(normal)*get<1>(k) +
            get<2>(normal)*get<2>(k));
        return (r == 0);
    }
};

/*class OcTree {
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
    string filename;
public:
    OcTree (string filename) {
        this->filename = filename;        
    }

    bool intersect (Plane plano, Node root) {
        if (get<0>(plano.p1) >= get<0>(root.p_start) && get<0> (plano.p1) <=  get<0>(root.p_end) &&
            get<1>(plano.p1) >= get<1>(root.p_start) && get<1> (plano.p1) <=  get<1>(root.p_end) &&
            get<2>(plano.p1) >= get<2>(root.p_start) && get<2> (plano.p1) <=  get<2>(root.p_end) &&
            get<0>(plano.p2) >= get<0>(root.p_start) && get<0> (plano.p2) <=  get<0>(root.p_end) &&
            get<1>(plano.p2) >= get<1>(root.p_start) && get<1> (plano.p2) <=  get<1>(root.p_end) &&
            get<2>(plano.p2) >= get<2>(root.p_start) && get<2> (plano.p2) <=  get<2>(root.p_end))
            return true;
        return false;
    }



    vector<Node> make_cut(Point p1, Point p2, Point p3, Point p4){
        vector<Node> nodos;
        fstream file(filename.c_str(), ios::binary | ios::in);
        Node root; root.read(file, 0);

        Point v1 = {get<0>(p2) - get<0>(p1),
                    get<1>(p2) - get<1>(p1),
                    get<2>(p2) - get<2>(p1)};

        Point v2 = {get<0>(p3) - get<0>(p2),
                    get<1>(p3) - get<1>(p2),
                    get<2>(p3) - get<2>(p2)};

        Point normal = {get<1>(v1)*get<2> (v2) - get<2>(v1)*get<1>(v2),
                        -1 * (get<2>(v1)*get<0> (v2) - get<0>(v1)*get<2>(v2)),
                        get<0>(v1)*get<1> (v2) - get<1>(v1)*get<0>(v2)};

        Plane plane(normal, p1, p4);
        make_cut(plane, root, nodos);
    }*/

    /*void make_cut (Plane plane,  Node root, vector<Node> &nodos) {
        if (intersect(plane, root)) {
            if (root.type != middle) {
                nodos.push_back(root);
                return;
            }
            fstream file(filename.c_str(), ios::binary | ios::in);
            Node curr;
            for (int i = 0; i < 8; i++) {
                if (root.children[i] != -1) {
                    curr.read(file, root.children[i]);
                    make_cut(plane,curr, nodos)
                    
                }
            }
        }
        return;
    }*/


    /*void rebuildByX (int x) {
        fstream file(filename.c_str(), ios::binary | ios::in);
        Node root;
        root.read(file, 0);
        CImg<char> image (get<1>(root.p_end) + 1, get<2>(root.p_end) + 1);
        int x_m = (get<0>(root.p_end) + get<0>(root.p_start))/2;

        vector<u_short> c_ids;
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
            if (get<0>(root.p_start) <= x && get<0>(root.p_end) >= x) {
                for (u_short k = get<2>(root.p_start); k <= get<2>(root.p_end); k++) {
                    for (u_short j = get<1>(root.p_start); j <= get<1>(root.p_end); j++) {
                        image(j, k) = root.type;
                    }
                }
            }
            
        } else {
            int x_m = (get<0>(root.p_end) + get<0>(root.p_start))/2;

            vector<u_short> c_ids;
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
        CImg<char> image (get<0>(root.p_end) + 1, get<2>(root.p_end) + 1);
        int y_m = (get<1>(root.p_end) + get<1>(root.p_start))/2;

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
            if (get<1>(root.p_start) <= y && get<1>(root.p_end) >= y) {
                for (u_short k = get<2>(root.p_start); k <= get<2>(root.p_end); k++) {
                    for (u_short i = get<0>(root.p_start); i <= get<0>(root.p_end); i++) {
                        image(i, k) = root.type;
                    }
                }
            }
            
        } else {
            int y_m = (get<1>(root.p_end) + get<1>(root.p_start))/2;
            
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
        CImg<char> image (get<0>(root.p_end) + 1, get<1>(root.p_end) + 1);
        int z_m = (get<2>(root.p_end) + get<2>(root.p_start))/2;

        vector<u_short> c_ids;
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
            if (get<2>(root.p_start) <= z && get<2>(root.p_end) >= z) {
                for (u_short j = get<1>(root.p_start); j <= get<1>(root.p_end); j++) {
                    for (u_short i = get<0>(root.p_start); i <= get<0>(root.p_end); i++) {
                        image(i, j) = root.type;
                    }
                }
            }
            
        } else {
            int z_m = (get<2>(root.p_end) + get<2>(root.p_start))/2;

            vector<u_short> c_ids;
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
        int dim_z = get<2>(root.p_end) + 1;
        int dim_y = get<1>(root.p_end) + 1;
        int dim_x = get<0>(root.p_end) + 1;
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
            for (int z = get<2>(root.p_start); z <= get<2>(root.p_end); z++)
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
        for (int y = get<1>(node.p_start); y <= get<1>(node.p_end); y++) {
            for (int x = get<0>(node.p_start); x <= get<0>(node.p_end); x++) {
                images[z] (y, x) = node.type;
            }
        }
    }

    OcTree(Cube &img) {
        filename = "octree.bin";
        fstream file(filename, ios::trunc | ios::binary | ios::in | ios::out);
        u_short size_x = img[0][0].size() - 1;
        u_short size_y = img[0].size() - 1;
        u_short size_z = img.size() - 1;
        Node root ({0, 0, 0}, {size_x, size_y, size_z});
        root.write(file, nNodes);
        nNodes++;

        build (0, 0, 0, size_x, size_y, size_z, root, img, file);
        file.close();
    }

    void build(u_short x_min, u_short y_min, u_short z_min, u_short x_max, u_short y_max, u_short z_max, Node &root, Cube &img, fstream &file) {
        //cout << "building ... " << ns++ << endl;
		if (check(x_min, y_min, z_min, x_max, y_max, z_max, img)) {
            root.type = img[z_min][y_min][x_min] == 0 ? full : empty;
            root.write(file, root.id);
            return;
        }

        u_short x_m = (x_max + x_min) / 2;
        u_short y_m = (y_max + y_min) / 2;
        u_short z_m = (z_max + z_min) / 2;

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

        //build (x_min, y_min, z_min, x_m, y_m, z_m, child_0, img, file);
        //build (x_min, y_min, z_m + 1, x_m, y_m, z_max,child_1, img, file);
        //build (x_m + 1, y_min, z_m + 1, x_max, y_m, z_max, child_2, img, file);
        //build (x_m + 1, y_min, z_min, x_max, y_m, z_m, child_3, img, file);
        //build (x_min, y_m + 1, z_min, x_m, y_max, z_m, child_4, img, file);
        //build (x_min, y_m + 1, z_m + 1, x_m, y_max, z_max, child_5, img, file);
        //build (x_m + 1, y_m + 1, z_m + 1, x_max, y_max, z_max, child_6, img, file);
        //build (x_m + 1, y_m + 1, z_min, x_max, y_max, z_m, child_7, img, file);
    }


    bool check (u_short &x_min, u_short &y_min, u_short &z_min, u_short &x_max, u_short &y_max, u_short &z_max, Cube &img) {
        bool c = img[z_min][y_min][x_min];

        for (u_short z = z_min; z <= z_max; ++z) {
            for (u_short y = y_min; y <= y_max; ++y) {
                for (u_short x = x_min; x <= x_max; ++x) {
                    if (img[z][y][x] != c) return false;
                }
            }
        }
        return true;
    }


    ~OcTree(){}
};*/
