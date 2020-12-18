#include <iostream>
#include "octree.h"

using namespace std;

int main () {
    Punto p1 (1, 4, 5);
    Punto p2 (3, 1, 3);
    cout << "p1: ";
    cout << p1;
    cout << endl << "p2: " << p2 << endl;
    cout << "producto escalar: " << p1 * p2 << endl;
    cout << "producto vectorial: "; 
    cout << (p1 % p2) << endl;
}