#define GLFW_DLL

// OpenGL
#include "GL/glew.h"
#include <GLFW/glfw3.h>

// C
#include <cstdlib>
#include <cstdio>
#include <cmath>

// C++
#include <iostream>
#include <list>

// Custom
#include "math/mat3.h"
#include "math/mat4.h"
#include "renderer/RenderObject.h"
#include "generators/Simplex.h"


#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

// Position Data
const Vec3 Camera(0, 1, 1);
const Vec3 Origin(0, 0, 0);
const Vec3 Up(0, 1, 0);
Mat4 TranslateMatrix = Mat4::LookAt(Camera, Origin, Up);

// The window and related data
static GLFWwindow *window;
static int height = 480, width = 640;

#define checkGL() {							\
	GLenum err = glGetError();				\
	if (err != GL_NO_ERROR) {				\
		std::cout << err << ": " <<			\
			gluErrorString(err) <<			\
			" at line " << __LINE__ <<		\
			" in file " << __FILE__ <<		\
			std::endl;						\
		std::exit(-1);						\
	}										\
}
GLenum MODES[2] = {GL_FILL , GL_LINE };
int mode = 0;

//////////////////////////////////////////////
//			Simple Random Data				//
//////////////////////////////////////////////
float* generateGround(float min_x, float max_x, float min_z, float max_z, int div) {
	
	init_simplex(242342);

	float x_len = max_x - min_x;
	float z_len = max_z - min_z;
	float delta_x = x_len / (div - 1);
	float delta_z = z_len / (div - 1);
	float* data = (float*)calloc(3 * div * div, sizeof(float));
	
	for ( int i = 0; i < div; i++) { // z
		float z = max_z - i * delta_z;
		for (int j = 0 ; j < div; j++) { // x
			float x = min_x + j * delta_x;
			int pos = 3 * i * div + 3 * j;
			data[pos] = x;
			//data[pos + 1] = (float)simplex2d( x , z ,7,2.323f);
			data[pos + 1] = 50 * (rand() / float(RAND_MAX));
			data[pos + 2] = z;
		}
	}
	return data;
}

GLuint* generateIndices(int div) {
	GLuint* data = (GLuint*)calloc( (div - 1) * (div - 1) * 6, sizeof(GLuint));
	int max_i, max_j;
	max_i = max_j = div - 1;

	int A = 0;
	int B = 1;
	int C = div;
	int D = div + 1;

	int i = 0;
	for ( ; i < max_i; i++){
		int j = 0;
		for ( ; j < max_j; j++) {
			int pos = 6 * ( i * (div - 1) + j);

			// Create indices
			// Triangle One
			data[pos] = A;			// 0
			data[pos + 1] = B;		// 1
			data[pos + 2] = D;		// 3

			// Triangle Two
			data[pos + 3] = A;		// 0
			data[pos + 4] = D;		// 3
			data[pos + 5] = C;		// 2
		
			A++;
			B++;
			C++;
			D++;
		}
		A++;
		B++;
		C++;
		D++;
	}
	return data;
}

//////////////////////////////////////////////
//				Callbacks					//
//////////////////////////////////////////////
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	float scale = 1.0;
	if (mods == GLFW_MOD_SHIFT) {
		scale = 10.0;
	}
	if (action == GLFW_REPEAT || action == GLFW_PRESS){
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		case GLFW_KEY_UP:
			TranslateMatrix.rotateX(0.0174532925f * (scale * 0.2f));
			break;
		case GLFW_KEY_DOWN:
			TranslateMatrix.rotateX(0.0174532925f * (scale * -0.2f));
			break;
		case GLFW_KEY_LEFT:
			TranslateMatrix.rotateY(0.0174532925f * (scale * 0.2f));
			break;
		case GLFW_KEY_RIGHT:
			TranslateMatrix.rotateY(0.0174532925f * (scale * -0.2f));
			break;
		case GLFW_KEY_W:
			TranslateMatrix.moveZ(scale * 5.0f);
			break;
		case GLFW_KEY_S:
			TranslateMatrix.moveZ(scale * -5.0f);
			break;
		case GLFW_KEY_D:
			TranslateMatrix.moveX(scale * -0.5f);
			break;
		case GLFW_KEY_A:
			TranslateMatrix.moveX(scale * 0.5f);
			// move right
			break;
		case GLFW_KEY_P:
			mode = (mode + 1) % 2;
			glPolygonMode(GL_FRONT_AND_BACK, MODES[mode]);
		}
	}
}

static void mousepos_callback(GLFWwindow *window, double x, double y) {
	TranslateMatrix.rotateX(-10.0f * 0.0174532925f * (float)y / height);
	TranslateMatrix.rotateY(-10.0f * 0.0174532925f * (float)x / width);
	// Reset to the center
	glfwSetCursorPos(window, 0, 0);
}

void resized_callback(GLFWwindow *window, int w, int h) {
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

//////////////////////////////////////////////
//				Helper Functions			//
//////////////////////////////////////////////
// Initializes all the subsystems, create the window.
void init() {

	// Set up the error callback for GLFW
	glfwSetErrorCallback(error_callback);

	// Initialize the GLFW system, if failure jump ship
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	// Set the hints on how to render to the 
	// screen, which OpenGL version we have, etc
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	
	// Create our window using the hints given above
	window = glfwCreateWindow(width, height, "Terrain Generator", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}glfwMakeContextCurrent(window);

	// Set callbacks for GLFW, window, keyboard, and mouse
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, resized_callback);
	glfwSetCursorPosCallback(window, mousepos_callback);
	
	// Disable the mouse so that we can lookaround
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Setup GLEW, enable newer OpenGL functions
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		glfwTerminate();
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		exit(EXIT_FAILURE);
	}glGetError();
	
	// The background color should be a 
	// nice light blue color
	glClearColor(0.5f, 0.5f, 1.0f, 0.0f);
}

// Just prints OpenGL information
void print() {
	char current[FILENAME_MAX];
	if (GetCurrentDir(current, sizeof(current))){
		printf("Current Working Directory : %s\n" , current );
	}
	printf("OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
	printf("OpenGL Vendor: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL Vendor: %s\n", glGetString(GL_VERSION));
}

//////////////////////////////////////////////
//					Main					//
//////////////////////////////////////////////
int main(int argc, char** args)
{
	init();print();
        TranslateMatrix.moveZ(-1000.0f);


	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Load the shader and compile it
	const std::string BaseShaderDir = std::string("../resources/shaders/");
	Shader shader(BaseShaderDir,std::string("shader"));

	if (shader.getError() == SHADER_ERROR::NO_SHADER_ERROR) {
		printf("No error loading shader\n");
	}else {
		printf("Error with shader\n");
	}
	
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	const Mat4 ProjectionMatrix = Mat4::Perspective(45.0f, (float)height / (float)width, -0.1f, 100.0f);
	
	const int divisions = 50;
	const int number_vertices = 3 * divisions * divisions;
	const int number_indicies = 6 * (divisions-1) * (divisions - 1);
	const float size = 100.0f;
	GLfloat *ground_data = generateGround(-size, size, -size, size, divisions);
	GLuint *indices = generateIndices(divisions);

	std::list<RenderObject> objs;
	objs.insert(
		objs.end(),
		RenderObject(shader, ground_data, number_vertices, indices, number_indicies)
	);

	// Main Loop.  Do the stuff!
	while (!glfwWindowShouldClose(window)) {
		// Clear everything on the screen
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		
		// for objects to be rendered
		for (RenderObject obj : objs) {
			obj.render(ProjectionMatrix, TranslateMatrix, Camera);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}
