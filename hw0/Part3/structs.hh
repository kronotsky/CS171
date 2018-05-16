#include <vector>
#include <iostream>
#include <Eigen/Dense>

using namespace std;

// #ifndef __VERTEX__
// #define __VERTEX__

struct Vertex {
    float x, y, z;
};

// #endif

// #ifndef __FACE__
// #define __FACE__

struct Face {
    int a, b, c;
};

// #endif

// #ifndef __OBJECT__
// #define __OBJECT__

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

//#endif

Eigen::Matrix4d translation(double tx, double ty, double tz);
Eigen::Matrix4d scaling(double sx, double sy, double sz);
Eigen::Matrix4d rotation(double rx, double ry, double rz, double phi);
vector<string> split(string line, char delim = ' ');
Object parse_obj(ifstream &objfile);
Object transform(Object &obj, const Eigen::Matrix4d &trans);
