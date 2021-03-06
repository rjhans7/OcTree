#pragma once

#define THREADS

#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <algorithm>
#include <string>
#include "CImg/CImg.h"
#include "structs.h"

using namespace cimg_library;
using namespace std;

int ns = 0;

mutex mtx; 

class OcTree {
private:

    struct Octant {
        long id;
        Point p_start, p_end;
        cube_type color; // color
        bool is_leaf;
        long children[8];

        Octant(){};

        Octant(Point p_start, Point p_end){
            this->p_start = p_start;
            this->p_end = p_end;
            is_leaf = false;
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

    clock_t t_start, t_end;
    double time_taken;
    long ram;
    int cut_num;

public:

    OcTree (string filename) {
        this->filename = filename;
        cut_num = 0;
        rebuild_all();
    }

    OcTree (Cube &img, bool is_binary, cube_type img_umbral) {
        cut_num = 0;
        filename = "octree.bin";
        fstream file(filename, ios::trunc | ios::binary | ios::in | ios::out);
        int size_x = img[0][0].size() - 1;
        int size_y = img[0].size() - 1;
        int size_z = img.size() - 1;
        ram += sizeof(int) * 3 + sizeof(nOctants) + sizeof(filename);
        Octant root ({0, 0, 0}, {size_x, size_y, size_z});
        root.write(file, nOctants);
        ram += sizeof(root);
        nOctants++;

        #ifdef THREADS
            first_build (0, 0, 0, size_x, size_y, size_z, root, img, file, is_binary, img_umbral);
        #else 
            build (0, 0, 0, size_x, size_y, size_z, root, img, file, is_binary, img_umbral);
        #endif
            
        file.close();
    }

    void start_measures() {
        t_start = clock();
        this->ram = 0;
    }

    std::pair<double,long> end_measures() {
        t_end = clock();
        time_taken = double(t_end - t_start)/CLOCKS_PER_SEC; 
        return {time_taken, this->ram};
    }


    // Deploys 8 threads;
    void first_build (int x_min, int y_min, int z_min, int x_max, int y_max, int z_max, Octant &root, Cube &img, fstream &file, bool is_binary, cube_type img_umbral) {
		
        cube_type img_color = img[z_min][y_min][x_min];
        bool all_equal = (is_binary)? check(x_min, y_min, z_min, x_max, y_max, z_max, img): check_gray_scale(x_min, y_min, z_min, x_max, y_max, z_max, img, img_umbral, img_color);
        //bool all_equal = check(x_min, y_min, z_min, x_max, y_max, z_max, img);
        if (all_equal) {
            root.color = img_color;
            root.is_leaf = true;
            root.write(file, root.id);
            return;
        }

        int x_m = (x_max + x_min) / 2;
        int y_m = (y_max + y_min) / 2;
        int z_m = (z_max + z_min) / 2;
        int x_m_p = x_m + 1;
        int y_m_p = y_m + 1;
        int z_m_p = z_m + 1;

        ram += sizeof(int) * 6;

        
        if ((x_min <= x_m) && (y_min <= y_m) && (z_min <= z_m)) {
            Octant child_0({x_min, y_min, z_min}, {x_m, y_m, z_m});
            ram += sizeof(Octant);
            child_0.write(file, nOctants);
            root.children[0] = nOctants;
            nOctants++;

            thread th1 ([this, x_min, y_min, z_min, x_m, y_m, z_m, &child_0, &img, &file, &is_binary, &img_umbral](){
                build (x_min, y_min, z_min, x_m, y_m, z_m, child_0, img, file, is_binary, img_umbral);
            });
            th1.join();

        }

        if ((x_min <= x_m) && (y_min <= y_m) && ((z_m + 1) <= z_max)) {
            Octant child_1({x_min, y_min, z_m + 1}, {x_m, y_m, z_max});
            ram += sizeof(Octant);
            child_1.write(file, nOctants);
            root.children[1] = nOctants;
            nOctants++;

            thread th2 ([this, x_min, y_min, z_m_p, x_m, y_m, z_max, &child_1, &img, &file, &is_binary, &img_umbral]() {
                build (x_min, y_min, z_m_p, x_m, y_m, z_max, child_1, img, file, is_binary, img_umbral);
            });
            th2.join();
        }

        if (((x_m + 1) <= x_max) && (y_min <= y_m) && ((z_m + 1) <= z_max)) {
            Octant child_2({x_m + 1, y_min, z_m + 1}, {x_max, y_m, z_max});
            ram += sizeof(Octant);
            child_2.write(file, nOctants);
            root.children[2] = nOctants;
            nOctants++;
            
            thread th3 ([this, x_m_p, y_min, z_m_p, x_max, y_m, z_max, &child_2, &img, &file, &is_binary, &img_umbral]() {
                build (x_m_p, y_min, z_m_p, x_max, y_m, z_max, child_2, img, file, is_binary, img_umbral);
            });
            th3.join();
        }

        if (((x_m + 1) <= x_max) && (y_min <= y_m) && (z_min <= z_m)) {
            Octant child_3({x_m + 1, y_min, z_min}, {x_max, y_m, z_m});
            ram += sizeof(Octant);
            child_3.write(file, nOctants);
            root.children[3] = nOctants;
            nOctants++;

            thread th4 ([this, x_m_p, y_min, z_min, x_max, y_m, z_m, &child_3, &img, &file, &is_binary, &img_umbral]() {
                build (x_m_p, y_min, z_min, x_max, y_m, z_m, child_3, img, file, is_binary, img_umbral);
            });
            th4.join();
        }

        if ((x_min <= x_m) && ((y_m + 1) <= y_max) && (z_min <= z_m)) {
            Octant child_4({x_min, y_m + 1, z_min}, {x_m, y_max, z_m});
            ram += sizeof(Octant);
            child_4.write(file, nOctants);
            root.children[4] = nOctants;
            nOctants++;

            thread th5 ([this, x_min, y_m_p, z_min, x_m, y_max, z_m, &child_4, &img, &file, &is_binary, &img_umbral]() {
                build (x_min, y_m_p, z_min, x_m, y_max, z_m, child_4, img, file, is_binary, img_umbral);
            });
            th5.join();
        }

        if ((x_min <= x_m) && ((y_m + 1) <= y_max) && ((z_m + 1) <= z_max)) {
            Octant child_5({x_min, y_m + 1, z_m + 1}, {x_m, y_max, z_max});
            ram += sizeof(Octant);
            child_5.write(file, nOctants);
            root.children[5] = nOctants;
            nOctants++;

            thread th6 ([this, x_min, y_m_p, z_m_p, x_m, y_max, z_max, &child_5, &img, &file, &is_binary, &img_umbral]() {
                build (x_min, y_m_p, z_m_p, x_m, y_max, z_max, child_5, img, file, is_binary, img_umbral);
            });
            th6.join();
        }

        if (((x_m + 1) <= x_max) && ((y_m + 1) <= y_max) && ((z_m + 1) <= z_max)) {
            Octant child_6({x_m + 1, y_m + 1, z_m + 1}, {x_max, y_max, z_max});
            ram += sizeof(Octant);
            child_6.write(file, nOctants);
            root.children[6] = nOctants;
            nOctants++;
            
            thread th7 ([this, x_m_p, y_m_p, z_m_p, x_max, y_max, z_max, &child_6, &img, &file, &is_binary, &img_umbral]() {
                build (x_m_p, y_m_p, z_m_p, x_max, y_max, z_max, child_6, img, file, is_binary, img_umbral);
            });
            th7.join();
        }

        if (((x_m + 1) <= x_max) && ((y_m + 1) <= y_max) && (z_min <= z_m)) {
            Octant child_7({x_m + 1, y_m + 1, z_min}, {x_max, y_max, z_m});
            ram += sizeof(Octant);
            child_7.write(file, nOctants);
            root.children[7] = nOctants;
            nOctants++;

            thread th8 ([this, x_m_p, y_m_p, z_min, x_max, y_max, z_m, &child_7, &img, &file, &is_binary, &img_umbral]() {
                build (x_m_p, y_m_p, z_min, x_max, y_max, z_m, child_7, img, file, is_binary, img_umbral);
            });
            th8.join();
        }

        root.write(file, root.id);
    }

    void build (int x_min, int y_min, int z_min, int x_max, int y_max, int z_max, Octant &root, Cube &img, fstream &file, bool is_binary, cube_type img_umbral) {
		cube_type img_color = img[z_min][y_min][x_min];
        bool all_equal = (is_binary)? check(x_min, y_min, z_min, x_max, y_max, z_max, img): check_gray_scale(x_min, y_min, z_min, x_max, y_max, z_max, img, img_umbral, img_color);
        //bool all_equal = check(x_min, y_min, z_min, x_max, y_max, z_max, img);
        if (all_equal) {
            root.color = img_color;
            root.is_leaf = true;
            root.write(file, root.id);

            return;
        }
        int x_m = (x_max + x_min) / 2;
        int y_m = (y_max + y_min) / 2;
        int z_m = (z_max + z_min) / 2;

        ram += sizeof(int) * 3;

        
        if ((x_min <= x_m) && (y_min <= y_m) && (z_min <= z_m)) {
            Octant child_0({x_min, y_min, z_min}, {x_m, y_m, z_m});
            ram += sizeof(Octant);
            mtx.lock();
            child_0.write(file, nOctants);
            mtx.unlock();
            root.children[0] = nOctants;
            nOctants++;

            build (x_min, y_min, z_min, x_m, y_m, z_m, child_0, img, file, is_binary, img_umbral);
        }

        if ((x_min <= x_m) && (y_min <= y_m) && ((z_m + 1) <= z_max)) {
            Octant child_1({x_min, y_min, z_m + 1}, {x_m, y_m, z_max});
            ram += sizeof(Octant);
            mtx.lock();
            child_1.write(file, nOctants);
            root.children[1] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_min, y_min, z_m + 1, x_m, y_m, z_max, child_1, img, file, is_binary, img_umbral);
        }

        if (((x_m + 1) <= x_max) && (y_min <= y_m) && ((z_m + 1) <= z_max)) {
            Octant child_2({x_m + 1, y_min, z_m + 1}, {x_max, y_m, z_max});
            ram += sizeof(Octant);
            mtx.lock();
            child_2.write(file, nOctants);
            root.children[2] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_m + 1, y_min, z_m + 1, x_max, y_m, z_max, child_2, img, file, is_binary, img_umbral);
        }

        if (((x_m + 1) <= x_max) && (y_min <= y_m) && (z_min <= z_m)) {
            Octant child_3({x_m + 1, y_min, z_min}, {x_max, y_m, z_m});
            ram += sizeof(Octant);
            mtx.lock();
            child_3.write(file, nOctants);
            root.children[3] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_m + 1, y_min, z_min, x_max, y_m, z_m, child_3, img, file, is_binary, img_umbral);
        }

        if ((x_min <= x_m) && ((y_m + 1) <= y_max) && (z_min <= z_m)) {
            Octant child_4({x_min, y_m + 1, z_min}, {x_m, y_max, z_m});
            ram += sizeof(Octant);
            mtx.lock();
            child_4.write(file, nOctants);
            root.children[4] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_min, y_m + 1, z_min, x_m, y_max, z_m, child_4, img, file, is_binary, img_umbral);
        }

        if ((x_min <= x_m) && ((y_m + 1) <= y_max) && ((z_m + 1) <= z_max)) {
            Octant child_5({x_min, y_m + 1, z_m + 1}, {x_m, y_max, z_max});
            ram += sizeof(Octant);
            mtx.lock();
            child_5.write(file, nOctants);
            root.children[5] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_min, y_m + 1, z_m + 1, x_m, y_max, z_max, child_5, img, file, is_binary, img_umbral);
        }

        if (((x_m + 1) <= x_max) && ((y_m + 1) <= y_max) && ((z_m + 1) <= z_max)) {
            Octant child_6({x_m + 1, y_m + 1, z_m + 1}, {x_max, y_max, z_max});
            ram += sizeof(Octant);
            mtx.lock();
            child_6.write(file, nOctants);
            root.children[6] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_m + 1, y_m + 1, z_m + 1, x_max, y_max, z_max, child_6, img, file, is_binary, img_umbral);
        }

        if (((x_m + 1) <= x_max) && ((y_m + 1) <= y_max) && (z_min <= z_m)) {
            Octant child_7({x_m + 1, y_m + 1, z_min}, {x_max, y_max, z_m});
            ram += sizeof(Octant);
            mtx.lock();
            child_7.write(file, nOctants);
            root.children[7] = nOctants;
            nOctants++;
            mtx.unlock();
            build (x_m + 1, y_m + 1, z_min, x_max, y_max, z_m, child_7, img, file, is_binary, img_umbral);
        }
        root.write(file, root.id);
    }


    bool check (int &x_min, int &y_min, int &z_min, int &x_max, int &y_max, int &z_max, Cube &img) {
        cube_type c = img[z_min][y_min][x_min];
        ram += sizeof(bool);

        for (int z = z_min; z <= z_max; ++z) {
            for (int y = y_min; y <= y_max; ++y) {
                for (int x = x_min; x <= x_max; ++x) {
                    if (img[z][y][x] != c) return false;
                }
            }
        }
        return true;
    }

    bool check_gray_scale(int &x_min, int &y_min, int &z_min, int &x_max, int &y_max, int &z_max, Cube &img, cube_type &umbral, cube_type &color_octant) {
        if (umbral == 0) return check(x_min, y_min, z_min, x_max, y_max, z_max, img);
        int color_sum = 0;
        int n = abs((x_max-x_min) * (y_max-y_min) * (z_max - z_min));
        for(int i=x_min; i<=x_max; i++){
            for(int j=y_min; j<=y_max; j++){
                for (int k=z_min; k<=z_max; k++){
                    color_sum += img[k][j][i];
                }
            }
        }
        if (n == 0) return true;

        // int dist_tot = 0;
        // cube_type color_avg= color_sum/n;
        //  for(int i=x_min; i<=x_max; i++) {
        //     for(int j=y_min; j<=y_max; j++) {
        //         for (int k=z_min; k<=z_max; k++) {
        //             dist_tot += pow(img[k][j][i]-color_avg, 2);
        //         }
        //     }
        // }

        // cube_type sqrt_dist_avg = sqrt(dist_tot)/n;
        // color_octant = color_avg;
        // return sqrt_dist_avg <= umbral;
        cube_type color_avg= color_sum/n;
         for(int i=x_min; i<=x_max; i++) {
            for(int j=y_min; j<=y_max; j++) {
                for (int k=z_min; k<=z_max; k++) {
                    if (abs(img[k][j][i]-color_avg) < umbral)
                        return false;
                }
            }
        }

        color_octant = color_avg;
        return false;
    }

    bool intersect (Plane plano, Octant root) {
        float diagonal = sqrt(pow (root.p_end.x - root.p_start.x, 2) + pow (root.p_end.y - root.p_start.y, 2) + pow (root.p_end.z - root.p_start.z, 2));
        float distance = plano.distance(root.p_end);
        ram += sizeof(float) * 2;
        
        if (distance <= diagonal) return true;

        return false;
    }

    static bool comparey(Point a, Point b) {return (a.y < b.y);}
    static bool comparex(Point a, Point b) {return (a.x < b.x);}


    void make_cut (Point p1, Point p2, Point p3, Point p4) {
        vector<Point> points = {p1, p2, p3, p4};
        ram += points.size() * sizeof(Point);

        vector<Octant> octants;
        fstream file(filename.c_str(), ios::binary | ios::in);
        Octant root; root.read(file, 0);

        Plane plane(points[0], points[1], points[2], points[3]);
        if (plane.normal.x == 0 && plane.normal.y == 0) {
            rebuildByZ (plane.p1.z);
        }
        else if (plane.normal.x == 0 && plane.normal.z == 0) {
            rebuildByY (plane.p1.y);
        }
        else if (plane.normal.y == 0 && plane.normal.z == 0) {
            rebuildByX (plane.p1.x);
        }
        else {
            make_cut(plane, root, octants, file);
            pintar(octants, plane);
        }
        ram += octants.size() * sizeof(Octant) + sizeof(Octant) + sizeof(Plane);
        
    }


    void make_cut (Plane plane,  Octant root, vector<Octant> &nodos, fstream &file) {
        if (intersect(plane, root)) {
            if (root.is_leaf) {   
                nodos.push_back(root);
                return;
            }

            Octant curr;
            ram += sizeof(Octant);
            for (int i = 0; i < 8; i++) {
                if (root.children[i] != -1) {
                    curr.read(file, root.children[i]);

                    make_cut(plane, curr, nodos, file);
                }
            }
        }

        return;
    }


    void rebuildByX (int x) {
        fstream file(filename.c_str(), ios::binary | ios::in);
        Octant root;
        root.read(file, 0);
        CImg<u_char> image (root.p_end.y + 1, root.p_end.z + 1);
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
    

    void rebuildByX (int x, Octant root, CImg<u_char> &image, fstream &file) {
        if (root.is_leaf) {
            if (root.p_start.x <= x && root.p_end.x >= x) {
                for (int k = root.p_start.z; k <= root.p_end.z; k++) {
                    for (int j = root.p_start.y; j <= root.p_end.y; j++) {
                        image(j, k) = root.color;
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
        CImg<u_char> image (root.p_end.x + 1, root.p_end.z + 1);
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

    void rebuildByY (int y, Octant root, CImg<u_char> &image, fstream &file) {
        if (root.is_leaf) {
            if (root.p_start.y <= y && root.p_end.y >= y) {
                for (int k = root.p_start.z; k <= root.p_end.z; k++) {
                    for (int i = root.p_start.x; i <= root.p_end.x; i++) {
                        image(i, k) = root.color;
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
        CImg<u_char> image (root.p_end.x + 1, root.p_end.y + 1);
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

    void rebuildByZ (int z, Octant root, CImg<u_char> &image, fstream &file) {
        if (root.is_leaf) {
            if (root.p_start.z <= z && root.p_end.z >= z) {
                for (int j = root.p_start.y; j <= root.p_end.y; j++) {
                    for (int i = root.p_start.x; i <= root.p_end.x; i++) {
                        image(i, j) = root.color;
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


    void rebuild_all () {
        fstream file(filename.c_str(), ios::binary | ios::in); 
        Octant root;
        root.read(file, 0);
        int dim_z = root.p_end.z + 1;
        int dim_y = root.p_end.y + 1;
        int dim_x = root.p_end.x + 1;

        CImg<u_char> images[dim_z];

        for (int i = 0; i < dim_z; i++) images[i] = CImg<u_char> (dim_x, dim_y);

        rebuild_all (root, images, file);

        for (int i = 0; i < dim_z; i++) images[i].display();

    }

    void rebuild_all(Octant root, CImg<u_char> *images, fstream &file){
        if (root.is_leaf) {
            for (int z = root.p_start.z; z <= root.p_end.z; z++)
                rebuild_img (root, z, images);
        }
        else {
            for (int i = 0; i < 8; i++) {
                if (root.children[i] != -1) {
                    Octant child;
                    child.read (file, root.children[i]);
                    rebuild_all (child, images, file);
                }
            }
        }
    }

    void rebuild_img (Octant octant, int z, CImg<u_char> *images) {
        for (int y = octant.p_start.y; y <= octant.p_end.y; y++) {
            for (int x = octant.p_start.x; x <= octant.p_end.x; x++) {
                images[z] (y, x) = octant.color;
            }
        }
    }

    
    void pintar (vector<Octant> octants, Plane corte) {
        /* Para cortes sobre eje y (y=0) */
        /* Obtener alto y ancho */
        int height = sqrt(pow(corte.p3.x - corte.p1.x, 2) + pow (corte.p3.y - corte.p1.y, 2) + pow(corte.p3.z - corte.p1.z, 2)) + 3;
        int width = abs (corte.p4.x - corte.p3.x) + abs(corte.p4.y - corte.p3.y) + abs(corte.p4.z - corte.p3.z) + 1;  //sqrt(pow(corte.p4.y - corte.p3.y, 2) + pow (corte.p4.x - corte.p3.x, 2));
        cout << height << " " << width << endl;
        CImg<u_char> img (height, width);
        ram += sizeof(int) * 2 + sizeof(CImg<u_char>);
        
        vector<Octant>::iterator begin = octants.begin();
        vector<Octant>::iterator end = octants.end();

        /* Pintar el color de cada nodo en el plano */ 
        draw (begin, end, img, corte);

        img.display();

        string name = "resultados_cortes/corte_" + to_string(cut_num) + ".jpg";

        img.save_jpeg(name.c_str());
        cut_num++;
    }


    void draw (vector<Octant>::iterator start, vector<Octant>::iterator end, CImg<u_char> &img, Plane &plano) {
        for (vector<Octant>::iterator octant = start; octant != end; octant++) {
            //Hallamos el punto de interseccion del plano con las rectas 

	        for (int z = octant->p_start.z; z <= octant->p_end.z; ++z) {
				for (int y = octant->p_start.y; y <= octant->p_end.y; ++y) {
					for (int x = octant->p_start.x; x <= octant->p_end.x; ++x) {
						if (plano.distance({x, y, z}) < 2) {
                            int pitagoraso;
                            /* revisar orientacion del corte */
                            if (plano.p3.y != plano.p4.y) {
                                pitagoraso = sqrt (pow (plano.p3.x - x, 2) + pow (plano.p3.z - z, 2));
                                img(pitagoraso, y) = octant->color;
                            }
							else if (plano.p3.x != plano.p4.x) {
                                pitagoraso = sqrt (pow (plano.p3.y - y, 2) + pow (plano.p3.z - z, 2));
                                img(pitagoraso, x) = octant->color;
                            }
                            else if (plano.p3.z != plano.p4.z) {
                                pitagoraso = sqrt (pow (plano.p3.x - x, 2) + pow (plano.p3.y - y, 2));
                                img(pitagoraso, z) = octant->color;
                            }
						}
					}
				}
			}		
				
			
        }
    }
    ~OcTree(){}

};
