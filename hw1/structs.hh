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
    vector<Eigen::Vector4d> vertices;
    vector<Face> faces;
    
    Object() { };
    Object(vector<Eigen::Vector4d> vs, vector<Face> fs) : vertices(vs), faces(fs) { };
    Object(const Object &source) : vertices(source.vertices), \
                                   faces(source.faces) { };
    
    Object transformed(Eigen::Matrix4d trans) const {
	Object ret(*this);
	ret *= trans;
	return ret;
    }
    
    Object & operator*=(Eigen::Matrix4d trans) {
	for (auto &v : vertices) {
	    v = trans * v;
	}
	return *this;
    }

    Object & operator=(const Object &rhs) {
	if (&rhs == this) {
	    return *this;
	}
	vertices = rhs.vertices;
	faces = rhs.faces;
	return *this;
    }

    vector<Vertex> cartesian_vertices() const {
	vector<Vertex> ret;
	for (auto &v : vertices) {
	    Vertex vert;
	    double w = v(3);
	    vert.x = v(0) / w;
	    vert.y = v(1) / w;
	    vert.z = v(2) / w;
	    ret.push_back(vert);
	}
	return ret;
    }
    
    void print() const {
	vector<Vertex> verts = this->cartesian_vertices();
	for (int i = 0; i < verts.size(); i++) 
	    cout << verts[i].x << " " << verts[i].y << " " << verts[i].z << endl;
    }
};


#endif

struct Pixel {
    int x, y;
    Pixel() : x(0), y(0) { };
    Pixel(int xi, int yi) : x(xi), y(yi) { };
};

struct Grid {
    int xres, yres;
    int *grid;

    Grid(int xr, int yr) : xres(xr), yres(yr) {
	grid = new int[xr * yr];
	for (int i = 0; i < xr * yr; i++)
	    grid[i] = 0;
    }
    Grid(Grid &old) : Grid(old.xres, old.yres) { }
    
    ~Grid() {
	delete[] grid;
    }
    

    void set(int x, int y, int val) {
	grid[x + y * xres] = val;
    }

    int get(int x, int y) const {
	return grid[x + y * xres];
    }
};



// Function prototypes, to go into routines.cpp:
Eigen::Matrix4d translation(double tx, double ty, double tz);
Eigen::Matrix4d scaling(double sx, double sy, double sz);
Eigen::Matrix4d rotation(double rx, double ry, double rz, double phi);
Eigen::Matrix4d perspective(double n, double r, double l, double t, \
			    double b, double f);
vector<string> split(string line, char delim = ' ');
Object parse_obj(ifstream &objfile);
Object transform(Object &obj, const Eigen::Matrix4d &trans);

