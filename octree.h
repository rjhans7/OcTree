#include<iostream> 
#include<tuple>
using namespace std;

typedef tuple <int, int, int> Point;

class OcTree
{
private:

    enum n_type {full, empty, middle};
    struct Node {
        Point p_start, p_end;
        n_type type;
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


public:
    OcTree(string filename) {
        root = nullptr;
        this->filename = filename;
    }

    void build() {
        

    }

    ~OcTree(){}
};