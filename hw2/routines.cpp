#include "structs.hh"
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>


// C split function (borrowed from stackexchange; not sure exactly where
// just Google the code if you want to find it)
vector<string> split(string line, char delim) {
    vector<string> ret;

    // Transition into C:
    char *str;
    strcpy(str, line.c_str());

    do {
	// start:
	char *begin = str;

	// inc until at null terminator or delimiter:
	while (*str != delim && *str)
	    str++;

	// string(begin, str) returns a string starting at begin and
	// ending the character before str:
	ret.push_back(string(begin, str));
    } while (*str++ != 0);

    // the while condition puts str at the point past the null terminator/
    // delimiter; in the former case *str++ evals to the null terminator so
    // the loop ends.
    
    return ret;
}


// Object parser (NEEDS TO CHANGE FOR HW2)
Object parse_obj(ifstream &objfile) {
    vector<string> tokens;
    string line;
    Object nope;

    // loop over lines:
    while (getline(objfile, line)) {
	tokens = split(line);
	if (tokens.size() != 4) {
	    cout << "Bad line '" << line << "', skipping." << endl;
	    continue;
	}

	// Add vertex in homogeneous coordinates:
	if (tokens[0] == "v") {
	    Eigen::Vector4d vert;
	    vert << stod(tokens[1]), stod(tokens[2]), stod(tokens[3]), 1;
	    nope.vertices.push_back(vert);
	}

	// Add face:
	else if (tokens[0] == "f") {
	    Face face;
	    face.a = stoi(tokens[1]);
	    face.b = stoi(tokens[2]);
	    face.c = stoi(tokens[3]);
	    nope.faces.push_back(face);
	}
	else {
	    cout << "Bad line '" << line << "', skipping." << endl;
	    continue;
	}
    }
    return nope;
}


// Transformation  matrices w/ standard parameters:

Eigen::Matrix4d translation(double tx, double ty, double tz) {
    Eigen::Matrix4d t;
    t << 1, 0, 0, tx,
	 0, 1, 0, ty,
         0, 0, 1, tz,
	 0, 0, 0, 1;
    return t;
}

Eigen::Matrix4d scaling(double sx, double sy, double sz) {
    Eigen::Matrix4d s;
    s << sx, 0, 0, 0,
	 0, sy, 0, 0,
	 0, 0, sz, 0,
         0, 0, 0, 1;
    return s;
}

Eigen::Matrix4d rotation(double rx, double ry, double rz, double phi) {
    // Using Rodrigues' formula. The cross product matrix is r:
    Eigen::Matrix4d r;

    // So that r * v is the cross product (rx, ry, rz) x v
    // The matrix is 3d, but this is a 4d subspace embedding:
    r << 0, -rz, ry, 0,
	rz, 0, -rx, 0,
	-ry, rx, 0, 0,
	0, 0, 0, 0;

    // The formula is R = I + r * sin(phi) + r^2 (1 - cos(phi)):
    return Eigen::MatrixXd::Identity(4,4) + r * sin(phi) + (r * r) * (1 - cos(phi));
}

Eigen::Matrix4d perspective(double n, double r, double l,\
			    double t, double b, double f) {
    Eigen::Matrix4d persp;
    persp << (2 * n) / (r - l), 0, (r + l)/(r-l), 0,
	      0, (2 * n) / (t - b), (t + b) / (t - b), 0,
	      0, 0, - (f + n) / (f - n), (-2 * f * n)/(f - n),
	      0, 0, -1, 0;
    return persp;
}

