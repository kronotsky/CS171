#include <vector>

using namespace std;

#ifndef __VERTEX__
#define __VERTEX__

struct Vertex {
    float x, y, z;
};

struct Face {
    int a, b, c;
};

struct Object {
    vector<Vertex> vertices;
    vector<Face> faces;
};

#endif
