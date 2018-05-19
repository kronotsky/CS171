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
    vector<Eigen::Vector4d> normals;
    vector<Face> faces;
    
    Object() { };
    Object(vector<Eigen::Vector4d> vs, vector<Eigen::4d> ns, vector<Face> fs)\
	: vertices(vs), faces(fs), normals(ns) { };
    Object(const Object &source) : vertices(source.vertices), \
                                   faces(source.faces), \
				   normals(source.normals) { };
    
    Object transformed(Eigen::Matrix4d trans) const {
	Object ret(*this);
	ret *= trans;
	return ret;
    }
    
    Object & operator*=(Eigen::Matrix4d trans) {
	for (auto &v : vertices) {
	    v = trans * v;
	}

	// Note: I am using a representation of normals such that
	// the fourth component is 0. Thus translation matrices are
	// the identity on this subspace, and there is no w component
	// to interfere with the norm calculation.
	for (auto &n : normals) {
	    n = (trans.inverse().transpose()) * n;
	    n = n / n.norm();
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

struct Color {
    float r, g, b;
    Color() : r(0), g(0), b(0) { };
    Color(int ri, int gi, int bi) : r(ri), g(gi), b(bi) { };

    Color & flatten() {
	r = (r > 1) ? 1 : r;
	g = (g > 1) ? 1 : g;
	b = (b > 1) ? 1 : b;	
    }
}

struct Grid {
    int xres, yres;
    int *r, *g, *b;
    
    Grid(int xr, int yr) : xres(xr), yres(yr) {
	r = new int[xr * yr];
	g = new int[xr * yr];
	b = new int[xr * yr];
	for (int i = 0; i < xr * yr; i++)
	    *r = *g = *b = 0;
    }
    
    Grid(Grid &old) : xres(old.xres), yres(old.yres) {
	r = new int[xres * yres];
	g = new int[xres * yres];
	b = new int[xres * yres];
	for (int i = 0; i < xres * yres; i++) {
	    r[i] = old.r[i];
	    g[i] = old.g[i];
	    b[i] = old.b[i];
	}
    }
    
    ~Grid() {
	delete[] r;
	delete[] g;
	delete[] b;
    }
    

    void fill(int x, int y, Color col) {
	r[x + y * xres] = (int)(255 * col.r);
	g[x + y * xres] = (int)(255 * col.g);
	b[x + y * xres] = (int)(255 * col.b);	
    }

    Color get(int x, int y) const {
	return Color((float)(r[x + y * res]) / 255.0 \
		     (float)(g[x + y * res]) / 255.0 \
		     (float)(b[x + y * res]) / 255.0)
		     
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

