#include <GL/glew.h>
#include <GL/glut.h>
#include "structs.hh"
#include "quaternion.hh"
#include <math.h>
#include <Eigen/Dense>
#define _USE_MATH_DEFINES

using namespace std;

void init(void);
void reshape(int width, int height);
void display(void);

void init_lights();
void set_lights();
void draw_objects();

void mouse_pressed(int button, int state, int x, int y);
void mouse_moved(int x, int y);
void key_pressed(unsigned char key, int x, int y);

void set_current_rotation(Quat r);
Quat compute_rotation(int pxc, int pyc, int pxs, int pys, int xres, int yres);

/* Index 0 has the x-coordinate
 * Index 1 has the y-coordinate
 * Index 2 has the z-coordinate
 */
float cam_position[] = {0.9, 0, 5.4};
float cam_orientation_axis[] = {0, 0, 1};

/* Angle in degrees.
 */ 
float cam_orientation_angle = 0;

float near_param = 1, far_param = 20,
      left_param = -0.5, right_param = 0.5,
      top_param = 0.5, bottom_param = -0.5;

vector<Light> lights;
vector<Object> objects;

int mouse_x, mouse_y;
float mouse_scale_x, mouse_scale_y;

const float step_size = 0.2;
const float x_view_step = 90.0, y_view_step = 90.0;
float x_view_angle = 0, y_view_angle = 0;

Quat last_rot = 1.0;
Quat cur_rot = 1.0;
double mult_mat[16] = {0};

bool is_pressed = false;
bool wireframe_mode = false;
bool arcball_mode = false;

int xres;
int yres;

void create_lights();
void create_cubes();

// Camera parser:
void parse_camera(ifstream &scfile) {
    vector<string> tokens;
    string line;
    if (getline(scfile, line) && line == "camera:") {
	while (getline(scfile, line) && !line.empty()) {
	    tokens = split(line);
	    if (tokens[0] == "position" && tokens.size() == 4) {
		cam_position[0] = stod(tokens[1]);
		cam_position[1] = stod(tokens[2]);
		cam_position[2] = stod(tokens[3]);
	    }
	    else if (tokens[0] == "orientation" && tokens.size() == 5) {
		cam_orientation_axis[0] = stod(tokens[1]);
		cam_orientation_axis[1] = stod(tokens[2]);
		cam_orientation_axis[2] = stod(tokens[3]);
		cam_orientation_angle = stod(tokens[4]);
	    }
	    else if (tokens.size() == 2) {
		if (tokens[0] == "near")
		    near_param = stod(tokens[1]);
		else if (tokens[0] == "far")
		    far_param = stod(tokens[1]);
		else if (tokens[0] == "left")
		    left_param = stod(tokens[1]);
		else if (tokens[0] == "right")
		    right_param = stod(tokens[1]);
		else if (tokens[0] == "top")
		    top_param = stod(tokens[1]);
		else if (tokens[0] == "bottom")
		    bottom_param = stod(tokens[1]);
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
}

void init(void)
{
    // GOURAUD SHADING:
    glShadeModel(GL_SMOOTH);
    
    // CULL:
    glEnable(GL_CULL_FACE);
    // W/ BACKFACE:
    glCullFace(GL_BACK);
    
    // DEPTH BUFFERING:
    glEnable(GL_DEPTH_TEST);

    // Normalize normals:
    glEnable(GL_NORMALIZE);
    
    /* The following two lines tell OpenGL to enable its "vertex array" and
     * "normal array" functionality. More details on these arrays are given
     * in the comments on the 'Object' struct and the 'draw_objects' and
     * 'create_objects' functions.
     */
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    
    /* The next 4 lines work with OpenGL's two main matrices: the "Projection
     * Matrix" and the "Modelview Matrix". Only one of these two main matrices
     * can be modified at any given time. We specify the main matrix that we
     * want to modify with the 'glMatrixMode' function.
     *
     * The Projection Matrix is the matrix that OpenGL applies to points in
     * camera space. For our purposes, we want the Projection Matrix to be
     * the perspective projection matrix, since we want to convert points into
     * NDC after they are in camera space.
     *
     * The line of code below:
     */
    glMatrixMode(GL_PROJECTION);
    /* ^tells OpenGL that we are going to modify the Projection Matrix. From
     * this point on, any matrix comamnds we give OpenGL will affect the
     * Projection Matrix. For instance, the line of code below:
     */
    glLoadIdentity();
    /* ^tells OpenGL to set the current main matrix (which is the Projection
     * Matrix right now) to the identity matrix. Then, the next line of code:
     */
    glFrustum(left_param, right_param,
              bottom_param, top_param,
              near_param, far_param);
    /* ^ tells OpenGL to create a perspective projection matrix using the
     * given frustum parameters. OpenGL then post-multiplies the current main
     * matrix (the Projection Matrix) with the created matrix. i.e. let 'P'
     * be our Projection Matrix and 'F' be the matrix created by 'glFrustum'.
     * Then, after 'F' is created, OpenGL performs the following operation:
     *
     * P = P * F
     * 
     * Since we had set the Projection Matrix to the identity matrix before the
     * call to 'glFrustum', the above multiplication results in the Projection
     * Matrix being the perspective projection matrix, which is what we want.
     */
    
    /* The Modelview Matrix is the matrix that OpenGL applies to untransformed
     * points in world space. OpenGL applies the Modelview Matrix to points
     * BEFORE it applies the Projection Matrix.
     * 
     * Thus, for our purposes, we want the Modelview Matrix to be the overall
     * transformation matrix that we apply to points in world space before
     * applying the perspective projection matrix. This means we would need to
     * factor in all the individual object transformations and the camera
     * transformations into the Modelview Matrix.
     *
     * The following line of code tells OpenGL that we are going to modify the
     * Modelview Matrix. From this point on, any matrix commands we give OpenGL
     * will affect the Modelview Matrix.
     *
     * We generally modify the Modelview Matrix in the 'display' function,
     * right before we tell OpenGL to render anything. See the 'display'
     * for details.
     */
    glMatrixMode(GL_MODELVIEW);
    
    /* The next two lines call our 2 helper functions that create some Point
     * Light and Object structs for us to render. Further details will be given
     * in the functions themselves.
     *
     * The reason we have these procedures as separate functions is to make
     * the code more organized.
     */
    
    /* The next line calls our function that tells OpenGL to initialize some
     * lights to represent our Point Light structs. Further details will be
     * given in the function itself.
     *
     * The reason we have this procedure as a separate function is to make
     * the code more organized.
     */
    init_lights();
}

/* 'reshape' function:
 * 
 * You will see down below in the 'main' function that whenever we create a
 * window in OpenGL, we have to specify a function for OpenGL to call whenever
 * the window resizes. We typically call this function 'reshape' or 'resize'.
 * 
 * The 'reshape' function is supposed to tell your program how to react
 * whenever the program window is resized. It is also called in the beginning
 * when the window is first created. You can think of the first call to
 * 'reshape' as an initialization phase and all subsequent calls as update
 * phases.
 * 
 * Anything that needs to know the dimensions of the program window should
 * be initialized and updated in the 'reshape' function. You will see below
 * that we use the 'reshape' function to initialize and update the conversion
 * scheme between NDC and screen coordinates as well as the mouse interaction
 * parameters.
 */
void reshape(int width, int height)
{
    /* The following two lines of code prevent the width and height of the
     * window from ever becoming 0 to prevent divide by 0 errors later.
     * Typically, we let 1x1 square pixel be the smallest size for the window.
     */
    height = (height == 0) ? 1 : height;
    width = (width == 0) ? 1 : width;
    
    /* The 'glViewport' function tells OpenGL to determine how to convert from
     * NDC to screen coordinates given the dimensions of the window. The
     * parameters for 'glViewport' are (in the following order):
     *
     * - int x: x-coordinate of the lower-left corner of the window in pixels
     * - int y: y-coordinate of the lower-left corner of the window in pixels
     * - int width: width of the window
     * - int height: height of the window
     *
     * We typically just let the lower-left corner be (0,0).
     *
     * After 'glViewport' is called, OpenGL will automatically know how to
     * convert all our points from NDC to screen coordinates when it tries
     * to render them.
     */
    glViewport(0, 0, width, height);
    
    /* The following two lines are specific to updating our mouse interface
     * parameters. Details will be given in the 'mouse_moved' function.
     */
    mouse_scale_x = (float) (right_param - left_param) / (float) width;
    mouse_scale_y = (float) (top_param - bottom_param) / (float) height;
   
    /* The following line tells OpenGL that our program window needs to
     * be re-displayed, meaning everything that was being displayed on
     * the window before it got resized needs to be re-rendered.
     */
    glutPostRedisplay();
}

/* 'display' function:
 * 
 * You will see down below in the 'main' function that whenever we create a
 * window in OpenGL, we have to specify a function for OpenGL to call whenever
 * it wants to render anything. We typically name this function 'display' or
 * 'render'.
 *
 * The 'display' function is supposed to handle all the processing of points
 * in world and camera space.
 */
void display(void)
{
    /* The following line of code is typically the first line of code in any
     * 'display' function. It tells OpenGL to reset the "color buffer" (which
     * is our pixel grid of RGB values) and the depth buffer.
     *
     * Resetting the "color buffer" is equivalent to clearing the program
     * window so that it only displays a black background. This allows OpenGL
     * to render a new scene onto the window without having to deal with the
     * remnants of the previous scene.
     *
     * Resetting the depth buffer sets all the values in the depth buffer back
     * to a very high number. This allows the depth buffer to be reused for
     * rendering a new scene.
     */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    /* With the program window cleared, OpenGL is ready to render a new scene.
     * Of course, before we can render anything correctly, we need to make all
     * the appropriate camera and object transformations to our coordinate
     * space.
     *
     * Recall that the 'init' function used the glMatrixMode function to put
     * OpenGL into a state where we can modify its Modelview Matrix. Also
     * recall that we want the Modelview Matrix to be the overall transform-
     * ation matrix that we apply to points in world space before applying the
     * perspective projection matrix. This means that we need to factor in all
     * the individual object transformations and the camera transformations
     * into the Modelview Matrix.
     *
     * To do so, our first step is to "reset" the Modelview Matrix by setting it
     * to the identity matrix:
     */
    glLoadIdentity();
    /* Now, if you recall, for a given object, we want to FIRST multiply the
     * coordinates of its points by the translations, rotations, and scalings
     * applied to the object and THEN multiply by the inverse camera rotations
     * and translations.
     *
     * HOWEVER, OpenGL modifies the Modelview Matrix using POST-MULTIPLICATION.
     * This means that if were to specify to OpenGL a matrix modification 'A',
     * then letting the Modelview Matrix be 'M', OpenGL would perform the
     * following operation:
     *
     * M = M * A
     *
     * So, for instance, if the Modelview Matrix were initialized to the
     * identity matrix 'I' and we were to specify a translation 'T' followed by
     * a rotation 'R' followed by a scaling 'S' followed by the inverse camera
     * transform 'C', then the Modelview Matrix is modified in the following
     * order:
     * 
     * M = I * T * R * S * C
     * 
     * Then, when OpenGL applies the Modelview Matrix to a point 'p', we would
     * get the following multiplication:
     *
     * M * p = I * T * R * S * C * p
     *
     * ^ So the camera transformation ends up being applied first even though
     * it was specified last. This is not what we want. What we want is
     * something like this:
     *
     * M * p = C * T * R * S * I * p
     *
     * Hence, to correctly transform a point, we actually need to FIRST specify
     * the inverse camera rotations and translations and THEN specify the
     * translations, rotations, and scalings applied to an object.
     *
     * We start by specifying any camera rotations caused by the mouse. We do
     * so by using the 'glRotatef' function, which takes the following parameters
     * in the following order:
     * 
     * - float angle: rotation angle in DEGREES
     * - float x: x-component of rotation axis
     * - float y: y-component of rotation axis
     * - float z: z-component of rotation axis
     *
     * The 'glRotatef' function tells OpenGL to create a rotation matrix using
     * the given angle and rotation axis.
     */
    glRotatef(y_view_angle, 1, 0, 0);
    glRotatef(x_view_angle, 0, 1, 0);
    /* 'y_view_angle' and 'x_view_angle' are parameters for our mouse user
     * interface. They keep track of how much the user wants to rotate the
     * camera from its default, specified orientation. See the 'mouse_moved'
     * function for more details.
     *
     * Our next step is to specify the inverse rotation of the camera by its
     * orientation angle about its orientation axis:
     */
    glRotatef(-cam_orientation_angle,
              cam_orientation_axis[0], cam_orientation_axis[1], cam_orientation_axis[2]);
    /* We then specify the inverse translation of the camera by its position using
     * the 'glTranslatef' function, which takes the following parameters in the
     * following order:
     *
     * - float x: x-component of translation vector
     * - float y: x-component of translation vector
     * - float z: x-component of translation vector
     */
    glTranslatef(-cam_position[0], -cam_position[1], -cam_position[2]);
    /* ^ And that should be it for the camera transformations.
     */

    // After setting up the screen and the camera, the next step is to post the redisplay:
    set_current_rotation(cur_rot * last_rot);
    glMultMatrixd(mult_mat);
    
    /* Our next step is to set up all the lights in their specified positions.
     * Our helper function, 'set_lights' does this for us. See the function
     * for more details.
     *
     * The reason we have this procedure as a separate function is to make
     * the code more organized.
     */

    set_lights();
    /* Once the lights are set, we can specify the points and faces that we
     * want drawn. We do all this in our 'draw_objects' helper function. See
     * the function for more details.
     *
     * The reason we have this procedure as a separate function is to make
     * the code more organized.
     */
    draw_objects();
    
    /* The following line of code has OpenGL do what is known as "double
     * buffering".
     *
     * Imagine this: You have a relatively slow computer that is telling OpenGL
     * to render something to display in the program window. Because your
     * computer is slow, OpenGL ends up rendering only part of the scene before
     * it displays it in the program window. The rest of the scene shows up a
     * second later. This effect is referred to as "flickering". You have most
     * likely experienced this sometime in your life when using a computer. It
     * is not the most visually appealing experience, right?
     *
     * To avoid the above situation, we need to tell OpenGL to display the
     * entire scene at once rather than rendering the scene one pixel at a
     * time. We do so by enabling "double buffering".
     * 
     * Basically, double buffering is a technique where rendering is done using
     * two pixel grids of RGB values. One pixel grid is designated as the
     * "active buffer" while the other is designated as the "off-screen buffer".
     * Rendering is done on the off-screen buffer while the active buffer is
     * being displayed. Once the scene is fully rendered on the off-screen buffer,
     * the two buffers switch places so that the off-screen buffer becomes the
     * new active buffer and gets displayed while the old active buffer becomes
     * the new off-screen buffer. This process allows scenes to be fully rendered
     * onto the screen at once, avoiding the flickering effect.
     * 
     * We actually enable double buffering in the 'main' function with the line:
     
       glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
       
     * ^ 'GLUT_DOUBLE' tells OpenGL to use double buffering. The other two
     * parameters, 'GLUT_RGB' and 'GLUT_DEPTH', tell OpenGL to initialize the
     * RGB pixel grids and the depth buffer respectively.
     *
     * The following function, 'glutSwapBuffers', tells OpenGL to swap the
     * active and off-screen buffers.
     */
    glutSwapBuffers();
}

/* 'init_lights' function:
 * 
 * This function has OpenGL enable its built-in lights to represent our point
 * lights.
 *
 * OpenGL has 8 built-in lights in all, each one with its own unique, integer
 * ID value. When setting the properties of a light, we need to tell OpenGL
 * the ID value of the light we are modifying.
 * 
 * The first light's ID value is stored in 'GL_LIGHT0'. The second light's ID
 * value is stored in 'GL_LIGHT1'. And so on. The eighth and last light's ID
 * value is stored in 'GL_LIGHT7'.
 *
 * The properties of the lights are set using the 'glLightfv' and 'glLightf'
 * functions as you will see below.
 */
void init_lights()
{
    /* The following line of code tells OpenGL to enable lighting calculations
     * during its rendering process. This tells it to automatically apply the
     * Phong reflection model or lighting model to every pixel it will render.
     */
    glEnable(GL_LIGHTING);
    
    int num_lights = lights.size();
    
    for(int i = 0; i < num_lights; ++i)
    {
        /* In this loop, we are going to associate each of our point lights
         * with one of OpenGL's built-in lights. The simplest way to do this
         * is to just let our first point light correspond to 'GL_LIGHT0', our
         * second point light correspond to 'GL_LIGHT1', and so on. i.e. let:
         * 
         * 'lights[0]' have an ID value of 'GL_LIGHT0'
         * 'lights[1]' have an ID value of 'GL_LIGHT1'
         * etc...
         */
        int light_id = GL_LIGHT0 + i;
        
        glEnable(light_id);
        
        /* The following lines of code use 'glLightfv' to set the color of
         * the light. The parameters for 'glLightfv' are:
         *
         * - enum light_ID: an integer between 'GL_LIGHT0' and 'GL_LIGHT7'
         * - enum property: this varies depending on what you are setting
         *                  e.g. 'GL_AMBIENT' for the light's ambient component
         * - float* values: a set of values to set for the specified property
         *                  e.g. an array of RGB values for the light's color
         * 
         * OpenGL actually lets us specify different colors for the ambient,
         * diffuse, and specular components of the light. However, since we
         * are used to only working with one overall light color, we will
         * just set every component to the light color.
         */
        glLightfv(light_id, GL_AMBIENT, lights[i].color);
        glLightfv(light_id, GL_DIFFUSE, lights[i].color);
        glLightfv(light_id, GL_SPECULAR, lights[i].color);
        
        /* The following line of code sets the attenuation k constant of the
         * light. The difference between 'glLightf' and 'glLightfv' is that
         * 'glLightf' is used for when the parameter is only one value like
         * the attenuation constant while 'glLightfv' is used for when the
         * parameter is a set of values like a color array. i.e. the third
         * parameter of 'glLightf' is just a float instead of a float*.
         */
        glLightf(light_id, GL_QUADRATIC_ATTENUATION, lights[i].attenuation_k);
    }
}

/* 'set_lights' function:
 *
 * While the 'init_lights' function enables and sets the colors of the lights,
 * the 'set_lights' function is supposed to position the lights.
 *
 * You might be wondering why we do not just set the positions of the lights in
 * the 'init_lights' function in addition to the other properties. The reason
 * for this is because OpenGL does lighting computations after it applies the
 * Modelview Matrix to points. This means that the lighting computations are
 * effectively done in camera space. Hence, to ensure that we get the correct
 * lighting computations, we need to make sure that we position the lights
 * correctly in camera space.
 * 
 * Now, the 'glLightfv' function, when used to position a light, applies all
 * the current Modelview Matrix to the given light position. This means that
 * to correctly position lights in camera space, we should call the 'glLightfv'
 * function to position them AFTER the Modelview Matrix has been modified by
 * the necessary camera transformations. As you can see in the 'display'
 * function, this is exactly what we do.
 */
void set_lights()
{
    int num_lights = lights.size();
    
    for(int i = 0; i < num_lights; ++i)
    {
        int light_id = GL_LIGHT0 + i;
        
        glLightfv(light_id, GL_POSITION, lights[i].position);
    }
}

/* 'draw_objects' function:
 *
 * This function has OpenGL render our objects to the display screen. It
 */
void draw_objects()
{
    int num_objects = objects.size();
    
    for(int i = 0; i < num_objects; ++i)
    {
        /* The current Modelview Matrix is actually stored at the top of a
         * stack in OpenGL. The following function, 'glPushMatrix', pushes
         * another copy of the current Modelview Matrix onto the top of the
         * stack. This results in the top two matrices on the stack both being
         * the current Modelview Matrix. Let us call the copy on top 'M1' and
         * the copy that is below it 'M2'.
         *
         * The reason we want to use 'glPushMatrix' is because we need to
         * modify the Modelview Matrix differently for each object we need to
         * render, since each object is affected by different transformations.
         * We use 'glPushMatrix' to essentially keep a copy of the Modelview
         * Matrix before it is modified by an object's transformations. This
         * copy is our 'M2'. We then modify 'M1' and use it to render the
         * object. After we finish rendering the object, we will pop 'M1' off
         * the stack with the 'glPopMatrix' function so that 'M2' returns to
         * the top of the stack. This way, we have the old unmodified Modelview
         * Matrix back to edit for the next object we want to render.
         */
        glPushMatrix();
        /* The following brace is not necessary, but it keeps things organized.
         */
        {
            int n = objects[i].transforms.size();
            
            /* The loop tells OpenGL to modify our modelview matrix with the
             * desired geometric transformations for this object. Remember
             * though that our 'transform_sets' struct assumes that transformations
             * are conveniently given in sets of translation -> rotate -> scaling;
             * and THIS IS NOT THE CASE FOR YOUR SCENE FILES. DO NOT BLINDLY
             * COPY THE FOLLOWING CODE.
             *
             * To explain how to correctly transform your objects, consider the
             * following example. Suppose our object has the following desired
             * transformations:
             *
             *    scale by 2, 2, 2
             *    translate by 2, 0, 5
             *    rotate about 0, 1, 0.5 and by angle = 0.6 radians
             *
             * Obviously, we cannot use the following loop for this, because the order
             * is not translate -> rotate -> scale. Instead, we need to make the following
             * calls in this exact order:
             *
             *    glRotatef(0.6 * 180.0 / M_PI, 0, 1, 0.5);
             *    glTranslatef(2, 0, 5);
             *    glScalef(2, 2, 2);
             *
             * We make the calls in the REVERSE order of how the transformations are specified
             * because OpenGL edits our modelview matrix using post-multiplication (see above
             * at the notes regarding the camera transforms in display()).
             *
             * Keep all this in mind to come up with an appropriate way to store and apply
             * geometric transformations for each object in your scenes.
             */
	    vector<string> tokens;
	    for(int j = 1; j <= n; j++) {
		tokens = split(objects[i].transforms[n - j]);
		if ((tokens.size() == 4) && (tokens[0] == "t")) 
		    glTranslatef(stof(tokens[1]), stof(tokens[2]), 
				 stod(tokens[3]));
		else if ((tokens.size() == 4) && (tokens[0] == "s")) 
		    glScalef(stof(tokens[1]), stof(tokens[2]),
			     stof(tokens[3]));
		else if ((tokens.size() == 5) && (tokens[0] == "r")) 
		    glRotatef(stof(tokens[4]) * 180.0 / M_PI, stof(tokens[1]),
			      stof(tokens[2]), stof(tokens[3]));
	    }

            glMaterialfv(GL_FRONT, GL_AMBIENT, objects[i].ambient_reflect);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, objects[i].diffuse_reflect);
            glMaterialfv(GL_FRONT, GL_SPECULAR, objects[i].specular_reflect);
            glMaterialf(GL_FRONT, GL_SHININESS, objects[i].shininess);
            
            glVertexPointer(3, GL_FLOAT, 0, &objects[i].vertex_buffer[0]);
            /* The "normal array" is the equivalent array for normals.
             * Each normal in the normal array corresponds to the vertex
             * of the same index in the vertex array.
             *
             * The 'glNormalPointer' function has the following parameters:
             *
             * - enum type_of_normals: e.g. int, float, double, etc
             * - sizei stride: same as the stride parameter in 'glVertexPointer'
             * - void* pointer_to_array: the pointer to the normal array
             */
            glNormalPointer(GL_FLOAT, 0, &objects[i].normal_buffer[0]);
            
            int buffer_size = objects[i].vertex_buffer.size();
            
            if(!wireframe_mode)
                /* Finally, we tell OpenGL to render everything with the
                 * 'glDrawArrays' function. The parameters are:
                 * 
                 * - enum mode: in our case, we want to render triangles,
                 *              so we specify 'GL_TRIANGLES'. If we wanted
                 *              to render squares, then we would use
                 *              'GL_QUADS' (for quadrilaterals).
                 * - int start_index: the index of the first vertex
                 *                    we want to render in our array
                 * - int num_vertices: number of vertices to render
                 *
                 * As OpenGL renders all the faces, it automatically takes
                 * into account all the specifications we have given it to
                 * do all the lighting calculations for us. It also applies
                 * the Modelview and Projection matrix transformations to
                 * the vertices and converts everything to screen coordinates
                 * using our Viewport specification. Everything is rendered
                 * onto the off-screen buffer.
                 */
                glDrawArrays(GL_TRIANGLES, 0, buffer_size);
            else
                /* If we are in "wireframe mode" (see the 'key_pressed'
                 * function for more information), then we want to render
                 * lines instead of triangle surfaces. To render lines,
                 * we use the 'GL_LINE_LOOP' enum for the mode parameter.
                 * However, we need to draw each face frame one at a time
                 * to render the wireframe correctly. We can do so with a
                 * for loop:
                 */
                for(int j = 0; j < buffer_size; j += 3)
                    glDrawArrays(GL_LINE_LOOP, j, 3);
        }
        /* As discussed before, we use 'glPopMatrix' to get back the
         * version of the Modelview Matrix that we had before we specified
         * the object transformations above. We then move on in our loop
         * to the next object we want to render.
         */
        glPopMatrix();
    }
}

Quat compute_rotation(int pxc, int pyc, int pxs, int pys, int xres, int yres) {
    Eigen::Vector3d p1, p2, c;
    double p1x, p1y, p2x, p2y;
    double theta;
    p1x = 2 * (double)(pxs) / (double)(xres) - 1;
    p2x = 2 * (double)(pxc) / (double)(xres) - 1;
    p1y = 1 - 2 * (double)(pys) / (double)(yres);
    p2y = 1 - 2 * (double)(pyc) / (double)(yres);

    p1 << p1x, p1y, ((p1x * p1x + p1y * p1y) < 1) ? sqrt(1 - p1x * p1x - p1y * p1y) : 0;
    p2 << p2x, p2y, ((p2x * p2x + p2y * p2y) < 1) ? sqrt(1 - p2x * p2x - p2y * p2y) : 0;

    theta = p1.dot(p2) / (p1.norm() * p2.norm());
    theta = acos(min(1.0, theta));
    c = p1.cross(p2);
    c.normalize();
    c *= sin(theta / 2);
    return Quat(cos(theta / 2), c(0), c(1), c(2));
}

void set_current_rotation(Quat r) {
    // First column:
    mult_mat[0] = 1 - 2 * r.q[2] * r.q[2] - 2 * r.q[3] * r.q[3];
    mult_mat[1] = 2 * (r.q[1] * r.q[2] + r.q[3] * r.q[0]);
    mult_mat[2] = 2 * (r.q[1] * r.q[3] - r.q[2] * r.q[0]);
    mult_mat[3] = 0;

    // Second column:
    mult_mat[4] = 2 * (r.q[1] * r.q[2] - r.q[3] * r.q[0]);
    mult_mat[5] = 1 - 2 * r.q[1] * r.q[1] - 2 * r.q[3] * r.q[3];
    mult_mat[6] = 2 * (r.q[3] * r.q[2] + r.q[1] * r.q[0]);
    mult_mat[7] = 0;

    // Third column:
    mult_mat[8] = 2 * (r.q[1] * r.q[3] + r.q[2] * r.q[0]);
    mult_mat[9] = 2 * (r.q[3] * r.q[2] - r.q[1] * r.q[0]); 
    mult_mat[10] = 1 - 2 * r.q[1] * r.q[1] - 2 * r.q[2] * r.q[2];
    mult_mat[11] = 0;

    // Fourth column:
    mult_mat[12] = 0;
    mult_mat[13] = 0;
    mult_mat[14] = 0;	
    mult_mat[15] = 1;
}

/* 'mouse_pressed function:
 * 
 * This function is meant to respond to mouse clicks and releases. The
 * parameters are:
 * 
 * - int button: the button on the mouse that got clicked or released,
 *               represented by an enum
 * - int state: either 'GLUT_DOWN' or 'GLUT_UP' for specifying whether the
 *              button was pressed down or released up respectively
 * - int x: the x screen coordinate of where the mouse was clicked or released
 * - int y: the y screen coordinate of where the mouse was clicked or released
 *
 * The function doesn't really do too much besides set some variables that
 * we need for the 'mouse_moved' function.
 */
void mouse_pressed(int button, int state, int x, int y)
{
    /* If the left-mouse button was clicked down, then...
     */
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        /* Store the mouse position in our global variables.
         */
        mouse_x = x;
        mouse_y = y;
        
        /* Since the mouse is being pressed down, we set our 'is_pressed"
         * boolean indicator to true.
         */
        is_pressed = true;
    }
    /* If the left-mouse button was released up, then...
     */
    else if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    {
        /* Mouse is no longer being pressed, so set our indicator to false.
         */
        is_pressed = false;
	last_rot = cur_rot * last_rot;
	cur_rot = 1;
    }
}

/* 'mouse_moved' function:
 *
 * This function is meant to respond to when the mouse is being moved. There
 * are just two parameters to this function:
 * 
 * - int x: the x screen coordinate of where the mouse was clicked or released
 * - int y: the y screen coordinate of where the mouse was clicked or released
 *
 * We compute our camera rotation angles based on the mouse movement in this
 * function.
 */
void mouse_moved(int x, int y)
{
    /* If the left-mouse button is being clicked down...
     */
    if(is_pressed && !arcball_mode)
    {
        /* You see in the 'mouse_pressed' function that when the left-mouse button
         * is first clicked down, we store the screen coordinates of where the
         * mouse was pressed down in 'mouse_x' and 'mouse_y'. When we move the
         * mouse, its screen coordinates change and are captured by the 'x' and
         * 'y' parameters to the 'mouse_moved' function. We want to compute a change
         * in our camera angle based on the distance that the mouse traveled.
         *
         * We have two distances traveled: a dx equal to 'x' - 'mouse_x' and a
         * dy equal to 'y' - 'mouse_y'. We need to compute the desired changes in
         * the horizontal (x) angle of the camera and the vertical (y) angle of
         * the camera.
         * 
         * Let's start with the horizontal angle change. We first need to convert
         * the dx traveled in screen coordinates to a dx traveled in camera space.
         * The conversion is done using our 'mouse_scale_x' variable, which we
         * set in our 'reshape' function. We then multiply by our 'x_view_step'
         * variable, which is an arbitrary value that determines how "fast" we
         * want the camera angle to change. Higher values for 'x_view_step' cause
         * the camera to move more when we drag the mouse. We had set 'x_view_step'
         * to 90 at the top of this file (where we declared all our variables).
         * 
         * We then add the horizontal change in camera angle to our 'x_view_angle'
         * variable, which keeps track of the cumulative horizontal change in our
         * camera angle. 'x_view_angle' is used in the camera rotations specified
         * in the 'display' function.
         */
        x_view_angle += ((float) x - (float) mouse_x) * mouse_scale_x * x_view_step;
        
        /* We do basically the same process as above to compute the vertical change
         * in camera angle. The only real difference is that we want to keep the
         * camera angle changes realistic, and it is unrealistic for someone in
         * real life to be able to change their vertical "camera angle" more than
         * ~90 degrees (they would have to detach their head and spin it vertically
         * or something...). So we decide to restrict the cumulative vertical angle
         * change between -90 and 90 degrees.
         */
        float temp_y_view_angle = y_view_angle +
                                  ((float) y - (float) mouse_y) * mouse_scale_y * y_view_step;
        // y_view_angle = (temp_y_view_angle > 90 || temp_y_view_angle < -90) ?
        //                y_view_angle : temp_y_view_angle;
	y_view_angle = temp_y_view_angle;
        
        /* We update our 'mouse_x' and 'mouse_y' variables so that if the user moves
         * the mouse again without releasing it, then the distance we compute on the
         * next call to the 'mouse_moved' function will be from this current mouse
         * position.
         */
        mouse_x = x;
        mouse_y = y;
        
        /* Tell OpenGL that it needs to re-render our scene with the new camera
         * angles.
         */
        glutPostRedisplay();
    }

    if (is_pressed && arcball_mode) {
	cur_rot = compute_rotation(x, y, mouse_x, mouse_y, xres, yres);
	glutPostRedisplay();
    }
}

/* 'deg2rad' function:
 * 
 * Converts given angle in degrees to radians.
 */
float deg2rad(float angle)
{
    return angle * M_PI / 180.0;
}

/* 'key_pressed' function:
 * 
 * This function is meant to respond to key pressed on the keyboard. The
 * parameters are:
 *
 * - unsigned char key: the character of the key itself or the ASCII value of
 *                      of the key
 * - int x: the x screen coordinate of where the mouse was when the key was pressed
 * - int y: the y screen coordinate of where the mouse was when the key was pressed
 *
 * Our function is pretty straightforward as you can see below. We also do not make
 * use of the 'x' and 'y' parameters.
 */
void key_pressed(unsigned char key, int x, int y)
{
    /* If 'q' is pressed, quit the program.
     */
    if(key == 'q')
    {
        exit(0);
    }
    /* If 't' is pressed, toggle our 'wireframe_mode' boolean to make OpenGL
     * render our cubes as surfaces of wireframes.
     */
    else if(key == 't')
    {
        wireframe_mode = !wireframe_mode;
        /* Tell OpenGL that it needs to re-render our scene with the cubes
         * now as wireframes (or surfaces if they were wireframes before).
         */
        glutPostRedisplay();
    }
    else if(key == 'r')
    {
	arcball_mode = !arcball_mode;
	glutPostRedisplay();
    }
    else
    {
	arcball_mode = false;
        /* These might look a bit complicated, but all we are really doing is
         * using our current change in the horizontal camera angle (ie. the
         * value of 'x_view_angle') to compute the correct changes in our x and
         * z coordinates in camera space as we move forward, backward, to the left,
         * or to the right.
         *
         * 'step_size' is an arbitrary value to determine how "big" our steps
         * are.
         *
         * We make the x and z coordinate changes to the camera position, since
         * moving forward, backward, etc is basically just shifting our view
         * of the scene.
         */
        
        float x_view_rad = deg2rad(x_view_angle);
        
        /* 'w' for step forward
         */
        if(key == 'w')
        {
            cam_position[0] += step_size * sin(x_view_rad);
            cam_position[2] -= step_size * cos(x_view_rad);
            glutPostRedisplay();
        }
        /* 'a' for step left
         */
        else if(key == 'a')
        {
            cam_position[0] -= step_size * cos(x_view_rad);
            cam_position[2] -= step_size * sin(x_view_rad);
            glutPostRedisplay();
        }
        /* 's' for step backward
         */
        else if(key == 's')
        {
            cam_position[0] -= step_size * sin(x_view_rad);
            cam_position[2] += step_size * cos(x_view_rad);
            glutPostRedisplay();
        }
        /* 'd' for step right
         */
        else if(key == 'd')
        {
            cam_position[0] += step_size * cos(x_view_rad);
            cam_position[2] += step_size * sin(x_view_rad);
            glutPostRedisplay();
        }
    }
}

/* The 'main' function:
 *
 * This function is short, but is basically where everything comes together.
 */
int main(int argc, char* argv[])
{
    if (argc != 4) {
	cout << "Usage: " << argv[1] << " [scenefile.txt] [int xres] [int yres]"\
	     << endl;
	return 1;
    }
    try {
	xres = stoi(argv[2]);
	yres = stoi(argv[3]);
    }
    catch (invalid_argument) {
	cout << "Usage: " << argv[1] << " [scenefile.txt] [int xres] [int yres]"\
	     << endl;
	return 1;
    }
    ifstream scfile;
    map<string, Object> objmap;
    
    /* 'glutInit' intializes the GLUT (Graphics Library Utility Toolkit) library.
     * This is necessary, since a lot of the functions we used above and below
     * are from the GLUT library.
     *
     * 'glutInit' takes the 'main' function arguments as parameters. This is not
     * too important for us, but it is possible to give command line specifications
     * to 'glutInit' by putting them with the 'main' function arguments.
     */
    glutInit(&argc, argv);

    scfile.open(argv[1]);
    if(!(scfile.is_open())) {
	cout << "Filename " << argv[1] << " not found!" << endl;
	return 1;
    }

    // Parsing functions:
    parse_camera(scfile);
    
    lights = parse_lights(scfile);

    objmap = parse_object_names(scfile);

    objects = parse_object_spec(scfile, objmap);

    /* The following line of code tells OpenGL that we need a double buffer,
     * a RGB pixel buffer, and a depth buffer.
     */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    /* The following line tells OpenGL to create a program window of size
     * 'xres' by 'yres'.
     */
    glutInitWindowSize(xres, yres);
    /* The following line tells OpenGL to set the program window in the top-left
     * corner of the computer screen (0, 0).
     */
    glutInitWindowPosition(0, 0);
    /* The following line tells OpenGL to name the program window "Test".
     */
    glutCreateWindow("Test");
    
    /* Call our 'init' function...
     */
    init();
    /* Specify to OpenGL our display function.
     */
    glutDisplayFunc(display);
    /* Specify to OpenGL our reshape function.
     */
    glutReshapeFunc(reshape);
    /* Specify to OpenGL our function for handling mouse presses.
     */
    glutMouseFunc(mouse_pressed);
    /* Specify to OpenGL our function for handling mouse movement.
     */
    glutMotionFunc(mouse_moved);
    /* Specify to OpenGL our function for handling key presses.
     */
    glutKeyboardFunc(key_pressed);
    /* The following line tells OpenGL to start the "event processing loop". This
     * is an infinite loop where OpenGL will continuously use our display, reshape,
     * mouse, and keyboard functions to essentially run our program.
     */
    glutMainLoop();
}
