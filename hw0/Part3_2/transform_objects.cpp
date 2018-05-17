#include "structs.hh"
#include <fstream>
#include <string>
#include <string.h>
#include <map>

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
	Object trans;
	trans = objs[objname].transformed(cum);
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
