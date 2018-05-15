#include <vector>
#include <map>
#include "structs.hh"
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <Eigen/Dense>

vector<string> split(string line, char delim = ' ') {
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
	    Vertex vert;
	    vert.x = stof(tokens[1]);
	    vert.y = stof(tokens[2]);
	    vert.z = stof(tokens[3]);
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

Object transform(Object &obj, const Eigen::Matrix4d &trans) {
    Object ret;
    ret.faces = obj.faces;
    for (auto &v : obj.vertices) {
	Eigen::Vector4d vert;
	vert << v.x, v.y, v.z, 1;
	vert = trans * vert;
	Vertex toadd;
	toadd.x = vert(0);
	toadd.y = vert(1);
	toadd.z = vert(2);
	ret.vertices.push_back(toadd);
    }
    return ret;
}

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


void parse_transform(ifstream &trfile) {
    vector <string> tokens;
    string line;
    map<string, Object> objs;
    map<string, int> copycount;
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
	objs.insert(make_pair(tokens[0], parse_obj(objfile)));
	copycount.insert(make_pair(tokens[0], 0));
    }
    while (getline(trfile, line)) {
	tokens = split(line);
	if (tokens.size() != 1) {
	    cout << "Bad syntax line: '" << line << "'" << endl;
	    continue;
	}
	else if (objs.find(line) == objs.end()) {
	    cout << "Object '" << line << "' not found" << endl;
	    continue;
	}
	string objname = line;
	copycount[objname] += 1;
	Eigen::Matrix4d cum = Eigen::MatrixXd::Identity(4,4);
	while (getline(trfile, line) && !line.empty()) {
	    tokens = split(line);
	    if ((tokens.size() == 4) && (tokens[0] == "t")) {
		cum *= translation(stod(tokens[1]), stod(tokens[2]),\
				   stod(tokens[3]));
	    }
	    else if ((tokens.size() == 4) && (tokens[0] == "s")) {
		cum *= scaling(stod(tokens[1]), stod(tokens[2]), \
			       stod(tokens[3]));
	    }
	    else if ((tokens.size() == 5) && (tokens[0] == "r")) {
		cum *= rotation(stod(tokens[1]), stod(tokens[2]), \
				stod(tokens[3]), stod(tokens[4]));
	    }
	    else {
		cout << "Bad line: '" << line << "', skipping." << endl;
	    }
	}
	Object trans = transform(objs[objname], cum);
	cout << objname << " copy #" << copycount[objname] << endl;
	trans.print();
    }
}

int main(int argc, char *argv[]) {
    ifstream transfile;
    if (argc != 2) {
	cout << "Usage: " << argv[0] << " file " << endl;
	return 1;
    }
    
    transfile.open(argv[1]);
    if (!(transfile.is_open())) {
	cout << "Filename " << argv[1] << " not found!" << endl;
	return 1;
    }
    parse_transform(transfile);
}
