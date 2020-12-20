#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <algorithm>
#include "CImg/CImg.h"
#include "structs.h"

using namespace cimg_library;
using namespace std;

int ns = 0;

mutex mtx; 
mutex read_mtx; 

class OcTree {
private:

    enum n_type {full, empty, middle};
    struct Octant {
        long id;
        Point p_start, p_end;
        n_type type; // color
        long children[8];

        Octant(){};

        Octant(Point p_start, Point p_end){
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
            fileout.seekp(n*sizeof(Octant), ios::beg);
            fileout.write((char *) this, sizeof(Octant));
        }

        void read(fstream &fileIn, long n) {
            fileIn.seekg(n*sizeof(Octant), ios::beg);
            fileIn.read((char *) this, sizeof(Octant));
        }
    };

    long nOctants = 0;
    string filename;

public:

    OcTree (string filename) {
        this->filename = filename;
        rebuildAll();
    }

    OcTree (Cube &img) {
        filename = "octree.bin";
        fstream file(filename, ios::trunc | ios::binary | ios::in | ios::out);
        int size_x = img[0][0].size() - 1;
        int size_y = img[0].size() - 1;
        int size_z = img.size() - 1;
        Octant root ({0, 0, 0}, {size_x, size_y, size_z});
        root.write(file, nOctants);
        nOctants++;

        first_build (0, 0, 0, size_x, size_y, size_z, root, img, file);
            
        file.close();
    }

    // Deploys 8 threads;
    void first_build (int x_min, int y_min, int z_min, int x_max, int y_max, int z_max, Octant &root, Cube &img, fstream &file) {
		if (check(x_min, y_min, z_min, x_max, y_max, z_max, img)) {
            root.type = img[z_min][y_min][x_min] == 0 ? full : empty;
            root.write(file, root.id);

            return;
        }

        int x_m = (x_max + x_min) / 2;
        int y_m = (y_max + y_min) / 2;
        int z_m = (z_max + z_min) / 2;
        int x_m_p = x_m + 1;
        int y_m_p = y_m + 1;
        int z_m_p = z_m + 1;

        
        if ((x_min <= x_m) && (y_min <= y_m) && (z_min <= z_m)) {
            Octant child_0({x_min, y_min, z_min}, {x_m, y_m, z_m});
            child_0.write(file, nOctants);
            root.children[0] = nOctants;
            nOctants++;

            thread th1 ([this, x_min, y_min, z_min, x_m, y_m, z_m, &child_0, &img, &file](){
                build (x_min, y_min, z_min, x_m, y_m, z_m, child_0, img, file);
            });
            th1.join();

        }

        if ((x_min <= x_m) && (y_min <= y_m) && ((z_m + 1) <= z_max)) {
            Octant child_1({x_min, y_min, z_m + 1}, {x_m, y_m, z_max});
            child_1.write(file, nOctants);
            root.children[1] = nOctants;
            nOctants++;

            thread th2 ([this, x_min, y_min, z_m_p, x_m, y_m, z_max, &child_1, &img, &file]() {
                build (x_min, y_min, z_m_p, x_m, y_m, z_max, child_1, img, file);
            });
            th2.join();
        }

        if (((x_m + 1) <= x_max) && (y_min <= y_m) && ((z_m + 1) <= z_max)) {
            Octant child_2({x_m + 1, y_min, z_m + 1}, {x_max, y_m, z_max});
            child_2.write(file, nOctants);
            root.children[2] = nOctants;
            nOctants++;
            
            thread th3 ([this, x_m_p, y_min, z_m_p, x_max, y_m, z_max, &child_2, &img, &file]() {
                build (x_m_p, y_min, z_m_p, x_max, y_m, z_max, child_2, img, file);
            });
            th3.join();
        }

        if (((x_m + 1) <= x_max) && (y_min <= y_m) && (z_min <= z_m)) {
            Octant child_3({x_m + 1, y_min, z_min}, {x_max, y_m, z_m});
            child_3.write(file, nOctants);
            root.children[3] = nOctants;
            nOctants++;

            thread th4 ([this, x_m_p, y_min, z_min, x_max, y_m, z_m, &child_3, &img, &file]() {
                build (x_m_p, y_min, z_min, x_max, y_m, z_m, child_3, img, file);
            });
            th4.join();
        }

        if ((x_min <= x_m) && ((y_m + 1) <= y_max) && (z_min <= z_m)) {
            Octant child_4({x_min, y_m + 1, z_min}, {x_m, y_max, z_m});
            child_4.write(file, nOctants);
            root.children[4] = nOctants;
            nOctants++;

            thread th5 ([this, x_min, y_m_p, z_min, x_m, y_max, z_m, &child_4, &img, &file]() {
                build (x_min, y_m_p, z_min, x_m, y_max, z_m, child_4, img, file);
            });
            th5.join();
        }

        if ((x_min <= x_m) && ((y_m + 1) <= y_max) && ((z_m + 1) <= z_max)) {
            Octant child_5({x_min, y_m + 1, z_m + 1}, {x_m, y_max, z_max});
            child_5.write(file, nOctants);
            root.children[5] = nOctants;
            nOctants++;

            int tmp6 = y_m + 1;
            int tmp6_1 = z_m + 1;
            thread th6 ([this, x_min, tmp6, tmp6_1, x_m, y_max, z_max, &child_5, &img, &file]() {
                build (x_min, tmp6, tmp6_1, x_m, y_max, z_max, child_5, img, file);
            });
            th6.join();
        }

        if (((x_m + 1) <= x_max) && ((y_m + 1) <= y_max) && ((z_m + 1) <= z_max)) {
            Octant child_6({x_m + 1, y_m + 1, z_m + 1}, {x_max, y_max, z_max});
            child_6.write(file, nOctants);
            root.children[6] = nOctants;
            nOctants++;
            
            int tmp7 = x_m + 1;
            int tmp7_1 = y_m + 1;
            int tmp7_2 = z_m + 1;
            thread th7 ([this, tmp7, tmp7_1, tmp7_2, x_max, y_max, z_max, &child_6, &img, &file]() {
                build (tmp7, tmp7_1, tmp7_2, x_max, y_max, z_max, child_6, img, file);
            });
            th7.join();
        }

        if (((x_m + 1) <= x_max) && ((y_m + 1) <= y_max) && (z_min <= z_m)) {
            Octant child_7({x_m + 1, y_m + 1, z_min}, {x_max, y_max, z_m});
            child_7.write(file, nOctants);
            root.children[7] = nOctants;
            nOctants++;

            int tmp8 = x_m + 1;
            int tmp8_1 = y_m + 1;

            thread th8 ([this, tmp8, tmp8_1, z_min, x_max, y_max, z_m, &child_7, &img, &file]() {
                build (tmp8, tmp8_1, z_min, x_max, y_max, z_m, child_7, img, file);
            });
            th8.join();
        }

        root.write(file, root.id);
    }

    void build(int x_min, int y_min, int z_min, int x_max, int y_max, int z_max, Octant &root, Cube &img, fstream &file) {
		if (check(x_min, y_min, z_min, x_max, y_max, z_max, img)) {
            root.type = img[z_min][y_min][x_min] == 0 ? full : empty;
            root.write(file, root.id);

            return;
        }

        int x_m = (x_max + x_min) / 2;
        int y_m = (y_max + y_min) / 2;
        int z_m = (z_max + z_min) / 2;

        
        if ((x_min <= x_m) && (y_min <= y_m) && (z_min <= z_m)) {
            Octant child_0({x_min, y_min, z_min}, {x_m, y_m, z_m});
            child_0.write(file, nOctants);
            root.children[0] = nOctants;
            nOctants++;

            build (x_min, y_min, z_min, x_m, y_m, z_m, child_0, img, file);
        }

        if ((x_min <= x_m) && (y_min <= y_m) && ((z_m + 1) <= z_max)) {
            Octant child_1({x_min, y_min, z_m + 1}, {x_m, y_m, z_max});
            mtx.lock();
            child_1.write(file, nOctants);
            root.children[1] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_min, y_min, z_m + 1, x_m, y_m, z_max, child_1, img, file);
        }

        if (((x_m + 1) <= x_max) && (y_min <= y_m) && ((z_m + 1) <= z_max)) {
            Octant child_2({x_m + 1, y_min, z_m + 1}, {x_max, y_m, z_max});
            mtx.lock();
            child_2.write(file, nOctants);
            root.children[2] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_m + 1, y_min, z_m + 1, x_max, y_m, z_max, child_2, img, file);
        }

        if (((x_m + 1) <= x_max) && (y_min <= y_m) && (z_min <= z_m)) {
            Octant child_3({x_m + 1, y_min, z_min}, {x_max, y_m, z_m});
            mtx.lock();
            child_3.write(file, nOctants);
            root.children[3] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_m + 1, y_min, z_min, x_max, y_m, z_m, child_3, img, file);
        }

        if ((x_min <= x_m) && ((y_m + 1) <= y_max) && (z_min <= z_m)) {
            Octant child_4({x_min, y_m + 1, z_min}, {x_m, y_max, z_m});
            mtx.lock();
            child_4.write(file, nOctants);
            root.children[4] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_min, y_m + 1, z_min, x_m, y_max, z_m, child_4, img, file);
        }

        if ((x_min <= x_m) && ((y_m + 1) <= y_max) && ((z_m + 1) <= z_max)) {
            Octant child_5({x_min, y_m + 1, z_m + 1}, {x_m, y_max, z_max});
            mtx.lock();
            child_5.write(file, nOctants);
            root.children[5] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_min, y_m + 1, z_m + 1, x_m, y_max, z_max, child_5, img, file);
        }

        if (((x_m + 1) <= x_max) && ((y_m + 1) <= y_max) && ((z_m + 1) <= z_max)) {
            Octant child_6({x_m + 1, y_m + 1, z_m + 1}, {x_max, y_max, z_max});
            mtx.lock();
            child_6.write(file, nOctants);
            root.children[6] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_m + 1, y_m + 1, z_m + 1, x_max, y_max, z_max, child_6, img, file);
        }

        if (((x_m + 1) <= x_max) && ((y_m + 1) <= y_max) && (z_min <= z_m)) {
            Octant child_7({x_m + 1, y_m + 1, z_min}, {x_max, y_max, z_m});
            mtx.lock();
            child_7.write(file, nOctants);
            root.children[7] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_m + 1, y_m + 1, z_min, x_max, y_max, z_m, child_7, img, file);
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

    bool intersect (Plane plano, Octant root) {
        float diagonal = sqrt(((root.p_end.x - root.p_start.x) << 1) + ((root.p_end.y - root.p_start.y) << 1) + ((root.p_end.z - root.p_start.z) << 1));
        float distance = plano.distance(root.p_end);
        
        if (distance <= diagonal) return true;

        return false;
    }

    static bool comparey(Point a, Point b) {return (a.y < b.y);}
    static bool comparex(Point a, Point b) {return (a.x < b.x);}


    vector<Octant> make_cut(Point p1, Point p2, Point p3, Point p4) {

        vector<Point> points = {p1, p2, p3, p4};
        sort(points.begin(), points.end(), comparey);
        sort(points.begin(), points.begin() + 2 + 1, comparex);
        sort(points.begin() + 2, points.end(), comparex);

        vector<Octant> nodos;
        fstream file(filename.c_str(), ios::binary | ios::in);
        Octant root; root.read(file, 0);

        Plane plane(points[0], points[1], points[2], points[3]);

        Octant curr;
        for (int i = 0; i < 8; i++) {
            if (root.children[i] != -1) {
                read_mtx.lock();
                curr.read(file, root.children[i]);
                read_mtx.unlock();

                thread th ([this, plane, curr, &nodos, &file]() {
                    make_cut(plane, curr, nodos, file);
                });
                th.join();
            }
        }
        
        pintar(nodos, plane);
        return nodos;
    }

    void make_cut (Plane plane,  Octant root, vector<Octant> &nodos, fstream &file) {
        if (intersect(plane, root)) {
            if (root.type != middle) {
                
                nodos.push_back(root);
                return;
            }

            Octant curr;
            for (int i = 0; i < 8; i++) {
                if (root.children[i] != -1) {
                    read_mtx.lock();
                    curr.read(file, root.children[i]);
                    read_mtx.unlock();

                    make_cut(plane,curr, nodos, file);
                }
            }
        }
        return;
    }


    void rebuildByX (int x) {
        fstream file(filename.c_str(), ios::binary | ios::in);
        Octant root;
        root.read(file, 0);
        CImg<char> image (root.p_end.y + 1, root.p_end.z + 1);
        int x_m = (root.p_end.x + root.p_start.x)/2;

        vector<int> c_ids;
        if (x <= x_m) c_ids = {0, 1, 5, 4};
        else c_ids = {3, 2, 6, 7};        
        for (size_t i = 0; i < 4; i++) {
            Octant temp;
            temp.read(file, root.children[c_ids[i]]);   
            rebuildByX(x, temp, image, file);
        }
        
        image.display();

    }

    void rebuildByX (int x, Octant root, CImg<char> &image, fstream &file) {
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
                    Octant temp;
                    temp.read(file, root.children[c_ids[i]]);
                    rebuildByX(x, temp, image, file);
                }
            }
        }

    }

    void rebuildByY (int y) {
        fstream file(filename.c_str(), ios::binary | ios::in); 
        Octant root;
        root.read(file, 0);
        CImg<char> image (root.p_end.x + 1, root.p_end.z + 1);
        int y_m = (root.p_end.y + root.p_start.y)/2;

        size_t i = (y <= y_m)? 0 : 4;
        size_t i_e = (y <= y_m)? 4 : 8;
        for (; i < i_e; i++) {
            Octant temp;
            temp.read(file, root.children[i]);
            rebuildByY(y, temp, image, file);
        }
        image.display();
    }

    void rebuildByY (int y, Octant root, CImg<char> &image, fstream &file) {
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
                    Octant temp;
                    temp.read(file, root.children[i]);
                    rebuildByY(y, temp, image, file);
                }
            }
        }

    }

    void rebuildByZ (int z) {
        fstream file(filename.c_str(), ios::binary | ios::in); 
        Octant root;
        root.read(file, 0);
        CImg<char> image (root.p_end.x + 1, root.p_end.y + 1);
        int z_m = (root.p_end.z + root.p_start.z)/2;

        vector<int> c_ids;
        if (z <= z_m) c_ids = {0, 4, 7, 3};
        else c_ids = {1, 5, 6, 2};
        for (size_t i = 0; i < 4; i++) {
            Octant temp;
            temp.read(file, root.children[c_ids[i]]);
            rebuildByZ(z, temp, image, file);
        }
        image.display();
    }

    void rebuildByZ (int z, Octant root, CImg<char> &image, fstream &file) {
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
                    Octant temp;
                    temp.read(file, root.children[c_ids[i]]);
                    rebuildByZ(z, temp, image, file);
                }
            }
        }

    }


    void rebuildAll() {
        fstream file(filename.c_str(), ios::binary | ios::in); 
        Octant root;
        root.read(file, 0);
        int dim_z = root.p_end.z + 1;
        int dim_y = root.p_end.y + 1;
        int dim_x = root.p_end.x + 1;

        CImg<char> images[dim_z];

        for (int i = 0; i < dim_z; i++) images[i] = CImg<char> (dim_x, dim_y);

        if (root.type == full || root.type == empty) {
            for (int z = root.p_start.z; z <= root.p_end.z; z++) rebuild_img (root, z, images);
        }

        else {
            vector<thread> threads;

            for (int i = 0; i < 8; i++) {
                if (root.children[i] != -1) {
                    Octant child;
                    child.read (file, root.children[i]);

                    thread th([this, child, &images, &file]() {
                        rebuildAll (child, images, file);
                    });
                    th.join();
                }
            }

        }

        for (int i = 0; i < dim_z; i++) images[i].display();

    }

    void rebuildAll(Octant root, CImg<char> *images, fstream &file){
        if (root.type == full || root.type == empty) {
            for (int z = root.p_start.z; z <= root.p_end.z; z++)
                rebuild_img (root, z, images);
        }
        else {
            for (int i = 0; i < 8; i++) {
                if (root.children[i] != -1) {
                    Octant child;
                    child.read (file, root.children[i]);
                    rebuildAll (child, images, file);
                }
            }
        }
    }

    void rebuild_img (Octant octant, int z, CImg<char> *images) {
        for (int y = octant.p_start.y; y <= octant.p_end.y; y++) {
            for (int x = octant.p_start.x; x <= octant.p_end.x; x++) {
                images[z] (y, x) = octant.type;
            }
        }
    }


    void pintar (vector<Octant> octants, Plane corte) {
        /* Para cortes sobre eje y (y=0) */
        /* Obtener alto y ancho */
        int height = sqrt(pow(abs(corte.p1.z - corte.p4.z) + 1, 2) + pow(abs (corte.p4.x - corte.p1.x) + 1, 2));
        int width = abs(corte.p4.y - corte.p3.y) + 1;  //sqrt(pow(corte.p4.y - corte.p3.y, 2) + pow (corte.p4.x - corte.p3.x, 2));
        CImg<u_char> img (width, height);
        /* Pintar el color de cada nodo en el plano */ 

        size_t const quarter_size = octants.size() / 4;

        vector<Octant>::iterator begin = octants.begin();
        vector<Octant>::iterator split_1 = octants.begin() + quarter_size;
        vector<Octant>::iterator split_2 = octants.begin() + 2 * quarter_size;
        vector<Octant>::iterator split_3 = octants.end() - quarter_size;
        vector<Octant>::iterator end = octants.end();

        //draw (begin, end, img);
        thread th1 ([this, begin, split_1, &img]() {
            draw (begin, split_1, img);
        });
        thread th2 ([this, split_1, split_2, &img]() {
            draw (split_1, split_2, img);
        });

        thread th3 ([this, split_2, split_3, &img]() {
            draw (split_2, split_3, img);
        });

        thread th4 ([this, split_3, end, &img]() {
            draw (split_3, end, img);
        });

        th1.join();
        th2.join();
        th3.join();
        th4.join();

        img.display();
        //img.save_bmp("hola.bmp");
    }


    void draw (vector<Octant>::iterator start, vector<Octant>::iterator end, CImg<u_char> &img) {
        for (vector<Octant>::iterator octant = start; octant != end; octant++) {
            int w1 = octant->p_start.y;
            int w2 = octant->p_end.y;
            int h1 = octant->p_start.x;
            int h2 = octant->p_end.x;
            
            for (int i = h1; i <= h2; i++) {
                for (int j = w1; j <= w2; j++) {
                    if(i < img.width() && j < img.height())
                        img(i, j) = (octant->type == 0) ? 0 : 255;
                }
            }
            
        }
    }
    ~OcTree(){}

};