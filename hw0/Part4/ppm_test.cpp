#include <iostream>
#include <stdlib.h>

using namespace std;

void usage(char *filename) {
    cout << "Usage: " << filename << " xres yres" << endl;
}

int main(int argc, char *argv[]) {
    int c1[] = {255, 0, 0}; // Red circle
    int c2[] = {0, 255, 0}; // Green background
    int i, j, k;
    if (argc != 3)
	usage(argv[0]);
    int xres = strtol(argv[1], 0, 0);
    int yres = strtol(argv[2], 0, 0);
    int xc = xres >> 1;
    int yc = yres >> 1;
    // Operator practice:
    int sqdiam = (xres > yres) ? xc*xc : yc*yc;
    cout << "P3" << endl << xres << " " << yres << endl << 255 << endl;
    for (j = -yc; j < yres - yc; j++) {
	for (i = -xc; i < xres - xc; i++) {
	    if (4 * (i*i+ j*j) <= sqdiam)
		for (k = 0; k < 3; k++)
		    cout << c1[k] << " ";
	    else
		for (k = 0; k < 3; k++)
		    cout << c2[k] << " ";
	    cout << endl;
	}
    }
}
