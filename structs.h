#pragma once

#include <iostream>
#include <vector>
#define PI 3.14159265
using namespace std;

typedef unsigned char u_char;
typedef u_char cube_type;
typedef vector<vector<vector<cube_type>>> Cube;


struct Point {
    int x, y, z;

    Point(){}

    Point (int x, int y, int z) : x (x), y (y), z (z) {}

    /* Producto Escalar */
    int operator*(const Point &p2) {
        return x * p2.x + y * p2.y + z * p2.z;
    }

    /* Producto Vectorial */
    const Point operator%(const Point &p2) const {
        return Point(y * p2.z - z * p2.y, x * p2.z - z * p2.x, x * p2.y - y * p2.x);
    }

    const Point operator+ (const Point &p2) const {
        return Point (x + p2.x, y + p2.y, z + p2.z);
    }

    const Point operator-(Point &p2) const{
        return Point (x - p2.x, y - p2.y, z - p2.z);
    }

    
};

struct Plane {
    Point normal;
    Point p1, p2, p3, p4;
    int d;
    
    Plane(Point p1, Point p2, Point p3, Point p4): p1(p1), p2(p2), p3(p3), p4(p4) {
        Point v1 = p2 - p1;
        Point v2 = p3 - p1;
        normal = v1 % v2;
        d = 0 - normal * p1;
    }

    bool checker(Point k) {
        int r = d + normal * k;
        return (r == 0);
    }

    float distance (Point k) {
        return abs((d + normal * k) / sqrt(pow(normal.x, 2) + pow(normal.y, 2) + pow(normal.z, 2)));
    }

    double insersect(Plane plane_b) {
        double alpha = abs(this->normal*plane_b.normal) / (sqrt(pow(this->normal.x, 2) + pow(this->normal.y, 2) + pow(this->normal.z, 2)) * sqrt(pow(plane_b.normal.x, 2) + pow(plane_b.normal.y, 2) + pow(plane_b.normal.z, 2)));
        return acos (alpha) * 180.0 / PI;
    }
};