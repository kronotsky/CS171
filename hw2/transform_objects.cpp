#include "structs.hh"
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <cmath>

const double P = 1;

// void render(Object obj, Grid &im) {
//     vector<Pixel> pixels;
//     vector<Face> faces = obj.faces;
//     vector<Vertex> vertices = obj.cartesian_vertices();
//     bool *exclude = new bool[vertices.size()];
//     int x, y, i, a, b, c;
    
//     for (i = 0; i < vertices.size(); i++) {
// 	x = (int)((vertices[i].x + 1) * (float)((im.xres - 1) >> 1));
// 	y = im.yres - (int)((vertices[i].y + 1) * (float)((im.yres - 1) >> 1));
// 	pixels.push_back(Pixel(x, y));

// 	if ((0 <= x) && (x < im.xres) && (0 <= y) && (y < im.yres))
// 	    exclude[i] = 0;
// 	else 
// 	    exclude[i] = 1;
//     }
//     for (const auto &f : faces) {
// 	a = f.a - 1;
// 	b = f.b - 1;
// 	c = f.c - 1;
// 	if (exclude[a] || exclude[b] || exclude[c])
// 	    continue;
// 	rasterize(pixels[a], pixels[b], im);
// 	rasterize(pixels[b], pixels[c], im);
// 	rasterize(pixels[c], pixels[a], im);
//     }
//     delete[] exclude;
// }

// void grid_to_ppm(Grid &im) {
//     int i, j, k;
//     cout << "P3" << endl << im.xres << " " << im.yres << endl << 255 << endl;
//     for (j = 0; j < im.yres; j++) {
// 	for (i = 0; i < im.xres; i++) {
// 	    for (k = 0; k < 3; k++)
// 		cout << im.get(i, j) << " ";
// 	    cout << endl;
// 	}
//     }
// }

Color lighting(Object &o, Eigen::Vector4d &p, Eigen::Vector4d &n, Camera &c, \
	       vector<Light> lights) {
    double dot, f;
    Eigen::Vector3d ld;
    // Final colors:
    Color dsum(0, 0, 0);
    Color ssum(0, 0, 0);
    Color ret;

    // Get e - P
    Eigen::Vector3d ed;
    ed << c.x - p(0), c.y - p(1), c.z - p(2);
    ed.normalize();

    for (const auto &l : lights) {
	ld << l.x - p(0), l.y - p(1), l.z - p(2);
	ld.normalize();

	dot = max(0, ed.dot(ld));

	f = dot / (1 + k * dot * dot);

	dsum.r += l.col.r * f;
	dsum.g += l.col.g * f;
	dsum.b += l.col.b * f;

	dot = pow(max(0, n.dot((ed + ld).normalized())), P);

	f = dot / (1 + k * dot * dot);

	dsum.r += l.col.r * f;
	dsum.g += l.col.g * f;
	dsum.b += l.col.b * f;
    }

    ret.r = min(1, obj.amb.r + dsum.r * obj.diff.r + ssum.r * obj.spec.r);
    ret.g = min(1, obj.amb.g + dsum.g * obj.diff.g + ssum.g * obj.spec.g);
    ret.r = min(1, obj.amb.b + dsum.b * obj.diff.b + ssum.b * obj.spec.b);   
}

int main(int argc, char *argv[]) {
    ifstream scfile;
    vector<string> tokens;
    vector<Light> lights;
    Camera cam;
    map<string, Object> objmap;
    vector<Object> torender;
    string line;
    double n, f, l, r, t, b;

    int xres, yres;
    if (argc != 4) {
	cout << "Usage: " << argv[0] << " [filename] [xres] [yres]" << endl;
	return 1;
    }
    xres = stoi(argv[2]);
    yres = stoi(argv[3]);
    
    scfile.open(argv[1]);
    if (!(scfile.is_open())) {
	cout << "Filename " << argv[1] << " not found!" << endl;
	return 1;
    }
    // Get camera transformations:
    cam = parse_camera(scfile);

    // // Get transformations:
    // persp = perspective(n, r, l, t, b, f);
    // camera = (ori * pos).inverse();

    // Get lights:
    lights = parse_lights(scfile);

    // Get object map:
    objmap = parse_object_names(scfile);

    // Find object copies and transformations: 
    torender = parse_object_spec(scfile, objmap);
}
