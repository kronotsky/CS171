#include "structs.h"
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>

using namespace std;

vector<string> split(string line, char delim = ' ') {
    stringstream ss(line);
    string next;
    vector<string> ret;
    while (getline(ss, next, delim)) {
	ret.push_back(next);
    }
    return ret;
}

vector<string> split_c(string line, char delim = ' ') {
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
	    nope.add_vertex(vert);
	}
	else if (tokens[0] == "f") {
	    Face face;
	    face.a = stoi(tokens[1]);
	    face.b = stoi(tokens[2]);
	    face.c = stoi(tokens[3]);
	    nope.add_face(face);
	}
    }
    return nope;
}

int main(int argc, char *argv[]) {
    ifstream objfile;
    vector<Object> objs;
    for (int i = 1; i < argc; i++) {
	cout << argv[i] << ":" << endl << "\n";
	objfile.open(argv[i]);
	if (objfile.is_open()) {
	    objs.push_back(parse_obj(objfile));
	}
	else {
	    cout << "Filename " << argv[i] << " not found; skipping.";
	}
	objfile.close();
	objs[i-1].print();
	cout << "\n";
    }
}
