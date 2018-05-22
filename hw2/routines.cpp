#include "structs.hh"
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>


// C split function (borrowed from stackexchange; not sure exactly where
// just Google the code if you want to find it)
vector<string> split(string line, char delim) {
    vector<string> ret;
    string topush;

    // Transition into C:
    char *str = new char[line.size() + 1];
    char *todel = str;
    strcpy(str, line.c_str());

    do {
	// start:
	char *begin = str;

	// inc until at null terminator or delimiter:
	while (*str != delim && *str)
	    str++;
	// string(begin, str) returns a string starting at begin and
	// ending the character before str:
	topush = string(begin, str);
	if(topush != "")
	    ret.push_back(topush);	
    } while (*str++ != 0);

    // the while condition puts str at the point past the null terminator/
    // delimiter; in the former case *str++ evals to the null terminator so
    // the loop ends.
    delete[] todel;
    
    return ret;
}

//**************************************************
// Transformation  matrices w/ standard parameters:
//**************************************************


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

//**************************************************
// SUB-PARSERS
//**************************************************

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


// Object parser (NEEDS TO CHANGE FOR HW2)
Object parse_obj(ifstream &objfile) {
    int i;
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

	else if (tokens[0] == "vn") {
	    Eigen::Vector4d norm;
	    norm << stod(tokens[1]), stod(tokens[2]), stod(tokens[3]), 0;
	    nope.normals.push_back(norm);
	}

	// Add face:
	else if (tokens[0] == "f") {
	    Face face;
	    vector<int> fd;
	    for (i = 1; i < 4; i++) {
		fd.push_back(stoi(split(tokens[i], '/')[0]));
		fd.push_back(stoi(split(tokens[i], '/')[1]));
	    }
	    face.va = fd[0];
	    face.vb = fd[2];
	    face.vc = fd[4];
	    face.na = fd[1];
	    face.nb = fd[3];
	    face.nc = fd[5];
	    nope.faces.push_back(face);
	}
	else {
	    cout << "Bad line '" << line << "', skipping." << endl;
	    continue;
	}
    }
    return nope;
}

vector<Light> parse_lights(ifstream &scfile) {
    vector<string> tokens;
    string line;
    vector<Light> ret;
    while (getline(scfile, line) && !line.empty()) {
	tokens = split(line);
	if (tokens[0] != "light") {
	    cout << "Bad line '" << line << "', expected light directive." << endl;
	    return ret;
	}
	Light toadd;
	toadd.x = stod(tokens[1]);
	toadd.y = stod(tokens[2]);
	toadd.z = stod(tokens[3]);
	toadd.col = Color(stod(tokens[5]), stod(tokens[6]), stod(tokens[7]));
	toadd.k = stod(tokens[9]);
	ret.push_back(toadd);
    }
    return ret;
}

map<string, Object> parse_object_names(ifstream &scfile) {
    vector<string> tokens;
    string line;
    map<string, Object> objmap;
    if (getline(scfile, line) && line == "objects:" ) {
	while (getline(scfile, line) && !line.empty()) {
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
	return objmap;
    }
    return objmap;
}

vector<Object> parse_object_spec(ifstream &scfile, \
				 map<string, Object> objmap) {
    vector<string> tokens;
    vector<Object> objects;
    string line;
    
    while (getline(scfile, line)) {
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
    	Object cobj = objmap[objname];
    	for (int i = 0; i < 4; i++) {
    	    if (getline(scfile, line) && !line.empty()) {
    		tokens = split(line);
    		if ((tokens[0] == "ambient") && (tokens.size() == 4)) 
    		    cobj.amb = Color(stod(tokens[1]), stod(tokens[2]),	\
				     stod(tokens[3]));
		else if ((tokens[0] == "diffuse") && (tokens.size() == 4))
		    cobj.diff = Color(stod(tokens[1]), stod(tokens[2]), \
				     stod(tokens[3]));
		else if ((tokens[0] == "specular") && (tokens.size() == 4))
		    cobj.spec = Color(stod(tokens[1]), stod(tokens[2]), \
				     stod(tokens[3]));
		else if ((tokens[0] == "shininess") && (tokens.size() == 2))
		    cobj.shiny = stod(tokens[1]);
		else
		    cout << "Bad line '" << line << "', looking for material" <<
			" property." << endl;
		
    	    }
    	}
	
    	// Apply the object's transformations:
    	cobj *= parse_transformations(scfile);

	// Add to the list:
    	objects.push_back(cobj);	
    }
    return objects;
}

Camera parse_camera(ifstream &scfile) {
    vector<string> tokens;
    string line;
    Camera c;
    if (getline(scfile, line) && line == "camera:") {
	while (getline(scfile, line) && !line.empty()) {
	    tokens = split(line);
	    if (tokens[0] == "position" && tokens.size() == 4) {
		c.x = stod(tokens[1]);
		c.y = stod(tokens[2]);
		c.z = stod(tokens[3]);
		c.pos = translation(c.x, c.y, c.z);

	    }
	    else if (tokens[0] == "orientation" && tokens.size() == 5) {
		c.ori = rotation(stod(tokens[1]), stod(tokens[2]), \
			       stod(tokens[3]), stod(tokens[4]));
	    }
	    else if (tokens.size() == 2) {
		if (tokens[0] == "near")
		    c.n = stod(tokens[1]);
		else if (tokens[0] == "far")
		    c.f = stod(tokens[1]);
		else if (tokens[0] == "left")
		    c.l = stod(tokens[1]);
		else if (tokens[0] == "right")
		    c.r = stod(tokens[1]);
		else if (tokens[0] == "top")
		    c.t = stod(tokens[1]);
		else if (tokens[0] == "bottom")
		    c.b = stod(tokens[1]);
	    }
	    else {
		cout << "Bad line '" << line << "' in camera directives"<<\
		    ", skipping." << endl;
	    }
	}
    }
    else {
	cout << "Expected camera directive; instead got '" <<
	    line << "', exiting.";
	exit(1);
    }
    return c;
}
