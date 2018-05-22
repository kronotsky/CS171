#include <vector>
#include <iostream>
#include <map>
#include <Eigen/Dense>

using namespace std;

#ifndef __VERTEX__
#define __VERTEX__

struct Vertex {
    double x, y, z;
};

#endif

#ifndef __FACE__
#define __FACE__

struct Face {
    int va, vb, vc;
    int na, nb, nc;
};

#endif


#ifndef __OBJECT__
#define __OBJECT__

struct Color {
    double r, g, b;
    Color() : r(0), g(0), b(0) { };
    Color(double ri, double gi, double bi) : r(ri), g(gi), b(bi) { };
    Color(const Color &in) : r(in.r), g(in.g), b(in.b) { };
};

struct Object {
    vector<Eigen::Vector4d> vertices;
    vector<Eigen::Vector4d> normals;
    vector<Face> faces;
    Color amb;
    Color diff;
    Color spec;
    double shiny;
    
    Object() { };
    Object(vector<Eigen::Vector4d> vs, vector<Eigen::Vector4d> ns, vector<Face> fs) \
	: vertices(vs), faces(fs), normals(ns) { };
    Object(const Object &source) : vertices(source.vertices), \
                                   faces(source.faces), \
				   normals(source.normals), \
				   amb(source.amb), \
				   diff(source.diff), \
				   spec(source.spec), \
				   shiny(source.shiny) { };
    
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
	    n.normalize();
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

struct Grid {
    int xres, yres;
    double *r, *g, *b;
    
    Grid(int xr, int yr) : xres(xr), yres(yr) {
	r = new double[xr * yr];
	g = new double[xr * yr];
	b = new double[xr * yr];
	for (int i = 0; i < xr * yr; i++)
	    r[i] = g[i] = b[i] = 0;
    }
    
    Grid(Grid &old) : xres(old.xres), yres(old.yres) {
	r = new double[xres * yres];
	g = new double[xres * yres];
	b = new double[xres * yres];
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
	r[x + y * xres] = col.r;
	g[x + y * xres] = col.g;
	b[x + y * xres] = col.b;
    }

    Color get(int x, int y) const {
	return Color(r[x + y * xres], g[x + y * xres], b[x + y * xres]);
    }
};

struct Light {
    double x, y, z, k;
    Color col;
};

struct Camera {
    Eigen::Matrix4d ori, pos;
    double n, r, l, t, b, f, x, y, z;

    Eigen::Matrix4d camera() {
	return (ori * pos).inverse();
    }

    Eigen::Matrix4d perspective() {
	Eigen::Matrix4d persp;
	persp << (2 * n) / (r - l), 0, (r + l)/(r-l), 0,
	      0, (2 * n) / (t - b), (t + b) / (t - b), 0,
	      0, 0, - (f + n) / (f - n), (-2 * f * n)/(f - n),
	      0, 0, -1, 0;
	return persp;
    }
};

struct Buffer {
    int xres, yres;
    double *depth;

    Buffer(int xr, int yr) : xres(xr), yres(yr) {
	depth = new double[xr * yr];
	for (int i = 0; i < xr * yr; i++) {
	    depth[i] = 1000;
	}
    }

    double get(int x, int y) const {
	return depth[x + y * xres];
    }

    void set(int x, int y, double d) {
	depth[x + y * xres] = d;
    }
};

    


// Function prototypes, to go into routines.cpp:
Eigen::Matrix4d translation(double tx, double ty, double tz);
Eigen::Matrix4d scaling(double sx, double sy, double sz);
Eigen::Matrix4d rotation(double rx, double ry, double rz, double phi);
Eigen::Matrix4d parse_transformations(ifstream &trfile);
vector<string> split(string line, char delim = ' ');
vector<Light> parse_lights(ifstream &scfile);
map<string, Object> parse_object_names(ifstream &scfile);
vector<Object> parse_object_spec(ifstream &scfile, \
				 map<string, Object> objname);
Camera parse_camera(ifstream &scfile);
Object parse_obj(ifstream &objfile);

// Parsing order:

// parse_camera
// parse_lights
// parse_object_names
//   |-- parse_obj
// parse_object_spec
//   |-- parse_transformations



