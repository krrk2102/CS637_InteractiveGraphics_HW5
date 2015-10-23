// HW5 for CS 637
// Shangqi Wu

#include "Angel.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

typedef vec4 color4;

// Height and width of main window. 
const int h = 500;
const int w = 500;

// True for perspective projection, false for parallel projection. 
bool perspective = true;

// Numbers of points triangles. 
const int NumPoints = 3;

// RGBA color for background of main window.
float Red = 0;
float Green = 0;
float Blue = 0;
float Alpha = 1;

// Radius of camera rotation and its delta.
float cameraRadius = 1;
float dr = 0.05;
// Height of camera and its delta.
float cameraHeight = 0;
float dh = 0.05;
// Current position and its delta of a circling camera
float t = 0;
float dt = 0.01;

// Initial position of look-at, camera position, and up vector for projection.
vec4 at( 0, 0, 0, 1 );
vec4 eye( 0, 0, 0, 1 );
vec4 up( 0, 1, 0, 1 );

// Position and color of light source 1.
vec4 light1_pos( 1, 1, 1, 1 );
color4 light1_diffuse( 0, 1, 1, 1 );
// Position and color of light source 2.
vec4 light2_pos( -1, 1, -1, 1 );
color4 light2_diffuse( 1, 1, 0, 1 );
// Color of object. 
color4 material_diffuse( 1, 1, 1, 1 );

// IDs for main window. 
int MainWindow;

// Vector for vertices.
vector<vec4> original_vertices;
// Vector containing vertices of every triangle to be drawn.
vector<vec4> vertices;
// Vector for normal vectors of every point in triangles. 
vector<vec4> normals;

// ID for shaders program.
GLuint program;

//--------------------------------------------------------------------------

vec4
product( const vec4 &a, const vec4 &b )
{
    return vec4( a[0]*b[0], a[1]*b[1], a[2]*b[2], a[3]*b[3] );
}

//--------------------------------------------------------------------------

void
init( void )
{
    
    // Create a vertex array object.
    GLuint vao[1];
    glGenVertexArrays( 1, vao );
    glBindVertexArray( vao[0] );
    cout<<"glGenVertexArrays(), glBindVertexArray() for main window initialization."<<endl;

    // Create and initialize a buffer object.
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    cout<<"glGenBuffer(), glBindBuffer() for main window initialization."<<endl;
    glBufferData( GL_ARRAY_BUFFER, vertices.size()*sizeof(vec4)+normals.size()*sizeof(vec4), NULL, GL_STATIC_DRAW );
    // Pass vertices & normals data to opengl buffer object.
    glBufferSubData( GL_ARRAY_BUFFER, 0, vertices.size()*sizeof(vec4), vertices.data() );
    glBufferSubData( GL_ARRAY_BUFFER, vertices.size()*sizeof(vec4), normals.size()*sizeof(vec4), normals.data() );
    cout<<"glBufferData(), glBufferSubData() for main window initialization."<<endl;

    // Load shaders and use the resulting shader program.
    program = InitShader( "vshader21.glsl", "fshader21.glsl" );
    glUseProgram( program );
    cout<<"InitShader(), glUseProgram() for main window initialization."<<endl;

    // Initialize the vertex position attribute from the vertex shader.
    GLuint loc_ver = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( loc_ver );
    glVertexAttribPointer( loc_ver, 4, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET(0) );

    // Pass normal vectors of each triangle to vertex shader
    GLuint loc_col = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( loc_col );
    glVertexAttribPointer( loc_col, 4, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET( vertices.size()*sizeof(vec4) ) );
    cout<<"glEnableVertexAttribArray(), glVertexAttribPointer() for main window initialization."<<endl;
    
    // Pass positions of light sources to vertex shader.
    GLint loc_light1_pos = glGetUniformLocation( program, "light1_pos" );
    GLint loc_light2_pos = glGetUniformLocation( program, "light2_pos" );
    glUniform4f( loc_light1_pos, light1_pos.x, light1_pos.y, light1_pos.z, light1_pos.w );
    glUniform4f( loc_light2_pos, light2_pos.x, light2_pos.y, light2_pos.z, light2_pos.w );

    // Calculate and pass color products of each light source to vertex shader.
    vec4 d_pro1 = product( light1_diffuse, material_diffuse );
    vec4 d_pro2 = product( light2_diffuse, material_diffuse );
    GLint loc_diffuse_product1 = glGetUniformLocation( program, "light1_diffuse_product" );
    GLint loc_diffuse_product2 = glGetUniformLocation( program, "light2_diffuse_product" );
    glUniform4f( loc_diffuse_product1, d_pro1.x, d_pro1.y, d_pro1.z, d_pro1.w );
    glUniform4f( loc_diffuse_product2, d_pro2.x, d_pro2.y, d_pro2.z, d_pro2.w );
    cout<<"glGetUniformLocation(), glUniform4f() for main window initialization."<<endl;
}

//----------------------------------------------------------------------------

void
recal( void )
{
    // Calculate renewed camera position. 
    eye = vec4( cameraRadius*sin(t), cameraHeight, cameraRadius*cos(t), 1 );

    // Create model and projection matrix.
    mat4 modelview = identity();
    mat4 projection = identity();

    if (perspective) projection *= Perspective( 90, 1, 0.000001, 1 );
    else projection *= Ortho( -1, 1, -1, 1, -100, 100 );

    modelview *= LookAt( eye, at, up );
    
    // Pass model and projection matrix to vertex shader. 
    GLint loc_modelview = glGetUniformLocation( program, "modelview" );
    glUniformMatrix4fv( loc_modelview, 1, GL_TRUE, modelview );
    GLint loc_projection = glGetUniformLocation( program, "projection" );
    glUniformMatrix4fv( loc_projection, 1, GL_TRUE, projection );
    cout<<"glGetUniformLocation(), glUniformMatrix4fv() for transformation matrix."<<endl;
}

//----------------------------------------------------------------------------

void
display( void )
{
    recal(); // Calculates vertices & colors for objects in main window. 
    glClearColor( Red, Green, Blue, Alpha ); // Set background color of main window.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // Clear main window.
    glMatrixMode( GL_MODELVIEW ); // Set up model view.
    glDrawArrays( GL_TRIANGLES, 0, vertices.size() ); // Draw the points by one triangle.
    glutSwapBuffers(); // Double buffer swapping. 
    glFlush(); // Flush. 
    cout<<"glClearColor(), glClear(), glDrawArrays(), glutSwapBuffers(), glFlush() for main window display function."<<endl;
}

//----------------------------------------------------------------------------

void
cameraRotation( void )
{
    t += dt;
    glutPostRedisplay();
    cout<<"glutPostRedislpay() for idle function."<<endl;
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
        case 033: exit( EXIT_SUCCESS ); break; // "Esc": exit the program.
        case (int)'w': cameraHeight += dh; break; // Increasing camera height.
        case (int)'s': cameraHeight -= dh; break; // Decreasing camera height.
        case (int)'a': cameraRadius += dr; break;// Incresing camera radius.
        case (int)'d': if (cameraRadius > dr) cameraRadius -= dr; break; // Decreasing camera radius.
        case (int)'e': dt += 0.0025; break; // Double camera rotation speed.
        case (int)'q': dt -= 0.0025; break; // Half camera rotation speed.
    }
    glutPostRedisplay();
    cout<<"glutPostRedisplay() for keyboard function."<<endl;
}

//----------------------------------------------------------------------------

void 
MainSubMenuRotation( int id )
{
    switch ( id ) {
        case 1: glutIdleFunc( cameraRotation ); break; // Start animation.
        case 2: glutIdleFunc( NULL ); break; // Stop animation.
    }
    glutPostRedisplay();
    cout<<"glutPostRedisplay() for idle rotation."<<endl;
}

//----------------------------------------------------------------------------

void 
MainSubMenuPerspective( int id )
{
    switch ( id ) {
        case 1: perspective = true; break; // Switch to persepctive projection.
        case 2: perspective = false; break; // Switch to parallel projection.
    }
    glutPostRedisplay();
    cout<<"glutPostRedisplay() for projection mode changing."<<endl;
}

//----------------------------------------------------------------------------

void
setMainWinMenu( void )
{
    int menu_id, submenu_id_r, submenu_id_p;
    // Create submenu for rotating animation.
    submenu_id_r = glutCreateMenu( MainSubMenuRotation );
    glutAddMenuEntry( "Start Camera Rotation", 1 );
    glutAddMenuEntry( "Stop Camera Rotation", 2 );

    // Create submenu for projection changing.
    submenu_id_p = glutCreateMenu( MainSubMenuPerspective );
    glutAddMenuEntry( "Perspective Projection", 1 );
    glutAddMenuEntry( "Parallel Projection", 2 );

    menu_id = glutCreateMenu( NULL ); // Set menu in main window. 
    cout<<"glutCreateMenu() for main window menu."<<endl;
    glutAddSubMenu( "Camera Rotation", submenu_id_r );
    glutAddSubMenu( "Projection", submenu_id_p );
    cout<<"glutAddMenuEntry() for main window menu."<<endl;
    glutAttachMenu( GLUT_RIGHT_BUTTON );
    cout<<"glutAttachMenu() for main window menu."<<endl;
}

//----------------------------------------------------------------------------

void 
readfile( vector<vec4> &vecver, vector<vec4> &vecf, vector<vec4> &vecn )
{
    // Start to read file.
    ifstream infile;
    do {
        string input;
        cout<<"Please enter your input smf file name:"<<endl;
        cin>>input;
        infile.open(input.c_str());
        if (!infile) cout<<"Your input file path is incorrect."<<endl;
    } while (!infile);
    bool storev = false;
    bool storef = false;
    string str;
    vector<double> buff;
    // Read file content.
    while (infile) {
        infile>>str;
        if (str.compare("v") == 0) {
            storev = true;
        }else if (str.compare("f") == 0){
            storef = true;
        }else if (storev == true){ // Add vertex to its vector.
            // Store a vertice.
            buff.push_back(atof(str.c_str()));
            if (buff.size() == 3) {
                vec4 ver( buff[0], buff[1], buff[2], 1 );
                vecver.push_back(ver);
                storev = false;
                buff.clear();
            }
        }else if (storef == true){ // Add face to its vector.
            buff.push_back(atoi(str.c_str()));
            // Store vertices for a triangle and calculate its normal vector.
            if (buff.size() == 3) {
                vecf.push_back( vecver[(int)buff[0]-1] );
                vecf.push_back( vecver[(int)buff[1]-1] );
                vecf.push_back( vecver[(int)buff[2]-1] );
                vecn.push_back( normalize( vec4( cross( (vecver[(int)buff[1]-1]-vecver[(int)buff[0]-1]), (vecver[(int)buff[2]-1]-vecver[(int)buff[0]-1]) ) ) ) );
                vecn.push_back( vecn.back() );
                vecn.push_back( vecn.back() );
                storef = false;
                buff.clear();
            }
        }
    }
    infile.close();
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{   
    readfile( original_vertices, vertices, normals ); // Read input smf file.

    glutInit( &argc, argv ); // Initializing environment.
    cout<<"glutInit(&argc,argv) called."<<endl;
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH ); // Enable depth.
    cout<<"glutInitDisplayMode() called."<<endl;
    glutInitWindowSize( w, h );
    cout<<"glutInitWindowSize() called."<<endl;
    glutInitWindowPosition( 100, 100 );
    cout<<"glutInitWindowPosition() called."<<endl;

    MainWindow = glutCreateWindow( "ICG_hw5" ); // Initializing & setting main window.
    cout<<"glutCreateWindow() for main window."<<endl;
    glewExperimental=GL_TRUE; 
    glewInit();    
    cout<<"glewInit() for main window."<<endl;
    init(); // Initializing VAOs & VBOs. 
    glutDisplayFunc( display ); // Setting display function for main window.
    cout<<"glutDisplayFunc() for main window."<<endl;
    glutKeyboardFunc( keyboard ); // Setting keyboard function for main window.
    cout<<"glutKeyboardFunc() for main window."<<endl;
    setMainWinMenu(); // Setting menu for main window. 
    glutIdleFunc( cameraRotation ); // Start animation by default.
    cout<<"glutIdleFunc() for main window."<<endl;

    glEnable( GL_DEPTH_TEST );
    cout<<"glEnable( GL_DEPTH_TEST ) called."<<endl;
    cout<<"glutMainLoop() called."<<endl;
    
    glutMainLoop(); // Start main loop. 
    return 0;
}
