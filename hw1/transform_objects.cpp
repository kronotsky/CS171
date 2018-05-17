#include "structs.hh"
#include <fstream>
#include <string>
#include <string.h>
#include <map>

Eigen::Matrix4d parse_transformations(ifstream &trfile) {
    vector<string> tokens;
    string line;

    Eigen::Matrix4d cum = Eigen::MatrixXd::Identity(4,4);
    
    while (getline(trfile, line) && !line.empty()) {
	tokens = split(line);
	if ((tokens.size() == 4) && (tokens[0] == "t")) {
	    cum *= translation(stod(tokens[1]), stod(tokens[2]),	\
			       stod(tokens[3]));
	}
	else if ((tokens.size() == 4) && (tokens[0] == "s")) {
	    cum *= scaling(stod(tokens[1]), stod(tokens[2]),	\
			   stod(tokens[3]));
	}
	else if ((tokens.size() == 5) && (tokens[0] == "r")) {
	    cum *= rotation(stod(tokens[1]), stod(tokens[2]),		\
			    stod(tokens[3]), stod(tokens[4]));
	}
	else {
	    cout << "Bad line: '" << line << "', skipping." << endl;
	}
    }
    return cum;
}

void parse_scene(ifstream &trfile) {
    vector <string> tokens;
    string line;
    map<string, Object> objmap;
    vector<Object> torender;
    Eigen::Matrix4d pos, ori;
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
	    line << "', exiting.";
	return;
    }

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
	    line << "', exiting.";
	return;
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
	cout << objname << endl;
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
    parse_scene(transfile);
}
