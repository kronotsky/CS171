#include <vector>
#include <iostream>
#include <Eigen/Dense>

using namespace std;

#ifndef __VERTEX__
#define __VERTEX__

struct Vertex {
    float x, y, z;
};

#endif

#ifndef __FACE__
#define __FACE__
struct Face {
    int a, b, c;
};

#endif

#ifndef __OBJECT__
#define __OBJECT__

struct Object {
    vector<Vertex> vertices;
    vector<Face> faces;

    Object() { };
    Object(vector<Vertex> vs, vector<Face> fs) : vertices(vs), faces(fs) { };

    void print() const {
	for (int i = 0; i < vertices.size(); i++) 
	    cout << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << endl;
    }
};

#endif
