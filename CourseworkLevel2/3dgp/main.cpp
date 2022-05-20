#include <iostream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

#include <array>
#include "Texture.h"

using namespace std;
using namespace _3dgl;
using namespace glm;

// GLSL Program
C3dglProgram Program;

// 3D Models
C3dglTerrain terrain, road;
C3dglModel lamp, heart;

// Textures
Texture heart_tex;
Texture heart_nm; // Normal map for the heart texture

GLuint null_texture; // Null texture. Used to turn off texturing

// camera position (for first person type camera navigation)
mat4 matrixView;			// The View Matrix
float angleTilt = 15.f;		// Tilt Angle
vec3 cam(0);				// Camera movement values

bool loadTextures() {
	// Heart texture
	heart_tex.tex.Load("models/fabric1.png", GL_RGBA);
	if (!heart_tex.tex.GetBits()) return false;
	CreateTexture(heart_tex, GL_TEXTURE0);

	// Heart normal map
	heart_nm.tex.Load("models/normal5.jpg", GL_RGBA);
	if (!heart_nm.tex.GetBits()) return false;
	CreateTexture(heart_nm, GL_TEXTURE1);
	Program.SendUniform("textureNormal", 1);

	// Null texture
	glGenTextures(1, &null_texture);
	glBindTexture(GL_TEXTURE_2D, null_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	std::array<BYTE, 3> bytes = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, bytes.data());
}

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	// Basic shaders, used for lighting and fog
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/basic.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/basic.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	// Create the program
	if (!Program.Create()) return false;
	if (!Program.Attach(VertexShader)) return false;
	if (!Program.Attach(FragmentShader)) return false;
	if (!Program.Link()) return false;
	if (!Program.Use(true)) return false;

	glutSetVertexAttribCoord3(Program.GetAttribLocation("aVertex"));
	glutSetVertexAttribNormal(Program.GetAttribLocation("aNormal"));

	// load your 3D models here!
	if (!terrain.loadHeightmap("models\\heightmap.bmp", 10)) return false;
	if (!road.loadHeightmap("models\\roadmap.bmp", 10)) return false;
	if (!lamp.load("models\\street lamp - fancy.obj")) return false;
	if (!heart.load("models\\heart.obj")) return false;

	// Load the textures
	if (!loadTextures()) return false;

	// Initialise light variables
	Program.SendUniform("lightAmbient.color", 0.05, 0.05, 0.05);
	Program.SendUniform("lightDir.direction", 0.1, 1.0, 0.1);
	Program.SendUniform("lightDir.diffuse", 0.1, 0.1, 0.1);

	// Initialise fog variables
	Program.SendUniform("fogDensity", 0.1);
	Program.SendUniform("fogColor", 0.3, 0.3, 0.3);

	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));
	matrixView *= lookAt(
		vec3(4.0, 1.5, 30.0),
		vec3(4.0, 1.5, 0.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky background

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

void done()
{
}

void renderScene(mat4& matrixView, float time)
{
	mat4 m;

	// Send time variable for texture animation
	Program.SendUniform("time", time);

	// Initially, textures will be non-animated, so send 0 for speedX and speedY
	Program.SendUniform("speedX", 0.0f);
	Program.SendUniform("speedY", 0.0f);

	// setup materials - green (grass)
	Program.SendUniform("materialAmbient", 0.2f, 0.8f, 0.2f);
	Program.SendUniform("materialDiffuse", 0.2f, 0.8f, 0.2f);
	Program.SendUniform("materialSpecular", 1.0f, 1.0f, 1.0f);
	Program.SendUniform("shininess", 3.0);

	glBindTexture(GL_TEXTURE_2D, null_texture);

	// render the terrain
	m = translate(matrixView, vec3(0, 0, 0));
	terrain.render(m);

	// setup materials - grey (road)
	Program.SendUniform("materialAmbient", 0.3f, 0.3f, 0.16f);
	Program.SendUniform("materialDiffuse", 0.3, 0.3, 0.16);
	Program.SendUniform("materialSpecular", 1.0f, 1.0f, 1.0f);
	Program.SendUniform("shininess", 3.0);

	// render the road
	m = translate(matrixView, vec3(0, 0, 0));
	m = translate(m, vec3(6.0f, 0.01f, 0.0f));
	road.render(m);

	// Render the heart
	m = matrixView;
	m = translate(m, vec3(-5.0, 5.0, 0.0));
	m = rotate(m, radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.3, 0.3, 0.3));
	Program.SendUniform("lightRim.diffuse", 1.0, 1.0, 1.0);
	glActiveTexture(heart_tex.id);
	glBindTexture(GL_TEXTURE_2D, heart_tex.id);
	glActiveTexture(heart_nm.id);
	glBindTexture(GL_TEXTURE_2D, heart_nm.id);
	Program.SendUniform("textureNormal", 1);
	
	// Animate the heart
	Program.SendUniform("speedY", 1.0f); // Make the texture move vertically

	heart.render(m);

	// After rendering the heart, stop texture animation
	Program.SendUniform("speedY", 0.0f);

	Program.SendUniform("lightRim.diffuse", 0.0, 0.0, 0.0); // turn off rim lighting
	glBindTexture(GL_TEXTURE_2D, null_texture); // turn off texturing

	// Render a lamp
	// Point lights
	m = matrixView;
	m = translate(m, vec3(4.0, 3.5, 20.0));
	m = scale(m, vec3(0.02, 0.02, 0.02));
	Program.SendUniform("lightPoint1.position", 4.0, 3.5, 20.0);
	Program.SendUniform("lightPoint1.specular", 1.0, 0.0, 0.0);
	lamp.render(m);

	m = matrixView;
	m = translate(m, vec3(7.0, 3.5, 0.0));
	m = scale(m, vec3(0.02, 0.02, 0.02));
	Program.SendUniform("lightPoint2.position", 7.0, 3.5, 0.0);
	Program.SendUniform("lightPoint2.specular", 0.0, 1.0, 0.0);
	lamp.render(m);

	// Spot light
	m = matrixView;
	m = translate(m, vec3(-15.0, 7.5, 20.0));
	m = scale(m, vec3(0.02, 0.02, 0.02));
	Program.SendUniform("lightSpot.position", -15.0, 7.5, 20.0);
	Program.SendUniform("lightSpot.specular", 1.0, 1.0, 0.0);
	Program.SendUniform("lightSpot.direction", 0.0, -1.0, 0.0);
	Program.SendUniform("lightSpot.cutoff", radians(60.0f));
	Program.SendUniform("lightSpot.attenuation", 4);
	lamp.render(m);

	// Attenuated light
	m = matrixView;
	m = translate(m, vec3(13.0, 5.0, -15.0));
	m = scale(m, vec3(0.02, 0.02, 0.02));
	Program.SendUniform("lightAtt.position", 13.0, 5.0, -15.0);
	Program.SendUniform("lightAtt.specular", 0.0, 0.0, 1.0);
	Program.SendUniform("lightAtt.att_const", 0.1);
	Program.SendUniform("lightAtt.att_linear", 0.1);
	Program.SendUniform("lightAtt.att_quadratic", 0.1);
	lamp.render(m);
}

void onRender()
{
	// this global variable controls the animation
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
	m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
	m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));			// switch tilt on
	matrixView = m * matrixView;
	Program.SendUniform("matrixView", matrixView);

	// move the camera up following the profile of terrain (Y coordinate of the terrain)
	float terrainY = -terrain.getInterpolatedHeight(inverse(matrixView)[3][0], inverse(matrixView)[3][2]);
	matrixView = translate(matrixView, vec3(0, terrainY, 0));

	// render the scene objects
	renderScene(matrixView, time);

	// the camera must be moved down by terrainY to avoid unwanted effects
	matrixView = translate(matrixView, vec3(0, -terrainY, 0));

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(60.f), ratio, 0.02f, 1000.f);

	// Setup the Projection Matrix
	Program.SendUniform("matrixProjection", matrixProjection);
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': cam.z = std::max(cam.z * 3.05f, 0.1f); break;
	case 's': cam.z = std::min(cam.z * 3.05f, -0.1f); break;
	case 'a': cam.x = std::max(cam.x * 3.05f, 0.1f); break;
	case 'd': cam.x = std::min(cam.x * 3.05f, -0.1f); break;
	case 'e': cam.y = std::max(cam.y * 3.05f, 0.1f); break;
	case 'q': cam.y = std::min(cam.y * 3.05f, -0.1f); break;
	}
	// speed limit
	cam.x = std::max(-0.15f, std::min(0.15f, cam.x));
	cam.y = std::max(-0.15f, std::min(0.15f, cam.y));
	cam.z = std::max(-0.15f, std::min(0.15f, cam.z));
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': cam.z = 0; break;
	case 'a':
	case 'd': cam.x = 0; break;
	case 'q':
	case 'e': cam.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
bool bJustClicked = false;
void onMouse(int button, int state, int x, int y)
{
	bJustClicked = (state == GLUT_DOWN);
	glutSetCursor(bJustClicked ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
}

// handle mouse move
void onMotion(int x, int y)
{
	if (bJustClicked)
		bJustClicked = false;
	else
	{
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// find delta (change to) pan & tilt
		float deltaPan = 0.25f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
		float deltaTilt = 0.25f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// View = Tilt * DeltaPan * Tilt^-1 * DeltaTilt * View;
		angleTilt += deltaTilt;
		mat4 m = mat4(1.f);
		m = rotate(m, radians(angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaPan), vec3(0.f, 1.f, 0.f));
		m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaTilt), vec3(1.f, 0.f, 0.f));
		matrixView = m * matrixView;
	}
}

int main(int argc, char** argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("CI5520 3D Graphics Programming");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// register callbacks
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	done();

	return 1;
}

