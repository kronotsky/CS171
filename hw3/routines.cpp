#include "structs.hh"
#include <iostream>
#include <string.h>

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
// SUB-PARSERS
//**************************************************

vector<string> parse_transformations(ifstream &trfile) {
    vector<string> lines;
    string line;
     
    while (getline(trfile, line) && !line.empty()) {
	lines.push_back(line);
    }
    return lines;
}


// Object parser 
Object parse_obj(ifstream &objfile) {
    int i;
    vector<Triple> vertices;
    vector<Triple> normals;
    vector<string> tokens;
    vector<string> face_tokens;
    string line;
    Object obj;

    // loop over lines:
    while (getline(objfile, line)) {
	tokens = split(line);
	if (tokens.size() != 4) {
	    cout << "Bad line '" << line << "', skipping." << endl;
	    continue;
	}

	// Add vertex in homogeneous coordinates:
	if (tokens[0] == "v") 
	    vertices.push_back(Triple(stod(tokens[1]),\
				      stod(tokens[2]), stod(tokens[3])));
    	else if (tokens[0] == "vn") 
	    normals.push_back(Triple(stod(tokens[1]),\
				     stod(tokens[2]), stod(tokens[3])));
	// Add face:
	else if (tokens[0] == "f") {
	    vector<string> ft;
	    for (i = 1; i < 4; i++) {
		ft = split(tokens[i], '/');
		obj.vertex_buffer.push_back(vertices[stoi(ft[0]) - 1]);
		obj.normal_buffer.push_back(normals[stoi(ft[1]) - 1]);
	    }
	}
	else {
	    cout << "Bad line '" << line << "', skipping." << endl;
	    continue;
	}
    }
    return obj;
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
	toadd.position[0] = stof(tokens[1]);
	toadd.position[1] = stof(tokens[2]);
	toadd.position[2] = stof(tokens[3]);
	toadd.position[3] = 1.0;
	toadd.color[0] = stof(tokens[5]);
	toadd.color[1] = stod(tokens[6]);
	toadd.color[2] = stod(tokens[7]);
	toadd.attenuation_k = stod(tokens[9]);
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
    		if ((tokens[0] == "ambient") && (tokens.size() == 4)) {
    		    cobj.ambient_reflect[0] = stof(tokens[1]);
		    cobj.ambient_reflect[1] = stof(tokens[2]);
		    cobj.ambient_reflect[2] = stof(tokens[3]);
		}
		else if ((tokens[0] == "diffuse") && (tokens.size() == 4)) {
		    cobj.diffuse_reflect[0] = stof(tokens[1]);
		    cobj.diffuse_reflect[1] = stof(tokens[2]);
		    cobj.diffuse_reflect[2] = stof(tokens[3]);
		}
		else if ((tokens[0] == "specular") && (tokens.size() == 4)) {
		    cobj.specular_reflect[0] = stof(tokens[1]);
		    cobj.specular_reflect[1] = stof(tokens[2]);
		    cobj.specular_reflect[2] = stof(tokens[3]);
		}	
		
		else if ((tokens[0] == "shininess") && (tokens.size() == 2))
		    cobj.shininess = stod(tokens[1]);
		else
		    cout << "Bad line '" << line << "', looking for material" <<
			" property." << endl;
		
    	    }
    	}
	
    	// Apply the object's transformations:
    	cobj.transforms = parse_transformations(scfile);

	// Add to the list:
    	objects.push_back(cobj);	
    }
    return objects;
}

