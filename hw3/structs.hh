#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <string>

using namespace std;

struct Light
{
    /* Index 0 has the x-coordinate
     * Index 1 has the y-coordinate
     * Index 2 has the z-coordinate
     * Index 3 has the w-coordinate
     */
    float position[4];
    
    /* Index 0 has the r-component
     * Index 1 has the g-component
     * Index 2 has the b-component
     */
    float color[3];
    
    /* This is our 'k' factor for attenuation as discussed in the lecture notes
     * and extra credit of Assignment 2.
     */
    float attenuation_k;
};

struct Triple
{
    float x;
    float y;
    float z;
    Triple(float xx, float yy, float zz) : x(xx), y(yy), z(zz) { };
};

struct Transforms
{
    /* For each array below,
     * Index 0 has the x-component
     * Index 1 has the y-component
     * Index 2 has the z-component
     */
    float translation[3];
    float rotation[3];
    float scaling[3];
    
    /* Angle in degrees.
     */
    float rotation_angle;
};

struct Object
{
    /* See the note above and the comments in the 'draw_objects' and
     * 'create_cubes' functions for details about these buffer vectors.
     */
    vector<Triple> vertex_buffer;
    vector<Triple> normal_buffer;
    
    vector<string> transforms;
    
    /* Index 0 has the r-component
     * Index 1 has the g-component
     * Index 2 has the b-component
     */
    float ambient_reflect[3];
    float diffuse_reflect[3];
    float specular_reflect[3];
    
    float shininess;
};

vector<string> split(string line, char delim = ' ');
vector<string> parse_transformations(ifstream &trfile);
Object parse_obj(ifstream &objfile);
vector<Light> parse_lights(ifstream &scfile);
map<string, Object> parse_object_names(ifstream &scfile);
vector<Object> parse_object_spec(ifstream &scfile, \
				 map<string, Object> objmap);
