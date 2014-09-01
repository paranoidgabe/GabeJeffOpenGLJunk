#define GLFW_DLL

// OpenGL
#include "GL/glew.h"	// glViewPort...
#include <GLFW/glfw3.h>	// glfw*

// C
#include <cstdlib>	// EXIT_SUCCESS, EXIT_FAILURE, std::exit
#include <cstdio>	// TODO: Is this needed?
#include <cmath>	// sin, cos, ...

// C++
#include <iostream>	// std::cout
#include <list>		// List<RenderObject>
#include <string>	// std::string
// Custom
#include "math/mat4.h"
#include "math/vec2.h"
#include "renderer/RenderObject.h"
#include "generators/Simplex.h"

// Used to get the current directory, can use later for something?
#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

// Position Data
Vec3 Camera(4.50184f, 120.615f, -138.001f);

double horizontalAngle = 87.45;
double verticalAngle = -1.0;
double initialiFOV = 45.0;
float initial_speed = 2.5f;
float mouseSpeed = 0.0005f;

// The window and related data
static GLFWwindow *window;
static float height = 768, width = 1024;

const float znear = 1.0;
const float zfar = -1.0f;

Mat4 TranslateMatrix;
Mat4 ProjectionMatrix = Mat4::Perspective(90.0f, (float)width / (float)height, znear, zfar);


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
GLenum MODES[2] = { GL_FILL, GL_LINE };
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

	for (int i = 0; i < div; i++) { // z
		float z = max_z - i * delta_z;
		for (int j = 0; j < div; j++) { // x
			float x = min_x + j * delta_x;
			int pos = 3 * i * div + 3 * j;
			data[pos] = x;
			data[pos + 1] = (float)simplex2d( x , z ,7,2.323f)/10;
			//data[pos + 1] = 50 * (rand() / float(RAND_MAX));
			data[pos + 2] = z;
		}
	}
	return data;
}

GLuint* generateIndices(int div) {
	GLuint* data = (GLuint*)calloc((div - 1) * (div - 1) * 6, sizeof(GLuint));
	int max_i, max_j;
	max_i = max_j = div - 1;

	int A = 0;
	int B = 1;
	int C = div;
	int D = div + 1;

	int i = 0;
	for (; i < max_i; i++){
		int j = 0;
		for (; j < max_j; j++) {
			int pos = 6 * (i * (div - 1) + j);

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

bool shift_down = false;

bool handleKey(int key, int check_key) {
	if (key == check_key) return true;
	return glfwGetKey(window, check_key) != GLFW_RELEASE;
}

void update() {

	float speed = initial_speed;
	if (shift_down) {
		speed = 3.5;
	}

	double xpos,ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	glfwSetCursorPos(window, width / 2, height / 2);

	horizontalAngle += mouseSpeed * (width * 0.5f - xpos);
	verticalAngle	+= mouseSpeed *float(height * 0.5f - ypos);

	Vec3 direction(
		float(cos(verticalAngle) * sin(horizontalAngle)),
		(float)sin(verticalAngle),
		(float)(cos(verticalAngle) * cos(horizontalAngle))
	);

	Vec3 right(
		float(sin(horizontalAngle - 1.570796325f)),
		0.0f,
		float(cos(horizontalAngle - 1.570796325f))
	);

	Vec3 up = right.cross(direction);

	if (glfwGetKey(window, GLFW_KEY_W) != GLFW_RELEASE) {
		Camera += direction * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) != GLFW_RELEASE) {
		Camera -= direction * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) != GLFW_RELEASE) {
		Camera += right * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) != GLFW_RELEASE) {
		Camera -= right * speed;
	}

	TranslateMatrix = Mat4::LookAt(Camera, Camera + direction, up);

}

//////////////////////////////////////////////
//				Callbacks					//
//////////////////////////////////////////////
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	shift_down = mods == GLFW_MOD_SHIFT;

	if (action == GLFW_REPEAT || action == GLFW_PRESS){

		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_P) {
			mode = (mode + 1) % 2;
			glPolygonMode(GL_FRONT_AND_BACK, MODES[mode]);
		}
	}
}

void resized_callback(GLFWwindow *window, int w, int h) {
	width = float(w);
	height = float(h);
	glViewport(0, 0, w, h);

	ProjectionMatrix = Mat4::Perspective(45.0f, width / height, znear, zfar);

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
	window = glfwCreateWindow((int)width, (int)height, "Terrain Generator", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}glfwMakeContextCurrent(window);

	// Set callbacks for GLFW, window, keyboard, and mouse
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, resized_callback);

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
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

// Just prints OpenGL information
void print() {
	char current[FILENAME_MAX];
	if (GetCurrentDir(current, sizeof(current))){
		printf("Current Working Directory : %s\n", current);
	}
	printf("OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
	printf("OpenGL Vendor: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL Vendor: %s\n", glGetString(GL_VERSION));
}

float area(const Vec3& A, const Vec3& B, const Vec3& C) {
	Vec3 AB = A - B;
	Vec3 AC = A - C;
	float theta = acos(AB * AC);
	return float(0.5f * AB.getMagnitude() * AC.getMagnitude() * sin(theta));
}

float interpolate(float min, float max, float alpha) {
	return min * (1.0 - alpha) + max * alpha;
}

float getHeight(RenderObject &ground) {
	
	const GLfloat *data = ground.getRawData();

	float x = Camera.getX();
	float z = Camera.getZ();

	int divisions = (int)sqrt(ground.getNumberVertices() / 3);
	float del = abs(2 * data[0]) / divisions;

	float x_index = (x - data[0]) / del;
	float z_index = (z - data[0]) / del;

	if (x_index < divisions && z_index < divisions ) {

		int x_idx = int(x_index);
		int z_idx = int(z_index);

		float low_x = x_index - x_idx;
		float low_z = z_index - z_idx;

		if ( low_x < low_z ) {			// Upper Triangle
			Vec3 upper_left(&data[3 * x_idx*divisions + 3 * z_idx]);
			Vec3 upper_right(&data[3 * x_idx*divisions + 3 * z_idx + 3]);
			Vec3 bottom_left(&data[3 * x_idx*divisions + 3 * z_idx + 3 * divisions]);

			float segment_len = (Vec2(upper_right.getX(), upper_right.getZ())
				- Vec2(x_index, z_index)).getMagnitude();
			float total_len = (Vec2(upper_right.getX(), upper_right.getZ())
				- Vec2(bottom_left.getX(), bottom_left.getZ())).getMagnitude();

			float u_y = interpolate(upper_left.getY(), upper_right.getY(), low_x);
			float l_y = interpolate(upper_left.getY(), bottom_left.getY(), low_x);
			float d_y = interpolate(upper_right.getY(), bottom_left.getY(), segment_len / total_len);

			return (u_y + l_y + d_y) / 3.0;

		} else if (low_x < low_z) {		// Lower Triangle

			Vec3 upper_right(&data[3 * x_idx*divisions + 3 * z_idx + 3]);
			Vec3 bottom_left(&data[3 * x_idx*divisions + 3 * z_idx + 3 * divisions]);
			Vec3 bottom_right(&data[3 * x_idx*divisions + 3 * z_idx + 3 * divisions + 3]);

			float segment_len = (Vec2(upper_right.getX(), upper_right.getZ())
				- Vec2(x_index, z_index)).getMagnitude();
			float total_len = (Vec2(upper_right.getX(), upper_right.getZ())
				- Vec2(bottom_left.getX(), bottom_left.getZ())).getMagnitude();

			float r_y = (1.0f - low_x) * upper_right.getY() + low_x * bottom_right.getY();
			float b_y = (1.0f - low_z) * bottom_left.getY() + low_z * bottom_right.getY();
			float d_y = interpolate(upper_right.getY(), bottom_left.getY(), segment_len / total_len);
			return (d_y + r_y + b_y) / 3.0;

		} else {						// On the line
			Vec3 upper_right(&data[3 * x_idx*divisions + 3 * z_idx + 3]);
			Vec3 bottom_left(&data[3 * x_idx*divisions + 3 * z_idx + 3 * divisions]);

			float segment_len = (Vec2(upper_right.getX(), upper_right.getZ())
				- Vec2(x_index, z_index)).getMagnitude();
			float total_len = (Vec2(upper_right.getX(), upper_right.getZ())
				- Vec2(bottom_left.getX(), bottom_left.getZ())).getMagnitude();

			return interpolate(upper_right.getY(), bottom_left.getY(), segment_len / total_len);
		}
	}

	return 0.0;
}

//////////////////////////////////////////////
//					Main					//
//////////////////////////////////////////////
int main(int argc, char** args)
{
	init(); print();
	
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Load the shader and compile it
	const std::string BaseShaderDir = std::string("../resources/shaders/");
	Shader shader(BaseShaderDir, std::string("shader"));

	if (shader.getError() == SHADER_ERROR::NO_SHADER_ERROR) {
		printf("No error loading shader\n");
	}
	else {
		printf("Error with shader\n");
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);


	const int divisions = 100;
	const int number_vertices = 3 * divisions * divisions;
	const int number_indicies = 6 * (divisions - 1) * (divisions - 1);
	const float size = 1000.0f;
	GLfloat *ground_data = generateGround(-size, size, -size, size, divisions);
	GLuint *indices = generateIndices(divisions);

	RenderObject ground(shader, ground_data, number_vertices, indices, number_indicies);

	std::list<RenderObject> objs;

	delete indices;

	double duration = 0;

	update();
	Camera =  Vec3(Camera.getX(), getHeight(ground), Camera.getZ());

	float y = 30;
	float heightOffset = 30;
	Vec3 oldPos = Vec3(Camera.getX(), 0 , Camera.getZ());
	// Main Loop.  Do the stuff!
	while (!glfwWindowShouldClose(window)) {
		// Clear everything on the screen
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		ground.render(ProjectionMatrix, TranslateMatrix, Camera);

		// for objects to be rendered
		for (RenderObject obj : objs) {
			obj.render(ProjectionMatrix, TranslateMatrix, Camera);
		}
		
		glfwSwapBuffers(window);
		glfwPollEvents();

		duration += glfwGetTime();
		if (duration > 1000.0/60.0) {
			update();
			duration = 0;
			if (oldPos.getX() != Camera.getX() && oldPos.getZ() != Camera.getZ()) {
				float tmp = getHeight(ground)+heightOffset;
                        	if (tmp > y)
					y+=0.5;
				else if (tmp < y)
					y-=0.5;
				else
					y=tmp;
				oldPos = Vec3(Camera.getX(), 0, Camera.getZ());
			}
			//std::cout << "Height: " << y << std::endl;
			Camera = Vec3(Camera.getX(), y , Camera.getZ());
		}

	}
	delete ground_data;

	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}
