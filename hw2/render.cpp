#include "structs.hh"
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <cmath>

void grid_to_ppm(Grid &im) {
    Color col;
    int i, j, k;
    cout << "P3" << endl << im.xres << " " << im.yres << endl << 255 << endl;
    for (j = 0; j < im.yres; j++) {
	for (i = 0; i < im.xres; i++) {
	    col = im.get(i, j);
	    cout << (int)(col.r * 255.0) << " "\
		 << (int)(col.g * 255.0) << " "\
		 << (int)(col.b * 255.0) << endl;
	}
    }
}

Color lighting(Object &o, Eigen::Vector4d &p, Eigen::Vector4d &n, Camera &c, \
	       vector<Light> lights) {
    double dot, f;
    Eigen::Vector4d ld, ed;
    // Final colors:
    Color dsum(0, 0, 0);
    Color ssum(0, 0, 0);
    Color ret;

    // Get e - P
    ed << c.x - p(0), c.y - p(1), c.z - p(2), 0;
    ed.normalize();
    for (const auto &l : lights) {
	ld << l.x - p(0), l.y - p(1), l.z - p(2), 0;
	double dist = ld.norm();
	ld.normalize();

	dot = n.dot(ld);
	dot = max(0.0, dot);

	f = dot / (1 + l.k * dist * dist);

	dsum.r += l.col.r * f;
	dsum.g += l.col.g * f;
	dsum.b += l.col.b * f;

	dot = n.dot((ed + ld).normalized());
	dot = pow(max(0.0, dot), o.shiny);

	f = dot / (1 + l.k * dist * dist);

	ssum.r += l.col.r * f;
	ssum.g += l.col.g * f;
	ssum.b += l.col.b * f;
    }
    ret.r = min(1.0, o.amb.r + dsum.r * o.diff.r + ssum.r * o.spec.r);
    ret.g = min(1.0, o.amb.g + dsum.g * o.diff.g + ssum.g * o.spec.g);
    ret.b = min(1.0, o.amb.b + dsum.b * o.diff.b + ssum.b * o.spec.b);
    return ret;
}

Eigen::Vector3d cartesian_vertex(Eigen::Vector4d &v) {
    Eigen::Vector3d vert;
    double w = v(3);
    vert << v(0) / w, v(1) / w, v(2) / w;
    return vert;    
}

inline double abg(int x, int y, int xi, int yi, int xj, int yj) {
    return (yi - yj) * x + (xj - xi) * y + xi * yj - xj * yi;
}

void raster_gouraud(Object &o, Face &f, vector<Light> lights, Camera &c, Grid &gr, Buffer &buf) {
    Eigen::Vector3d va, vb, vc, cross;
    Eigen::Vector4d vat, vbt, vct;
    Eigen::Matrix4d cam = c.perspective() * c.camera();
    int xa, ya, xb, yb, xc, yc, xmin, xmax, ymin, ymax, x, y;
    Color ca, cb, cc;
    double alpha, beta, gamma, r, g, b, depth;
    ca = lighting(o, o.vertices[f.va - 1], o.normals[f.na - 1], c, lights);
    cb = lighting(o, o.vertices[f.vb - 1], o.normals[f.nb - 1], c, lights);
    cc = lighting(o, o.vertices[f.vc - 1], o.normals[f.nc - 1], c, lights);

    vat = cam * o.vertices[f.va - 1];
    vbt = cam * o.vertices[f.vb - 1];
    vct = cam * o.vertices[f.vc - 1];
    va = cartesian_vertex(vat);
    vb = cartesian_vertex(vbt);
    vc = cartesian_vertex(vct);
    
    // Backface culling:
    cross = (vc - vb).cross(va - vb);
    if (cross(2) < 0)
	return;

    // Convert to grid coordinates:
    xa = (int)((va(0) + 1) * (double)((gr.xres - 1) >> 1));
    ya = gr.yres - (int)((va(1) + 1) * (double)((gr.yres - 1) >> 1));
    xb = (int)((vb(0) + 1) * (double)((gr.xres - 1) >> 1));
    yb = gr.yres - (int)((vb(1) + 1) * (double)((gr.yres - 1) >> 1));
    xc = (int)((vc(0) + 1) * (double)((gr.xres - 1) >> 1));
    yc = gr.yres - (int)((vc(1) + 1) * (double)((gr.yres - 1) >> 1));

    // Get minima/maxima
    xmin = min(min(xa, xb), xc);
    ymin = min(min(ya, yb), yc);
    xmax = max(max(xa, xb), xc);
    ymax = max(max(ya, yb), yc);

    for (x = xmin; x <= xmax; x++) {
	for (y = ymin; y <= ymax; y++) {
	    alpha = abg(x, y, xb, yb, xc, yc) / abg(xa, ya, xb, yb, xc, yc);
	    beta = abg(x, y, xa, ya, xc, yc) / abg(xb, yb, xa, ya, xc, yc);
	    gamma = abg(x, y, xa, ya, xb, yb) / abg(xc, yc, xa, ya, xb, yb);

	    if ((0 <= alpha) && (alpha <= 1) && (0 <= beta) && (beta <= 1) &&\
		(0 <= gamma) && (gamma <= 1)) {
		depth = alpha * va(2) + beta * vb(2) + gamma * vc(2);
		
		if ((0 <= x) && (x < gr.xres) && \
		    (0 <= y) && (y < gr.yres) && \
		    (-1 <= depth) && (depth <= 1) && \
		    (depth <= buf.get(x,y))) {

		    buf.set(x, y, depth);
		    r = alpha * ca.r + beta * cb.r + gamma * cc.r;
		    g = alpha * ca.g + beta * cb.g + gamma * cc.g;
		    b = alpha * ca.b + beta * cb.b + gamma * cc.b;
		    gr.fill(x, y, Color(r, g, b));
		}		   
	    }
	}
    }
}

void raster_phong(Object &o, Face &f, vector<Light> lights, Camera &c, Grid &gr, Buffer &buf) {
    Eigen::Vector3d va, vb, vc, cross;
    Eigen::Vector4d vat, vbt, vct, na, nb, nc, vbary, nbary;
    Eigen::Matrix4d cam = c.perspective() * c.camera();
    int xa, ya, xb, yb, xc, yc, xmin, xmax, ymin, ymax, x, y;
    Color ca, cb, cc;
    double alpha, beta, gamma, r, g, b, depth;

    vat = o.vertices[f.va - 1];
    vbt = o.vertices[f.vb - 1];
    vct = o.vertices[f.vc - 1];
    vbary = cam * vat;
    va = cartesian_vertex(vbary);
    vbary = cam * vbt;
    vb = cartesian_vertex(vbary);
    vbary = cam * vct;
    vc = cartesian_vertex(vbary);

    na = o.normals[f.na - 1];
    nb = o.normals[f.nb - 1];
    nc = o.normals[f.nc - 1];
    
    
    // Backface culling:
    cross = (vc - vb).cross(va - vb);
    if (cross(2) < 0)
	return;

    // Convert to grid coordinates:
    xa = (int)((va(0) + 1) * (double)((gr.xres - 1) >> 1));
    ya = gr.yres - (int)((va(1) + 1) * (double)((gr.yres - 1) >> 1));
    xb = (int)((vb(0) + 1) * (double)((gr.xres - 1) >> 1));
    yb = gr.yres - (int)((vb(1) + 1) * (double)((gr.yres - 1) >> 1));
    xc = (int)((vc(0) + 1) * (double)((gr.xres - 1) >> 1));
    yc = gr.yres - (int)((vc(1) + 1) * (double)((gr.yres - 1) >> 1));

    // Get minima/maxima
    xmin = min(min(xa, xb), xc);
    ymin = min(min(ya, yb), yc);
    xmax = max(max(xa, xb), xc);
    ymax = max(max(ya, yb), yc);

    for (x = xmin; x <= xmax; x++) {
	for (y = ymin; y <= ymax; y++) {
	    alpha = abg(x, y, xb, yb, xc, yc) / abg(xa, ya, xb, yb, xc, yc);
	    beta = abg(x, y, xa, ya, xc, yc) / abg(xb, yb, xa, ya, xc, yc);
	    gamma = abg(x, y, xa, ya, xb, yb) / abg(xc, yc, xa, ya, xb, yb);
	    if ((0 <= alpha) && (alpha <= 1) && (0 <= beta) && (beta <= 1) &&\
		(0 <= gamma) && (gamma <= 1)) {
		vbary = alpha * vat + beta * vbt + gamma * vct;
		nbary = alpha * na + beta * nb + gamma * nc;
		depth = alpha * va(2) + beta * vb(2) + gamma * vc(2);
		
		if ((0 <= x) && (x < gr.xres) && \
		    (0 <= y) && (y < gr.yres) && \
		    (-1 <= depth) && (depth <= 1) && \
		    (depth <= buf.get(x,y))) {

		    buf.set(x, y, depth);
		    gr.fill(x, y, lighting(o, vbary, nbary, c, lights));
		}		   
	    }
	}
    }
}

int main(int argc, char *argv[]) {
    ifstream scfile;
    int mode;
    vector<string> tokens;
    vector<Light> lights;
    Camera cam;
    map<string, Object> objmap;
    vector<Object> torender;
    string line;
    double n, f, l, r, t, b;

    int xres, yres;
    if (argc != 5) {
	cout << "Usage: " << argv[0] << " [filename] [xres] [yres] [mode:0/1]" << endl;
	return 1;
    }
    xres = stoi(argv[2]);
    yres = stoi(argv[3]);
    mode = stoi(argv[4]);

    Grid gr(xres, yres);
    Buffer buf(xres, yres);
   
    scfile.open(argv[1]);
    if (!(scfile.is_open())) {
	cout << "Filename " << argv[1] << " not found!" << endl;
	return 1;
    }
    // Get camera transformations:
    cam = parse_camera(scfile);

#ifndef NDEBUG
    cout << "Parsed camera" << endl;
#endif	

    // // Get transformations:
    // persp = perspective(n, r, l, t, b, f);
    // camera = (ori * pos).inverse();

    // Get lights:
    lights = parse_lights(scfile);

#ifndef NDEBUG
    cout << "Parsed lights" << endl;
#endif	

    // Get object map:
    objmap = parse_object_names(scfile);

#ifndef NDEBUG
    cout << "Parsed objects" << endl;
#endif	

    // Find object copies and transformations: 
    torender = parse_object_spec(scfile, objmap);
    for (auto &obj : torender)
	for (auto &face : obj.faces)
	    if (mode)
		raster_gouraud(obj, face, lights, cam, gr, buf);
	    else
		raster_phong(obj, face, lights, cam, gr, buf);
#ifndef NDEBUG
    cout << "Rendered Objects" << endl;
#endif	
    grid_to_ppm(gr);
}
