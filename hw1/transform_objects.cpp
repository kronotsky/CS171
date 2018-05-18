#include "structs.hh"
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <map>


Eigen::Matrix4d parse_transformations(ifstream &trfile) {
    vector<string> tokens;
    string line;
    Eigen::Matrix4d cum = Eigen::MatrixXd::Identity(4,4);
    
    while (getline(trfile, line) && !line.empty()) {
	tokens = split(line);
	if ((tokens.size() == 4) && (tokens[0] == "t")) {
	    cum = translation(stod(tokens[1]), stod(tokens[2]),	\
			      stod(tokens[3])) * cum;
	}
	else if ((tokens.size() == 4) && (tokens[0] == "s")) {
	    cum = scaling(stod(tokens[1]), stod(tokens[2]),	\
			   stod(tokens[3])) * cum;
	}
	else if ((tokens.size() == 5) && (tokens[0] == "r")) {
	    cum = rotation(stod(tokens[1]), stod(tokens[2]),		\
			   stod(tokens[3]), stod(tokens[4])) * cum;
	}
	else {
	    cout << "Bad line: '" << line << "', skipping." << endl;
	}
    }
    return cum;
}

vector<Object> parse_scene(ifstream &trfile) {
    vector <string> tokens;
    string line;
    map<string, Object> objmap;
    vector<Object> torender;
    Eigen::Matrix4d pos, ori, camera, persp;
    double n, f, l, r, t, b;

    // Parse camera directives:    
    if (getline(trfile, line) && line == "camera:") {
	while (getline(trfile, line) && !line.empty()) {
	    tokens = split(line);
	    if (tokens[0] == "position" && tokens.size() == 4) {
		pos = translation(stod(tokens[1]), stod(tokens[2]), \
				  stod(tokens[3]));
	    }
	    else if (tokens[0] == "orientation" && tokens.size() == 5) {
		ori = rotation(stod(tokens[1]), stod(tokens[2]), \
			       stod(tokens[3]), stod(tokens[4]));
	    }
	    else if (tokens.size() == 2) {
		if (tokens[0] == "near")
		    n = stod(tokens[1]);
		else if (tokens[0] == "far")
		    f = stod(tokens[1]);
		else if (tokens[0] == "left")
		    l = stod(tokens[1]);
		else if (tokens[0] == "right")
		    r = stod(tokens[1]);
		else if (tokens[0] == "top")
		    t = stod(tokens[1]);
		else if (tokens[0] == "bottom")
		    b = stod(tokens[1]);
	    }
	    else {
		cout << "Bad line '" << line << "' in camera directives"<<\
		    ", skipping." << endl;
	    }
	}
    }
    else {
	cout << "Expected camera directive; instead got '" <<
	    line << "', exiting with empty render list.";
	return torender;
    }

    // Get transformations:
    persp = perspective(n, r, l, t, b, f);
    camera = (ori * pos).inverse();

    // Insert objects into map:
    if (getline(trfile, line) && line == "objects:" ) {
	while (getline(trfile, line) && !line.empty()) {
	    tokens = split(line);
	    if (tokens.size() != 2) {
		cout << "Bad object file line!" << endl;
		continue;
	    }
	    ifstream objfile;
	    objfile.open(tokens[1]);
	    if (!(objfile.is_open())) {
		cout << "Filename " << tokens[1] << " not found!" << endl;
	    }
	    objmap.insert(make_pair(tokens[0], parse_obj(objfile)));
	}
    }
    else {
	cout << "Expected objects directive; instead got '" <<
	    line << "', exiting with empty render list.";
	return torender;
    }


    // Find object copies and transformations: 
    while (getline(trfile, line)) {
	tokens = split(line);

	// Looking for an object name:
	if (tokens.size() != 1) {
	    cout << "Bad syntax line: '" << line << "'" << endl;
	    continue;
	}
	else if (objmap.find(line) == objmap.end()) {
	    cout << "Object '" << line << "' not found" << endl;
	    continue;
	}

	string objname = line;

	// Apply the object's transformations:
	Object trans = objmap[objname].transformed(parse_transformations(trfile));
	// Apply camera transformation then perspective transformation:
	trans *= (persp * camera);
	
	torender.push_back(trans);	
	// cout << objname << endl;
	// trans.print();
    }
    return torender;
}

void rasterize(Pixel a, Pixel b, Grid &im) {
    // Get dx and dy:
    int dx = b.x - a.x;
    int dy = b.y - a.y;

    // Obtain the signs:
    int sx = (dx >= 0) ? 1 : -1;
    int sy = (dy >= 0) ? 1 : -1;

    // bool xlong is true iff abs(dx) >= abs(dy)
    bool xlong;

    // dl and ds serve as "dx and dy" from the first-octant
    // algorithm:
    int dl, ds;

    // Set dl to be the longer dimension length, ds the shorter,
    // determine xlong:
    if (dx * sx >= dy * sy) {
	dl = dx * sx;
	ds = dy * sy;
	xlong = true;
    }
    else {
	dl = dy * sy;
	ds = dx * sx;
	xlong = false;
    }

    // Procede with the algorithm as normal, using l, s instead
    // of x, y:
    int eps = 0;
    int s = 0;
    int l;
    // cout << a.x << " " << a.y << endl;
    // cout << dl << " " << ds << " " << endl;
    // cout << dx << " " << dy << " " << sx << " " << sy << " " << endl;
    for (l = 0; l < dl; l++) {

	// This is where the generalization happens: map l, s onto
	// +/- x, +/- y depending on sx and xlong, and set that
	// pixel:
	if (xlong) 
	    im.set(a.x + l * sx, a.y + s * sy, 255);
	else
	    im.set(a.x + s * sx, a.y + l * sy, 255);

	// Rest of algorithm as normal:
	if (((eps + ds) << 1) < dl) {
	    eps += ds;
	}
	else {
	    eps += ds - dl;
	    s += 1;
	}
    }    
}

void render(Object obj, Grid &im) {
    vector<Pixel> pixels;
    vector<Face> faces = obj.faces;
    vector<Vertex> vertices = obj.cartesian_vertices();
    bool *exclude = new bool[vertices.size()];
    int x, y, i, a, b, c;
    
    for (i = 0; i < vertices.size(); i++) {
	x = (int)((vertices[i].x + 1) * (float)((im.xres - 1) >> 1));
	y = im.yres - (int)((vertices[i].y + 1) * (float)((im.yres - 1) >> 1));
	pixels.push_back(Pixel(x, y));

	if ((0 <= x) && (x < im.xres) && (0 <= y) && (y < im.yres))
	    exclude[i] = 0;
	else 
	    exclude[i] = 1;
    }
    for (const auto &f : faces) {
	a = f.a - 1;
	b = f.b - 1;
	c = f.c - 1;
	if (exclude[a] || exclude[b] || exclude[c])
	    continue;
	rasterize(pixels[a], pixels[b], im);
	rasterize(pixels[b], pixels[c], im);
	rasterize(pixels[c], pixels[a], im);
    }
    delete[] exclude;
}

void grid_to_ppm(Grid &im) {
    int i, j, k;
    cout << "P3" << endl << im.xres << " " << im.yres << endl << 255 << endl;
    for (j = 0; j < im.yres; j++) {
	for (i = 0; i < im.xres; i++) {
	    for (k = 0; k < 3; k++)
		cout << im.get(i, j) << " ";
	    cout << endl;
	}
    }
 }

int main(int argc, char *argv[]) {
    ifstream transfile;
    int xres, yres;
    if (argc != 4) {
	cout << "Usage: " << argv[0] << " [filename] [xres] [yres]" << endl;
	return 1;
    }
    xres = stoi(argv[2]);
    yres = stoi(argv[3]);
    
    transfile.open(argv[1]);
    if (!(transfile.is_open())) {
	cout << "Filename " << argv[1] << " not found!" << endl;
	return 1;
    }
    vector<Object> torender = parse_scene(transfile);
    Grid im(xres, yres);
    for (const auto &obj : torender) {
    	render(obj, im);
    }
    // Pixel a(0, yres);
    // Pixel b(xres / 2, 0);
    // rasterize(b, a, im);
    grid_to_ppm(im);
}


// TO DO: Pretty sure rasterize and grid_to_ppm are 100% functional. Therefore, need
// to debug render.

