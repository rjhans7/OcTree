#include<iostream> 
#include<tuple>
#include <vector>

using namespace std;

typedef tuple <int, int, int> Point;
typedef vector<vector<vector<int>>> Cube;

class OcTree {
private:

    enum n_type {full, empty, middle};
    struct Node {
        Point p_start, p_end;
        n_type type; // color
        Node* children[8];

        Node(Point p_start, Point p_end){
            this->p_start = p_start;
            this->p_end = p_end;
            type = middle;
            for (int i = 0; i < 8; i++) {
                children[i] = nullptr;
            }
        }
    };

    Node* root;
 
    Cube img;

public:
    OcTree(Cube img) {
        root = new Node({0, 0, 0}, {img[0][0].size() - 1, img[0].size() - 1, img.size() - 1});
        this->img = img;

        build (0, 0, 0, img[0][0].size() - 1, img[0].size() - 1, img.size() - 1, root);
    }

    void build(int x_min, int y_min, int z_min, int x_max, int y_max, int z_max, Node *root) {
        if (check(x_min, y_min, z_min, x_max, y_max, z_max)) {
            root->type = img[z_min][y_min][x_min] == 0 ? full : empty;
            return;
        }

        int x_m = (x_max + x_min) / 2;
        int y_m = (y_max + y_min) / 2;
        int z_m = (z_max + z_min) / 2;

        root->children[0] = new Node ({x_min, y_min, z_min}, {x_m, y_m, z_m});
        build (x_min, y_min, z_min, x_m, y_m, z_m, root->children[0]);
        
        root->children[1] = new Node ({x_min, y_min, z_m + 1}, {x_m, y_m, z_max});
        build (x_min, y_min, z_m + 1, x_m, y_m, z_max, root->children[1]);

        root->children[2] = new Node ({x_m + 1, y_min, z_m+1}, {x_max, y_m, z_max});
        build (x_m+1, y_min, z_m+1, x_max, y_m, z_max, root->children[2]);

        root->children[3] = new Node ({x_m + 1, y_min, z_min}, {x_max, y_m, z_m});
        build (x_min + 1, y_min, z_min, x_max, y_m, z_m, root->children[3]);

        root->children[4] = new Node ({x_min, y_m, z_min}, {x_m, y_max, z_m});
        build (x_min, y_m, z_min, x_m, y_max, z_m, root->children[4]);
        
        root->children[5] = new Node ({x_min, y_min, z_m + 1}, {x_m, y_max, z_max});
        build (x_min, y_min, z_m + 1, x_m, y_max, z_max, root->children[5]);
        
        root->children[6] = new Node ({x_m + 1, y_m + 1, z_m + 1}, {x_max, y_max, z_max});
        build (x_m + 1, y_m + 1, z_m + 1, x_max, y_max, z_max, root->children[6]);
        
        root->children[7] = new Node ({x_m + 1, y_m + 1, z_min}, {x_max, y_max, z_m});
        build (x_m + 1, y_m + 1, z_min, x_max, y_max, z_m, root->children[7]);

    }


    bool check (int x_min, int y_min, int z_min, int x_max, int y_max, int z_max) {
        bool c = img[z_min][y_min][x_min];

        for (int z = z_min; z < z_max; ++z) {
            for (int y = y_min; y < y_max; ++y) {
                for (int x = x_min; x < x_max; ++x) {
                    if (img[z][y][x] != c) return false;
                }
            }
        }

        return true;
    }


    ~OcTree(){}
};