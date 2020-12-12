#include<iostream> 
#include<tuple>
#include <vector>

using namespace std;

typedef tuple <int, int, int> Point;

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
    string filename;
    vector<vector<vector<int>>> img;

public:
    OcTree(string filename) {
        root = nullptr;
        this->filename = filename;
    }

    void build(int x_min, int y_min, int z_min, int x_max, int y_max, int z_max, Node *root) {
        if (check(x_min, y_min, z_min, x_max, y_max, z_max)) {
            root->type = img[x_min][y_min][z_min] == 0 ? empty : full;
            return;
        }

        int x_m = (x_max + x_min) / 2;
        int y_m = (y_max + y_min) / 2;
        int z_m = (z_max + z_min) / 2;

        root->children[0] = new Node ({x_min, y_min, z_min}, {x_m - 1, y_m - 1, z_m - 1});
        build (x_min, y_min, z_min, x_m - 1, y_m - 1, z_m - 1, root->children[0]);
        root->children[1] = new Node ({x_m + 1, y_min, z_min}, {x_max, y_max, z_m - 1});
        root->children[2] = new Node ({x_min, y_m + 1, z_min}, {x_m - 1, y_max, z_max});
        root->children[3] = new Node ({x_min, y_min, z_m + 1}, {x_max, y_m - 1, z_max});
        root->children[4] = new Node ({x_m, y_m + 1, z_m + 1}, {x_m - 1, y_max, z_max});
        root->children[5] = new Node ({x_m + 1, y_min, z_m + 1}, {x_max, y_m - 1, z_max});
        root->children[6] = new Node ({x_m + 1, y_m + 1, z_min}, {x_max, y_max, z_m - 1});
        root->children[7] = new Node ({x_m + 1, y_m + 1, z_m + 1}, {x_max, y_max, z_max});

    }


    bool check (int x_min, int y_min, int z_min, int x_max, int y_max, int z_max) {
        bool c = img[x_min][y_min][z_min];

        for (int x = x_min; x < x_max; ++x) {
            for (int y = y_min; y < y_max; ++y) {
                for (int z = z_min; z < z_max; ++z) {
                    if (img[x][y][z] != c) return false;
                }
            }
        }

        return true;
    }


    ~OcTree(){}
};