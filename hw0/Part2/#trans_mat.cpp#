#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <cmath>
#include <Eigen/Dense>
#include <Eigen/Geometry>

using namespace std;

vector<string> split(string line, char delim = ' ') {

    // Data structure is a vector of strings:
    vector<string> ret;

    // It's easy to tokenize a c string, so convert:
    char *str;
    strcpy(str, line.c_str());

    do {
	// Beginning of the current token:
	char *begin = str;

	// Increment str until it points to the end of the
	// string or an instance of delim:
	while (*str != delim && *str)
	    str++;

	// string(char *start, char *end) returns a string with
	// first character *start and last character *(end - 1)
	ret.push_back(string(begin, str));
	
    } while (*str++ != 0);
    // If str is pointing to the end of the string, *str++
    // evaluates to '\0' = 0. Otherwise, it points to the
    // delimiter, which we want to skip past.

    // Note that, if consecutive instances of delim are in line,
    // this function will return an empty token between them.

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

Eigen::Matrix4d parse_transformation(ifstream &matfile) {
    vector<string> tokens;
    string line;
    Eigen::Matrix4d cum = Eigen::MatrixXd::Identity(4,4);

    while (getline(matfile, line)) {
	tokens = split(line);
	if ((tokens.size() == 4) && (tokens[0] == "t")) {
	    cum *= translation(stod(tokens[1]), stod(tokens[2]), \
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
    return cum.inverse();
}

int main(int argc, char *argv[]) {
    ifstream matfile;
    if (argc != 2) {
	cout << "Usage: " << argv[0] << " file " << endl;
    }
    matfile.open(argv[1]);
    if (!(matfile.is_open())) {
	cout << "Filename " << argv[1] << " not found!" << endl;
	return 1;
    }
    cout << parse_transformation(matfile) << endl;
    return 0;
}
