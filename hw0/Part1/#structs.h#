#include <vector>
#include <iostream>

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

    void add_vertex(Vertex v) {
	vertices.push_back(v);
    }

    void add_face(Face f) {
	faces.push_back(f);
    }

    void print() const {
	for (int i = 0; i < vertices.size(); i++) 
	    cout << "v " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << endl;
	for (int i = 0; i < faces.size(); i++)
	    cout << "f " << faces[i].a << " " << faces[i].b << " " << faces[i].c << endl;
    }
};

#endif
