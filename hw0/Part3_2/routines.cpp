#include "structs.hh"
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>

vector<string> split(string line, char delim) {
    vector<string> ret;
    char *str;
    strcpy(str, line.c_str());

    do {
	char *begin = str;

	while (*str != delim && *str)
	    str++;

	ret.push_back(string(begin, str));
    } while (*str++ != 0);

    return ret;
}

Object parse_obj(ifstream &objfile) {
    vector<string> tokens;
    string line;
    Object nope;
    while (getline(objfile, line)) {
	tokens = split(line);
	if (tokens.size() != 4) {
	    cout << "Bad line!" << endl;
	    continue;
	}
	if (tokens[0] == "v") {
	    Eigen::Vector4d vert;
	    vert << stod(tokens[1]), stod(tokens[2]), stod(tokens[3]), 1;
	    nope.vertices.push_back(vert);
	}
	else if (tokens[0] == "f") {
	    Face face;
	    face.a = stoi(tokens[1]);
	    face.b = stoi(tokens[2]);
	    face.c = stoi(tokens[3]);
	    nope.faces.push_back(face);
	}
    }
    return nope;
}


// Object transform(Object &obj, const Eigen::Matrix4d &trans) {
//     Object ret;
//     ret.faces = obj.faces;
//     for (auto &v : obj.vertices) {
// 	Eigen::Vector4d vert;
// 	vert << v.x, v.y, v.z, 1;
// 	vert = trans * vert;
// 	Vertex toadd;
// 	toadd.x = vert(0);
// 	toadd.y = vert(1);
// 	toadd.z = vert(2);
// 	ret.vertices.push_back(toadd);
//     }
//     return ret;
// }

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
    // The matrix is 3d, but this is a 4d subspace embedding:
    r << 0, -rz, ry, 0,
	rz, 0, -rx, 0,
	-ry, rx, 0, 0,
	0, 0, 0, 0;

    // This is the formula:
    return Eigen::MatrixXd::Identity(4,4) + r * sin(phi) + (r * r) * (1 - cos(phi));
}
