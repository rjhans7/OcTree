#pragma once

#include <iostream>
#include <vector>

using namespace std;

typedef unsigned char u_char;
typedef bool cube_type;
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

/*ostream& operator<< (ostream &out, const Punto &p) {
        out << "x: " << p.x << " y: " << p.y << " z: " << p.z;
        return out;
    }
*/
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