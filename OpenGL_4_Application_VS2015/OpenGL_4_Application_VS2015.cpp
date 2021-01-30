//
//  main.cpp
//  OpenGL Shadows
//
//  Created by CGIS on 05/12/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"
#include "SkyBox.hpp"

#include <random>

struct directionalLightStruct {
	float ambient, specular;
	glm::vec3 lightPos; // it's actually a vector towards the light considering the light direction is towards to centre of the scene
	glm::vec3 lightColor;
} directionalLight;

struct pointLightStruct {
	glm::vec3 ambient, specular, diffuse;
	float constant, linear, quadratic;
	glm::vec3 lightPos,lightColor;
} pointLight, pointLightLampPost;

struct spotLightStruct {
	GLfloat cutoff1, cutoff2, constant, linear, quadratic;
	glm::vec3 lightPos, lightColor, ambient, specular, diffuse, direction;
} spotLight;
struct modelMatrix {
	glm::vec3 translateVector;
	glm::vec3 scaleVector;
	GLfloat rotateAngle;
	glm::vec3 point1BoundingBox;
	glm::vec3 point2BoundingBox;
	glm::mat4 transformation;
} tree1ModelMatrices[11], tree2ModelMatrices[4], towerMatrix, woodLogMatrix, woodHouseMatrix, campFireMatrix, cloudMatrix[8], actualCloudMatrix[8], groundMatrix, lampHouseMatrix, towerLampMatrix, lightPostMatrix;
GLint numberOfTree1,numberOfTree2,numberOfClouds;

glm::vec3 moonBounding1, moonBounding2;

int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

std::vector<glm::vec3> raindrops;
GLfloat raindropDown = 0.1f;
bool rainStopped = rainStopped;
bool startAnimation = false;
bool checkForMoon = true;
bool checkForClouds = false;
glm::vec3 wind;
GLuint cloudSteps = 1000;
GLuint cloudStepCount = 1;

bool showBoundingBoxes = false;

GLfloat cameraAnimationSpeed = 10.0f;
bool cameraAnimation = false;

bool fillMode = true;

bool towerLightOn = false;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

glm::vec3 lightColor;
GLuint lightColorLoc;
GLuint lightPosLoc;

gps::Camera myCamera(glm::vec3(0.0f, 1.0f, 2.5f), glm::vec3(0.0f, 0.0f, 0.0f));
GLfloat cameraSpeed = 5.0f;

glm::vec3 towerLampTarget;

bool pressedKeys[1024];
GLfloat lightAngle;
GLfloat moonAngle = 180.0f;
GLfloat moonAngleBias = 10.0f;

GLfloat lampTowerAngle;

GLfloat mouseX;
GLfloat mouseY;
GLfloat yaw;
GLfloat pitch;
bool firstMouseMovement = true;

GLfloat deltaTime;
GLfloat lastFrame;

gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D moon;
gps::Model3D tree1;
gps::Model3D tree2;
gps::Model3D tower;
gps::Model3D woodLogs;
gps::Model3D woodHouse;
gps::Model3D campfire;
gps::Model3D raindrop;
gps::Model3D waterSplash;
gps::Model3D cloud;
gps::Model3D lampHouse;
gps::Model3D towerLamp;
gps::Model3D lightPost;
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
GLfloat fogDensity = 0.005;
//pt skybox
//std::vector<const GLchar*> faces;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	
	lightShader.useShaderProgram();
	
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (!cameraAnimation) {
		if (firstMouseMovement)
		{
			mouseX = 0.0f;
			mouseY = 1.0f;
			firstMouseMovement = false;
		}

		GLfloat xoffset = xpos - mouseX;
		GLfloat yoffset = mouseY - ypos; // reversed since y-coordinates range from bottom to top
		mouseX = xpos;
		mouseY = ypos;

		float sensitivity = 0.05f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		myCamera.rotate(pitch, yaw);
	}
}

gps::boundingBox computeNewBoundingBoxFromRotation(gps::boundingBox boundingBox, glm::mat4 modelRotateMatrix) {
	std::vector<glm::vec3> points;
	points.push_back(glm::vec3(boundingBox.xmin, boundingBox.ymin, boundingBox.zmin));
	points.push_back(glm::vec3(boundingBox.xmax, boundingBox.ymin, boundingBox.zmin));
	points.push_back(glm::vec3(boundingBox.xmin, boundingBox.ymax, boundingBox.zmin));
	points.push_back(glm::vec3(boundingBox.xmax, boundingBox.ymax, boundingBox.zmin));
	points.push_back(glm::vec3(boundingBox.xmin, boundingBox.ymin, boundingBox.zmax));
	points.push_back(glm::vec3(boundingBox.xmax, boundingBox.ymin, boundingBox.zmax));
	points.push_back(glm::vec3(boundingBox.xmin, boundingBox.ymax, boundingBox.zmax));
	points.push_back(glm::vec3(boundingBox.xmax, boundingBox.ymax, boundingBox.zmax));

	std::vector<glm::vec3> newPoints;
	for (int i = 0; i < 8; i++) {
		newPoints.push_back(glm::vec3(modelRotateMatrix * glm::vec4(points.at(i), 1.0f)));
	}

	float xmin, xmax, ymin, ymax, zmin, zmax;
	xmin = ymin = zmin = 10000.0f;
	xmax = ymax = zmax = -10000.0f;
	for (int i = 0; i < 8; i++) {
		if (xmin > newPoints.at(i).x) {
			xmin = newPoints.at(i).x;
		}
		if (xmax < newPoints.at(i).x) {
			xmax = newPoints.at(i).x;
		}
		if (ymin > newPoints.at(i).y) {
			ymin = newPoints.at(i).y;
		}
		if (ymax < newPoints.at(i).y) {
			ymax = newPoints.at(i).y;
		}
		if (zmin > newPoints.at(i).z) {
			zmin = newPoints.at(i).z;
		}
		if (zmax < newPoints.at(i).z) {
			zmax = newPoints.at(i).z;
		}
	}
	gps::boundingBox newBoundingBox;
	newBoundingBox.xmin = xmin;
	newBoundingBox.xmax = xmax;
	newBoundingBox.ymin = ymin;
	newBoundingBox.ymax = ymax;
	newBoundingBox.zmin = zmin;
	newBoundingBox.zmax = zmax;
	return newBoundingBox;
}

modelMatrix computeNewBoundingBoxFromTransformations(gps::boundingBox boundingBox, modelMatrix modelMatrixBox) {
	glm::vec3 corner1 = glm::vec3(boundingBox.xmin, boundingBox.ymin, boundingBox.zmin);
	glm::vec3 corner2 = glm::vec3(boundingBox.xmax, boundingBox.ymax, boundingBox.zmax);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), modelMatrixBox.translateVector);
	model = glm::scale(model, modelMatrixBox.scaleVector);
	glm::mat4 modelRotate = glm::rotate(glm::mat4(1.0f), glm::radians(modelMatrixBox.rotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	if (modelMatrixBox.rotateAngle != 0.0f) {
		gps::boundingBox newBoundingBox = computeNewBoundingBoxFromRotation(boundingBox, modelRotate);
		corner1 = glm::vec3(newBoundingBox.xmin, newBoundingBox.ymin, newBoundingBox.zmin);
		corner2 = glm::vec3(newBoundingBox.xmax, newBoundingBox.ymax, newBoundingBox.zmax);
	}

	corner1 = glm::vec3(model * glm::vec4(corner1, 1.0f));
	corner2 = glm::vec3(model * glm::vec4(corner2, 1.0f));

	modelMatrix newModelMatrix;
	newModelMatrix.point1BoundingBox = corner1;
	newModelMatrix.point2BoundingBox = corner2;
	return newModelMatrix;
}

gps::boundingBox computeMoonBoundingBox() {
	gps::boundingBox moonBoundingBox = moon.getBoundingBox();
	glm::vec3 corner1;
	glm::vec3 corner2;

	glm::mat4 modelRotate = glm::rotate(glm::mat4(1.0f), glm::radians(moonAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	moonBoundingBox = computeNewBoundingBoxFromRotation(moonBoundingBox, modelRotate);
	corner1 = glm::vec3(moonBoundingBox.xmin, moonBoundingBox.ymin, moonBoundingBox.zmin);
	corner2 = glm::vec3(moonBoundingBox.xmax, moonBoundingBox.ymax, moonBoundingBox.zmax);

	glm::mat4 modelScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	corner1 = glm::vec3(modelScale * glm::vec4(corner1, 1.0f));
	corner2 = glm::vec3(modelScale * glm::vec4(corner2, 1.0f));

	glm::mat4 modelTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(-8.0f, 0.0f, -4.0f));
	corner1 = glm::vec3(modelTranslate * glm::vec4(corner1, 1.0f));
	corner2 = glm::vec3(modelTranslate * glm::vec4(corner2, 1.0f));

	moonBoundingBox.xmin = corner1.x;
	moonBoundingBox.ymin = corner1.y;
	moonBoundingBox.zmin = corner1.z;
	moonBoundingBox.xmax = corner2.x;
	moonBoundingBox.ymax = corner2.y;
	moonBoundingBox.zmax = corner2.z;

	modelRotate = glm::rotate(glm::mat4(1.0f), glm::radians(moonAngle), glm::vec3(1.0f, 0.0f, 0.0f));
	moonBoundingBox = computeNewBoundingBoxFromRotation(moonBoundingBox, modelRotate);

	modelRotate = glm::rotate(glm::mat4(1.0f), glm::radians(moonAngle), glm::vec3(0.0f, 0.0f, 1.0f));
	moonBoundingBox = computeNewBoundingBoxFromRotation(moonBoundingBox, modelRotate);
	return moonBoundingBox;
}

bool intersectWithMoon(glm::vec3 point) {
	gps::boundingBox moonBoundingBox = computeMoonBoundingBox();
	glm::vec3 corner1 = glm::vec3(moonBoundingBox.xmin, moonBoundingBox.ymin, moonBoundingBox.zmin);
	glm::vec3 corner2 = glm::vec3(moonBoundingBox.xmax, moonBoundingBox.ymax, moonBoundingBox.zmax);

	if (point.x >= corner1.x && point.x <= corner2.x) {
		if (point.y >= corner1.y && point.y <= corner2.y) {
			if (point.z >= corner1.z && point.z <= corner2.z) {
				return true;
			}
		}
	}
	return false;
}

bool intersectBoundingBox(glm::vec3 point,  modelMatrix newModelMatrix) {
	glm::vec3 corner1 = newModelMatrix.point1BoundingBox;
	glm::vec3 corner2 = newModelMatrix.point2BoundingBox;

	if (point.x >= corner1.x && point.x <= corner2.x) {
		if (point.y >= corner1.y && point.y <= corner2.y) {
			if (point.z >= corner1.z && point.z <= corner2.z) {
				return true;
			}
		}
	}
	return false;
}

bool intersectBoundingBoxes(glm::vec3 point) {
	for (int i = 0; i < numberOfTree1; i++) {
		if (intersectBoundingBox(point, tree1ModelMatrices[i])) {
			return true;
		}
	}

	for (int i = 0; i < numberOfTree2; i++) {
		if (intersectBoundingBox(point, tree2ModelMatrices[i])) {
			return true;
		}
	}

	if (intersectBoundingBox(point, towerMatrix)) {
		return true;
	}

	if (intersectBoundingBox(point, woodLogMatrix)) {
		return true;
	}

	if (intersectBoundingBox(point, woodHouseMatrix)) {
		return true;
	}

	if (intersectBoundingBox(point, campFireMatrix)) {
		return true;
	}

	if (intersectBoundingBox(point, towerLampMatrix)) {
		return true;
	}

	if (intersectBoundingBox(point, lightPostMatrix)) {
		return true;
	}

	if (checkForMoon) {
		if (intersectWithMoon(point)) {
			return true;
		}
	}

	if (checkForClouds) {
		for (int i = 0; i < numberOfClouds; i++) {
			modelMatrix newModelMatrix = computeNewBoundingBoxFromTransformations(cloud.getBoundingBox(), actualCloudMatrix[i]);

			if (intersectBoundingBox(point, newModelMatrix)) {
				return true;
			}
		}
	}

	return false;
}

void moveTowerLampOnXandZ(GLfloat quantity) {
	if (quantity > 0.0f) {
		towerLampMatrix.transformation = glm::rotate(towerLampMatrix.transformation, lampTowerAngle * 2, glm::vec3(-1.0f, 0.0f, -1.0f));
		spotLight.lightPos = glm::vec3(glm::rotate(glm::mat4(1.0f), lampTowerAngle * 0.4f, glm::vec3(-1.0f, 0.0f, -1.0f)) * glm::vec4(spotLight.lightPos, 1.0f));
	}
	else {
		towerLampMatrix.transformation = glm::rotate(towerLampMatrix.transformation, -1 * lampTowerAngle * 2, glm::vec3(-1.0f, 0.0f, -1.0f));
		spotLight.lightPos = glm::vec3(glm::rotate(glm::mat4(1.0f), lampTowerAngle * -0.4f, glm::vec3(-1.0f, 0.0f, -1.0f)) * glm::vec4(spotLight.lightPos, 1.0f));
	}
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.lightPos"), 1, glm::value_ptr(spotLight.lightPos));
}

void processMovement()
{

	if (pressedKeys[GLFW_KEY_2]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		fillMode = true;
	}

	if (pressedKeys[GLFW_KEY_1]) {
	
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		fillMode = false;
	}

	if (pressedKeys[GLFW_KEY_3]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		fillMode = false;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed * deltaTime);
		if (intersectBoundingBoxes(myCamera.getCameraPosition())) {
			myCamera.move(gps::MOVE_BACKWARD, cameraSpeed * deltaTime);
		}
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed * deltaTime);
		if (intersectBoundingBoxes(myCamera.getCameraPosition())) {
			myCamera.move(gps::MOVE_FORWARD, cameraSpeed * deltaTime);
		}
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed * deltaTime);
		if (intersectBoundingBoxes(myCamera.getCameraPosition())) {
			myCamera.move(gps::MOVE_RIGHT, cameraSpeed * deltaTime);
		}
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed * deltaTime);
		if (intersectBoundingBoxes(myCamera.getCameraPosition())) {
			myCamera.move(gps::MOVE_LEFT, cameraSpeed * deltaTime);
		}
	}
	
	if (pressedKeys[GLFW_KEY_U]) {
		raindropDown += 0.1f;
		if (raindropDown > 5.0f) {
			raindropDown = 5.0f;
		}
	}

	if (pressedKeys[GLFW_KEY_J]) {
		raindropDown -= 0.1f;
		if (raindropDown < 0.1f) {
			raindropDown = 0.1f;
		}
	}

	if (pressedKeys[GLFW_KEY_P]) {
		startAnimation = true;
	} 

	if (pressedKeys[GLFW_KEY_O]) {
		startAnimation = false;
		cloudStepCount = 1;
	}

	if (pressedKeys[GLFW_KEY_H]) {
		GLfloat newX = wind.x + 0.1f;
		if (newX > 1.0f)
			newX = 1.0f;
		wind.x = newX;
	}

	if (pressedKeys[GLFW_KEY_G]) {
		GLfloat newX = wind.x - 0.1f;
		if (newX < -1.0f)
			newX = -1.0f;
		wind.x = newX;
	}

	if (pressedKeys[GLFW_KEY_K]) {
		GLfloat newZ = wind.z + 0.1f;
		if (newZ > 1.0f)
			newZ = 1.0f;
		wind.z = newZ;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		GLfloat newZ = wind.z - 0.1f;
		if (newZ < -1.0f)
			newZ = -1.0f;
		wind.z = newZ;
	}

	if (pressedKeys[GLFW_KEY_I]) {
		if (!startAnimation) {
			GLuint newSteps = cloudSteps + 10;
			if (newSteps > 1500)
				newSteps = 1500;
			cloudSteps = newSteps;
		}
	}


	if (pressedKeys[GLFW_KEY_Y]) {
		if (!startAnimation) {
			GLuint newSteps = cloudSteps - 10;
			if (newSteps < 500)
				newSteps = 500;
			cloudSteps = newSteps;
		}
	}

	if (pressedKeys[GLFW_KEY_X]) {
		fogDensity += 0.001f;
		if (fogDensity > 0.1f) {
			fogDensity = 0.1f;
		}
		else {
			myCustomShader.useShaderProgram();
			glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
		}
	}

	if (pressedKeys[GLFW_KEY_Z]) {
		fogDensity -= 0.001;
		if (fogDensity < 0.0f) {
			fogDensity = 0.0f;
		} else {
			myCustomShader.useShaderProgram();
			glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
		}
	}

	if (pressedKeys[GLFW_KEY_V]) {
		cameraAnimation = true;
	}

	if (pressedKeys[GLFW_KEY_B]) {
		cameraAnimation = false;
	}

	if (pressedKeys[GLFW_KEY_M]) {
		showBoundingBoxes = true;
	}

	if (pressedKeys[GLFW_KEY_N]) {
		showBoundingBoxes = false;
	}

	if (pressedKeys[GLFW_KEY_UP]) {
		towerLampTarget.y = towerLampTarget.y + 0.1;
		if (towerLampTarget.y > -2.5f) {
			towerLampTarget.y = -2.5f;
		}
		else {
			moveTowerLampOnXandZ(1);
		}
		myCustomShader.useShaderProgram();
		spotLight.direction = towerLampTarget - spotLight.lightPos;
		glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.direction"), 1, glm::value_ptr(spotLight.direction));
	}

	if (pressedKeys[GLFW_KEY_DOWN]) {
		towerLampTarget.y = towerLampTarget.y - 0.1;
		if (towerLampTarget.y < -5.0f) {
			towerLampTarget.y = -5.0f;
		}
		else {
			moveTowerLampOnXandZ(-1);
		}
		myCustomShader.useShaderProgram();
		spotLight.direction = towerLampTarget - spotLight.lightPos;
		glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.direction"), 1, glm::value_ptr(spotLight.direction));
	}

	if (pressedKeys[GLFW_KEY_Q]) {
		towerLightOn = true;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		towerLightOn = false;
	}

	if (cameraAnimation) {
		myCamera.rotateAroundY(cameraAnimationSpeed * deltaTime);
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", glfwGetPrimaryMonitor(), NULL);
	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(glWindow, mouseCallback);

    //glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	//glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBOs()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeObjectTransformation(glm::vec3 translate_vector, glm::vec3 scale_vector, GLfloat rotateAngle) {
	glm::mat4 model = glm::translate(glm::mat4(1.0f), translate_vector);
	model = glm::scale(model, scale_vector);
	model = glm::rotate(model, glm::radians(rotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	return model;
}

void initModelMatrices() {
	tree1ModelMatrices[0].translateVector = glm::vec3(5.0f, -1.3f, -1.0f);
	tree1ModelMatrices[0].scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);
	tree1ModelMatrices[0].rotateAngle = 10.0f;
	tree1ModelMatrices[1].translateVector = glm::vec3(5.0f, -1.3f, 5.0f);
	tree1ModelMatrices[1].scaleVector = glm::vec3(0.7f, 0.7f, 0.7f);
	tree1ModelMatrices[1].rotateAngle = 20.0f;
	tree1ModelMatrices[2].translateVector = glm::vec3(5.0f, -1.3f, -5.0f);
	tree1ModelMatrices[2].scaleVector = glm::vec3(0.3f, 0.3f, 0.3f);
	tree1ModelMatrices[2].rotateAngle = 30.0f;
	tree1ModelMatrices[3].translateVector = glm::vec3(2.0f, -1.3f, 2.0f);
	tree1ModelMatrices[3].scaleVector = glm::vec3(0.4f, 0.4f, 0.4f);
	tree1ModelMatrices[3].rotateAngle = 40.0f;
	tree1ModelMatrices[4].translateVector = glm::vec3(2.0f, -1.3f, 4.0f);
	tree1ModelMatrices[4].scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);
	tree1ModelMatrices[4].rotateAngle = 50.0f;
	tree1ModelMatrices[5].translateVector = glm::vec3(4.0f, -1.3f, 2.0f);
	tree1ModelMatrices[5].scaleVector = glm::vec3(0.6f, 0.6f, 0.6f);
	tree1ModelMatrices[5].rotateAngle = 60.0f;
	tree1ModelMatrices[6].translateVector = glm::vec3(-2.0f, -1.3f, -2.0f);
	tree1ModelMatrices[6].scaleVector = glm::vec3(0.7f, 0.7f, 0.7f);
	tree1ModelMatrices[6].rotateAngle = 70.0f;
	tree1ModelMatrices[7].translateVector = glm::vec3(5.0f, -1.3f, 1.0f);
	tree1ModelMatrices[7].scaleVector = glm::vec3(0.3f, 0.3f, 0.3f);
	tree1ModelMatrices[7].rotateAngle = 80.0f;
	tree1ModelMatrices[8].translateVector = glm::vec3(-5.0f, -1.3f, -5.0f);
	tree1ModelMatrices[8].scaleVector = glm::vec3(0.4f, 0.4f, 0.4f);
	tree1ModelMatrices[8].rotateAngle = 90.0f;
	tree1ModelMatrices[9].translateVector = glm::vec3(-5.0f, -1.3f, 5.0f);
	tree1ModelMatrices[9].scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);
	tree1ModelMatrices[9].rotateAngle = 100.0f;
	tree1ModelMatrices[10].translateVector = glm::vec3(-2.0f, -1.3f, -4.0f);
	tree1ModelMatrices[10].scaleVector = glm::vec3(0.6f, 0.6f, 0.6f);
	tree1ModelMatrices[10].rotateAngle = 110.0f;
	numberOfTree1 = 11;

	for (int i = 0; i < numberOfTree1; i++) {
		modelMatrix newModelMatrix = computeNewBoundingBoxFromTransformations(tree1.getBoundingBox(), tree1ModelMatrices[i]);
		tree1ModelMatrices[i].point1BoundingBox = newModelMatrix.point1BoundingBox;
		tree1ModelMatrices[i].point2BoundingBox = newModelMatrix.point2BoundingBox;
		tree1ModelMatrices[i].transformation = computeObjectTransformation(tree1ModelMatrices[i].translateVector, tree1ModelMatrices[i].scaleVector, tree1ModelMatrices[i].rotateAngle);
	}

	tree2ModelMatrices[0].translateVector = glm::vec3(-1.0f, -1.0f, -1.0f);
	tree2ModelMatrices[0].scaleVector = glm::vec3(0.5f, 0.5f, 0.5);
	tree2ModelMatrices[0].rotateAngle = 10.0f;
	tree2ModelMatrices[1].translateVector = glm::vec3(0.0f, -1.0f, 5.0f);
	tree2ModelMatrices[1].scaleVector = glm::vec3(0.6f, 0.6f, 0.6f);
	tree2ModelMatrices[1].rotateAngle = 20.0f;
	tree2ModelMatrices[2].translateVector = glm::vec3(-2.0f, -1.0f, 2.0f);
	tree2ModelMatrices[2].scaleVector = glm::vec3(0.7f, 0.7f, 0.7f);
	tree2ModelMatrices[2].rotateAngle = 30.0f;
	tree2ModelMatrices[3].translateVector = glm::vec3(3.0f, -1.0f, 3.0f);
	tree2ModelMatrices[3].scaleVector = glm::vec3(0.4f, 0.4f, 0.4f);
	tree2ModelMatrices[3].rotateAngle = 40.0f;

	numberOfTree2 = 4;

	for (int i = 0; i < numberOfTree2; i++) {
		modelMatrix newModelMatrix = computeNewBoundingBoxFromTransformations(tree2.getBoundingBox(), tree2ModelMatrices[i]);
		tree2ModelMatrices[i].point1BoundingBox = newModelMatrix.point1BoundingBox;
		tree2ModelMatrices[i].point2BoundingBox = newModelMatrix.point2BoundingBox;
		tree2ModelMatrices[i].transformation = computeObjectTransformation(tree2ModelMatrices[i].translateVector, tree2ModelMatrices[i].scaleVector, tree2ModelMatrices[i].rotateAngle);
	}

	towerMatrix.translateVector = glm::vec3(3.3f, -1.1f, -3.3f);
	towerMatrix.scaleVector = glm::vec3(0.4f, 0.4f, 0.4f);
	towerMatrix.rotateAngle = 135.0f;

	modelMatrix newModelMatrix = computeNewBoundingBoxFromTransformations(tower.getBoundingBox(), towerMatrix);
	towerMatrix.point1BoundingBox = newModelMatrix.point1BoundingBox;
	towerMatrix.point2BoundingBox = newModelMatrix.point2BoundingBox;
	towerMatrix.transformation = computeObjectTransformation(towerMatrix.translateVector, towerMatrix.scaleVector, towerMatrix.rotateAngle);

	woodLogMatrix.translateVector = glm::vec3(-1.4f, -0.9f, 2.6f);
	woodLogMatrix.scaleVector = glm::vec3(0.05f, 0.05f, 0.05f);
	woodLogMatrix.rotateAngle = -45.0f;

	newModelMatrix = computeNewBoundingBoxFromTransformations(woodLogs.getBoundingBox(), woodLogMatrix);
	woodLogMatrix.point1BoundingBox = newModelMatrix.point1BoundingBox;
	woodLogMatrix.point2BoundingBox = newModelMatrix.point2BoundingBox;
	woodLogMatrix.transformation = computeObjectTransformation(woodLogMatrix.translateVector, woodLogMatrix.scaleVector, woodLogMatrix.rotateAngle);

	woodHouseMatrix.translateVector = glm::vec3(-1.6f, -0.85f, 1.0f);
	woodHouseMatrix.scaleVector = glm::vec3(1.0f, 1.0f, 1.0f);
	woodHouseMatrix.rotateAngle = 90.0f;

	newModelMatrix = computeNewBoundingBoxFromTransformations(woodHouse.getBoundingBox(), woodHouseMatrix);
	woodHouseMatrix.point1BoundingBox = newModelMatrix.point1BoundingBox;
	woodHouseMatrix.point2BoundingBox = newModelMatrix.point2BoundingBox;
	woodHouseMatrix.transformation = computeObjectTransformation(woodHouseMatrix.translateVector, woodHouseMatrix.scaleVector, woodHouseMatrix.rotateAngle);

	campFireMatrix.translateVector = glm::vec3(1.0f, -0.7f, 0.5f);
	campFireMatrix.scaleVector = glm::vec3(0.1f, 0.1f, 0.1);
	campFireMatrix.rotateAngle = 0.0f;

	newModelMatrix = computeNewBoundingBoxFromTransformations(campfire.getBoundingBox(), campFireMatrix);
	campFireMatrix.point1BoundingBox = newModelMatrix.point1BoundingBox;
	campFireMatrix.point2BoundingBox = newModelMatrix.point2BoundingBox;
	campFireMatrix.transformation = computeObjectTransformation(campFireMatrix.translateVector, campFireMatrix.scaleVector, campFireMatrix.rotateAngle);

	groundMatrix.translateVector = glm::vec3(0.0f, -1.3f, 0.0f);
	groundMatrix.scaleVector = glm::vec3(0.55, 0.55, 0.55);
	groundMatrix.transformation = computeObjectTransformation(groundMatrix.translateVector, groundMatrix.scaleVector, groundMatrix.rotateAngle);

	gps::boundingBox boundingBox = ground.getBoundingBox();
	glm::vec3 corner1 = glm::vec3(boundingBox.xmin, boundingBox.ymin, boundingBox.zmin);
	glm::vec3 corner2 = glm::vec3(boundingBox.xmax, boundingBox.ymax, boundingBox.zmax);
	corner1 = glm::vec3(groundMatrix.transformation * glm::vec4(corner1, 1.0f));
	corner2 = glm::vec3(groundMatrix.transformation * glm::vec4(corner2, 1.0f));
	boundingBox.xmin = corner1.x;
	boundingBox.ymin = corner1.y;
	boundingBox.zmin = corner1.z;
	boundingBox.xmax = corner2.x;
	boundingBox.ymax = corner2.y;
	boundingBox.zmax = corner2.z;

	ground.setBoundingBox(boundingBox);

	cloudMatrix[0].translateVector = glm::vec3(ground.getBoundingBox().xmin, 8.0f, ground.getBoundingBox().zmin);
	cloudMatrix[0].scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);
	cloudMatrix[0].rotateAngle = 0.0f;


	cloudMatrix[1].translateVector = glm::vec3(ground.getBoundingBox().xmin, 8.0f, ground.getBoundingBox().zmax);
	cloudMatrix[1].scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);
	cloudMatrix[1].rotateAngle = 0.0f;


	cloudMatrix[2].translateVector = glm::vec3(ground.getBoundingBox().xmax, 8.0f, ground.getBoundingBox().zmin);
	cloudMatrix[2].scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);
	cloudMatrix[2].rotateAngle = 0.0f;

	
	cloudMatrix[3].translateVector = glm::vec3(ground.getBoundingBox().xmax, 8.0f, ground.getBoundingBox().zmax);
	cloudMatrix[3].scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);
	cloudMatrix[3].rotateAngle = 0.0f;

	cloudMatrix[4].translateVector = glm::vec3((ground.getBoundingBox().xmax + ground.getBoundingBox().xmin)/2, 8.0f, ground.getBoundingBox().zmax);
	cloudMatrix[4].scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);
	cloudMatrix[4].rotateAngle = 0.0f;
	
	cloudMatrix[5].translateVector = glm::vec3((ground.getBoundingBox().xmax + ground.getBoundingBox().xmin)/2, 8.0f, ground.getBoundingBox().zmin);
	cloudMatrix[5].scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);
	cloudMatrix[5].rotateAngle = 0.0f;

	cloudMatrix[6].translateVector = glm::vec3(ground.getBoundingBox().xmax, 8.0f, (ground.getBoundingBox().zmax + ground.getBoundingBox().zmin) / 2);
	cloudMatrix[6].scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);
	cloudMatrix[6].rotateAngle = 0.0f;

	cloudMatrix[7].translateVector = glm::vec3(ground.getBoundingBox().xmin, 8.0f, (ground.getBoundingBox().zmax + ground.getBoundingBox().zmin)/2);
	cloudMatrix[7].scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);
	cloudMatrix[7].rotateAngle = 0.0f;

	numberOfClouds = 8;

	for (int i = 0; i < numberOfClouds; i++) {
		cloudMatrix[i].transformation = computeObjectTransformation(cloudMatrix[i].translateVector, cloudMatrix[i].scaleVector, cloudMatrix[i].rotateAngle);
	}

	for (int i = 0; i < numberOfClouds; i++) {
		actualCloudMatrix[i] = cloudMatrix[i];
	}

	lampHouseMatrix.translateVector = glm::vec3(-0.6f, -0.61f, 1.2f);
	lampHouseMatrix.scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);
	lampHouseMatrix.rotateAngle = 180.0f;
	lampHouseMatrix.transformation = computeObjectTransformation(lampHouseMatrix.translateVector, lampHouseMatrix.scaleVector, lampHouseMatrix.rotateAngle);

	towerLampMatrix.translateVector = glm::vec3(2.3f, 1.9f, -2.5f);
	towerLampMatrix.scaleVector = glm::vec3(0.25f, 0.25f, 0.25f);
	towerLampMatrix.transformation = computeObjectTransformation(towerLampMatrix.translateVector, towerLampMatrix.scaleVector, towerLampMatrix.rotateAngle);
	towerLampMatrix.transformation = glm::rotate(towerLampMatrix.transformation, glm::radians(45.0f), glm::vec3(-1.0f, 0.0f, -1.0f));

	newModelMatrix = computeNewBoundingBoxFromTransformations(towerLamp.getBoundingBox(), towerLampMatrix);
	towerLampMatrix.point1BoundingBox = newModelMatrix.point1BoundingBox;
	towerLampMatrix.point2BoundingBox = newModelMatrix.point2BoundingBox;

	lightPostMatrix.translateVector = glm::vec3(2.1f, -0.75f, -2.9f);
	lightPostMatrix.scaleVector = glm::vec3(0.2f, 0.2f, 0.2f);
	lightPostMatrix.rotateAngle = -45.0f;
	lightPostMatrix.transformation = computeObjectTransformation(lightPostMatrix.translateVector, lightPostMatrix.scaleVector, lightPostMatrix.rotateAngle);

	newModelMatrix = computeNewBoundingBoxFromTransformations(lightPost.getBoundingBox(), lightPostMatrix);
	lightPostMatrix.point1BoundingBox = newModelMatrix.point1BoundingBox;
	lightPostMatrix.point2BoundingBox = newModelMatrix.point2BoundingBox;

}


glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = -10.0f, far_plane = 20.0f;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);


	model = glm::rotate(glm::mat4(1.0f), glm::radians(moonAngle), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(moonAngle), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-8.0f, 0.0f, -4.0f));
	glm::vec3 lightPosTr = model * glm::vec4(directionalLight.lightPos, 1.0f);
	glm::mat4 lightView = glm::lookAt(lightPosTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}

void initModels()
{
	ground = gps::Model3D("objects/ground/ground.obj", "objects/ground/");
	lightCube = gps::Model3D("objects/cube/cube.obj", "objects/cube/");
	moon = gps::Model3D("objects/moon/moon.obj", "objects/moon/");
	tower = gps::Model3D("objects/tower/tower.obj", "objects/tower/");
	woodLogs = gps::Model3D("objects/wood_logs/wood_logs.obj", "objects/wood_logs/");
	campfire = gps::Model3D("objects/campFire/campfire.obj", "objects/campFire/");
	woodHouse = gps::Model3D("objects/wooden_house/house.obj", "objects/wooden_house/");
	raindrop = gps::Model3D("objects/raindrop/raindrop.obj", "objects/raindrop/");
	waterSplash = gps::Model3D("objects/water_splash/waterSplash.obj", "objects/water_splash/");
	cloud = gps::Model3D("objects/cloud/cloud.obj", "objects/cloud/");
	lampHouse = gps::Model3D("objects/lampHouse/lampHouse.obj", "objects/lampHouse/");
	towerLamp = gps::Model3D("objects/towerLamp/towerLamp.obj", "objects/towerLamp/");
	lightPost = gps::Model3D("objects/lightPost/lightPost.obj", "objects/lightPost/");
	tree1 = gps::Model3D("objects/trees/tree1/tree.obj", "objects/trees/tree1/");
	tree2 = gps::Model3D("objects/trees/tree2/tree.obj", "objects/trees/tree2/");
}

void initShaders()
{
	//pt skybox
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	

	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
}

void initUniformsDirectionalLight() {
	//set the light direction (direction towards the light)
	directionalLight.lightPos = glm::vec3(0.0f, 1.0f, 1.0f);
	lightPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "directionalLight.lightPos");
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(directionalLight.lightPos));

	//set light color
	directionalLight.lightColor = glm::vec3(0.9f, 0.9f, 0.9f); 
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "directionalLight.lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(directionalLight.lightColor));

	directionalLight.ambient = 0.05f;
	directionalLight.specular = 0.25f;
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "directionalLight.ambient"), directionalLight.ambient);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "directionalLight.specular"), directionalLight.specular);
}

void initUniformsPointLight() {
	pointLight.constant = 0.05f;
	pointLight.linear = 0.75f;
	pointLight.quadratic = 0.5f;

	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLight.constant"), pointLight.constant);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLight.linear"), pointLight.linear);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLight.quadratic"), pointLight.quadratic);

	pointLight.lightPos = lampHouseMatrix.translateVector;
	pointLight.lightPos.y = pointLight.lightPos.y + 0.05f;

	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLight.lightPos"), 1, glm::value_ptr(pointLight.lightPos));

	pointLight.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	pointLight.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	pointLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);

	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLight.ambient"),1, glm::value_ptr(pointLight.ambient));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLight.diffuse"),1,glm::value_ptr( pointLight.diffuse));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLight.specular"),1, glm::value_ptr(pointLight.specular));

	pointLight.lightColor = glm::vec3(2.0f, 2.0f, 2.0f);
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLight.lightColor"), 1, glm::value_ptr(pointLight.lightColor));
}

void initUniformsSpotLight() {
	towerLampTarget = glm::vec3(-1.475f, -3.0f, 2.075f);

	spotLight.constant = 1.0f;
	spotLight.linear = 0.09f;
	spotLight.quadratic = 0.02f;

	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.constant"), spotLight.constant);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.linear"), spotLight.linear);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.quadratic"), spotLight.quadratic);

	spotLight.lightPos = glm::vec3((towerLampMatrix.point2BoundingBox.x + towerLampMatrix.point1BoundingBox.x)/2 + 0.4f , (towerLampMatrix.point2BoundingBox.y + towerLampMatrix.point1BoundingBox.y) / 2 + 0.105f, (towerLampMatrix.point2BoundingBox.z + towerLampMatrix.point1BoundingBox.z) / 2 - 0.35f);
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.lightPos"), 1, glm::value_ptr(spotLight.lightPos));

	spotLight.ambient = glm::vec3(0.0f, 0.0f, 0.0f);
	spotLight.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	spotLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);

	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.ambient"), 1, glm::value_ptr(spotLight.ambient));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.diffuse"), 1, glm::value_ptr(spotLight.diffuse));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.specular"), 1, glm::value_ptr(spotLight.specular));

	spotLight.lightColor = glm::vec3(8.0f, 8.0f, 8.0f);
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.lightColor"), 1, glm::value_ptr(spotLight.lightColor));

	spotLight.cutoff1 = glm::cos(glm::radians(10.0f));
	spotLight.cutoff2 = glm::cos(glm::radians(13.0f));

	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.cutoff1"), spotLight.cutoff1);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.cutoff2"), spotLight.cutoff2);

	spotLight.direction = towerLampTarget - spotLight.lightPos;
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.direction"), 1, glm::value_ptr(spotLight.direction));

	glm::vec3 point1 = towerLampTarget - spotLight.lightPos;
	glm::vec3 point2 = glm::vec3(point1.x,point1.y - 0.1,point1.z);
	point1 = glm::normalize(point1);
	point2 = glm::normalize(point2);
	lampTowerAngle = glm::acos(glm::dot(point1, point2));
}

void initUniformsPointLightLampPost() {
	pointLightLampPost.constant = 0.05f;
	pointLightLampPost.linear = 0.5f;
	pointLightLampPost.quadratic = 0.005f;

	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightLampPost.constant"), pointLightLampPost.constant);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightLampPost.linear"), pointLightLampPost.linear);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightLampPost.quadratic"), pointLightLampPost.quadratic);

	pointLightLampPost.lightPos = glm::vec3(2.4f, 0.07f, -2.6f);

	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightLampPost.lightPos"), 1, glm::value_ptr(pointLightLampPost.lightPos));

	pointLightLampPost.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	pointLightLampPost.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	pointLightLampPost.specular = glm::vec3(1.0f, 1.0f, 1.0f);

	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightLampPost.ambient"), 1, glm::value_ptr(pointLightLampPost.ambient));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightLampPost.diffuse"), 1, glm::value_ptr(pointLightLampPost.diffuse));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightLampPost.specular"), 1, glm::value_ptr(pointLightLampPost.specular));

	pointLightLampPost.lightColor = glm::vec3(2.0f, 2.0f, 2.0f);
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightLampPost.lightColor"), 1, glm::value_ptr(pointLightLampPost.lightColor));
}

void initUniforms()
{
	myCustomShader.useShaderProgram();

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	
	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	initUniformsDirectionalLight();
	initUniformsPointLight();
	initUniformsSpotLight();
	initUniformsPointLightLampPost();

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

//pt skybox
void initSkyBox() {
	std::vector<const GLchar*> faces;
	faces.push_back("skybox/right.tga");
	faces.push_back("skybox/left.tga");
	faces.push_back("skybox/top.tga");
	faces.push_back("skybox/bottom.tga");
	faces.push_back("skybox/back.tga");
	faces.push_back("skybox/front.tga");

	mySkyBox.Load(faces);

}

void sendModelTransformation(gps::Shader shader, glm::mat4 model) {
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}

void computeMoonPosition(gps::Shader shader) {
	//create model matrix 
	model = glm::rotate(glm::mat4(1.0f), glm::radians(moonAngle), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(moonAngle), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-8.0f, 0.0f, -4.0f));
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
	model = glm::rotate(model, glm::radians(moonAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

}

void computeMoonAngle() {
	moonAngle += moonAngleBias * deltaTime;
	if (moonAngle > 360.0f) {
		moonAngle = 360.0f;
		moonAngleBias *= -1;
	}

	if (moonAngle < 180.0f) {
		moonAngle = 180.0f;
		moonAngleBias *= -1;
	}
}

void moveRaindrops() {
	checkForMoon = false;
	if (!startAnimation) {
		raindrops.clear();
		for (int i = 0; i < numberOfClouds; i++) {
			actualCloudMatrix[i] = cloudMatrix[i];
		}
		checkForClouds = false;
	}
	else {
		if (cloudStepCount <= cloudSteps) {
			for (int i = 0; i < numberOfClouds; i++) {
				glm::vec3 newTranslateVector = glm::vec3(actualCloudMatrix[i].translateVector.x - cloudMatrix[i].translateVector.x / (cloudSteps * 2), cloudMatrix[i].translateVector.y, actualCloudMatrix[i].translateVector.z - cloudMatrix[i].translateVector.z / (cloudSteps * 2));
				glm::mat4 newModelMatrix = computeObjectTransformation(newTranslateVector, cloudMatrix[i].scaleVector, cloudMatrix[i].rotateAngle);
				sendModelTransformation(lightShader, newModelMatrix);
				cloud.Draw(lightShader);
				actualCloudMatrix[i].translateVector = newTranslateVector;
				actualCloudMatrix[i].transformation = newModelMatrix;
			}
			cloudStepCount++;
		}
		else {
			checkForClouds = false;
			for (int i = 0; i <= numberOfClouds; i++) {
				glm::mat4 newModelMatrix = computeObjectTransformation(actualCloudMatrix[i].translateVector, cloudMatrix[i].scaleVector, cloudMatrix[i].rotateAngle);
				sendModelTransformation(lightShader, newModelMatrix);
				cloud.Draw(lightShader);
			}
			std::vector<glm::vec3> newRaindrops;
			for (int i = 0; i < raindrops.size(); i++) {
				glm::vec3 droplet = raindrops.at(i);
				GLfloat newY = droplet.y - raindropDown * deltaTime;
				GLfloat newX = droplet.x + wind.x * deltaTime;
				GLfloat newZ = droplet.z + wind.z * deltaTime;
				if (newY > ground.getBoundingBox().ymin && newX >= ground.getBoundingBox().xmin && newX <= ground.getBoundingBox().xmax && newZ >= ground.getBoundingBox().zmin && newZ <= ground.getBoundingBox().zmax) {
					if (!intersectBoundingBoxes(glm::vec3(newX, newY, newZ))) {
						droplet = glm::vec3(newX, newY, newZ);
						newRaindrops.push_back(droplet);
					}
					else {
						glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(newX, newY, newZ));
						model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
						glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"),
							1,
							GL_FALSE,
							glm::value_ptr(model));
						waterSplash.Draw(lightShader);
					}
				}
			}

			std::random_device rd1;
			std::random_device rd2;
			std::random_device rd3;
			std::mt19937 gen1(rd1());
			std::mt19937 gen2(rd2());
			std::mt19937 gen3(rd3());
			std::uniform_real_distribution<> dis1(ground.getBoundingBox().xmin, ground.getBoundingBox().xmax);
			std::uniform_real_distribution<> dis2(ground.getBoundingBox().zmin, ground.getBoundingBox().zmax);
			std::uniform_real_distribution<> dis3(8.0f, 1.0f);

			for (int i = 0; i < 1000; i++) {
				newRaindrops.push_back(glm::vec3(dis1(gen1), dis3(gen3), dis2(gen2)));
			}
			for (int i = 0; i < newRaindrops.size(); i++) {
				glm::mat4 model = glm::translate(glm::mat4(0.1f), newRaindrops.at(i));
				model = glm::scale(model, glm::vec3(0.01f, 0.01, 0.01f));
				glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"),
					1,
					GL_FALSE,
					glm::value_ptr(model));
				raindrop.Draw(lightShader);
			}
			raindrops = newRaindrops;
		}
		checkForClouds = true;
	}
	checkForMoon = true;
}

void drawBoundingBox(modelMatrix modelMatrix) {
	glm::vec3 point1 = modelMatrix.point1BoundingBox;
	glm::vec3 point2 = modelMatrix.point2BoundingBox;
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3((point1.x + point2.x) / 2, (point1.y + point2.y) / 2, (point1.z + point2.z) / 2));
	gps::boundingBox box = lightCube.getBoundingBox();
	model = glm::scale(model, glm::vec3((point2.x - point1.x) / (box.xmax - box.xmin), (point2.y - point1.y) / (box.ymax - box.ymin), (point2.z - point1.z) / (box.zmax - box.zmin)));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	lightCube.Draw(lightShader);
}

void drawBoundingBoxes(glm::vec3 point1, glm::vec3 point2) {
	if (fillMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	for (int i = 0; i < numberOfTree1; i++) {
		drawBoundingBox(tree1ModelMatrices[i]);
	}

	for (int i = 0; i < numberOfTree2; i++) {
		drawBoundingBox(tree2ModelMatrices[i]);
	}

	drawBoundingBox(towerMatrix);

	drawBoundingBox(woodLogMatrix);

	drawBoundingBox(woodHouseMatrix);

	drawBoundingBox(campFireMatrix);

	drawBoundingBox(towerLampMatrix);

	drawBoundingBox(lightPostMatrix);

	gps::boundingBox moonBoundingBox = computeMoonBoundingBox();
	modelMatrix moonModelMatrix;
	moonModelMatrix.point1BoundingBox = glm::vec3(moonBoundingBox.xmin, moonBoundingBox.ymin, moonBoundingBox.zmin);
	moonModelMatrix.point2BoundingBox = glm::vec3(moonBoundingBox.xmax, moonBoundingBox.ymax, moonBoundingBox.zmax);
	drawBoundingBox(moonModelMatrix);

	if (checkForClouds) {
		for (int i = 0; i < numberOfClouds; i++) {
			modelMatrix newModelMatrix = computeNewBoundingBoxFromTransformations(cloud.getBoundingBox(), actualCloudMatrix[i]);

			drawBoundingBox(newModelMatrix);
		}
	}
	if (fillMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void renderScene()
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	computeMoonAngle();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	processMovement();	

	//render the scene to the depth buffer (first pass)

	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
		
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	sendModelTransformation(depthMapShader, towerMatrix.transformation);
	tower.Draw(depthMapShader);

	sendModelTransformation(depthMapShader, woodLogMatrix.transformation);
	woodLogs.Draw(depthMapShader);

	sendModelTransformation(depthMapShader, woodHouseMatrix.transformation);
	woodHouse.Draw(depthMapShader);

	sendModelTransformation(depthMapShader, campFireMatrix.transformation);
	campfire.Draw(depthMapShader);

	sendModelTransformation(depthMapShader, lightPostMatrix.transformation);
	lightPost.Draw(depthMapShader);

	for (int i = 0; i < numberOfTree1; i++) {
		sendModelTransformation(depthMapShader, tree1ModelMatrices[i].transformation);
		tree1.Draw(depthMapShader);
	}
	for (int i = 0; i < numberOfTree2; i++) {
		sendModelTransformation(depthMapShader, tree2ModelMatrices[i].transformation);
		tree2.Draw(depthMapShader);
	}
	
	sendModelTransformation(depthMapShader, groundMatrix.transformation);
	ground.Draw(depthMapShader);

	sendModelTransformation(depthMapShader, towerLampMatrix.transformation);
	towerLamp.Draw(depthMapShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//render the scene (second pass)
	myCustomShader.useShaderProgram();

	//send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	//send view matrix to shader
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
		1,
		GL_FALSE,
		glm::value_ptr(view));	

	//compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	myCustomShader.useShaderProgram();
	if (towerLightOn) {
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "lightOn"), 1.0f);
	}
	else {
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "lightOn"), 0.0f);
	}

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

	//compute normal matrix

	sendModelTransformation(myCustomShader, campFireMatrix.transformation);
	campfire.Draw(myCustomShader);

	sendModelTransformation(myCustomShader, towerMatrix.transformation);
	tower.Draw(myCustomShader);

	sendModelTransformation(myCustomShader, woodLogMatrix.transformation);
	woodLogs.Draw(myCustomShader);

	sendModelTransformation(myCustomShader, woodHouseMatrix.transformation);
	woodHouse.Draw(myCustomShader);

	sendModelTransformation(myCustomShader, lampHouseMatrix.transformation);
	lampHouse.Draw(myCustomShader);

	for (int i = 0; i < numberOfTree1; i++) {
		sendModelTransformation(myCustomShader, tree1ModelMatrices[i].transformation);
		tree1.Draw(myCustomShader);
	}
	for (int i = 0; i < numberOfTree2; i++) {
		sendModelTransformation(myCustomShader, tree2ModelMatrices[i].transformation);
		tree2.Draw(myCustomShader);
	}

	sendModelTransformation(myCustomShader, towerLampMatrix.transformation);
	towerLamp.Draw(myCustomShader);

	sendModelTransformation(myCustomShader, lightPostMatrix.transformation);
	lightPost.Draw(myCustomShader);

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	sendModelTransformation(myCustomShader, groundMatrix.transformation);
	ground.Draw(myCustomShader);

	//draw a white cube around the light

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	computeMoonPosition(lightShader);
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	moon.Draw(lightShader);

	moveRaindrops();

	if (showBoundingBoxes) {
		drawBoundingBoxes(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(2.0f, 3.0f, 2.0f));
	}
	
	glm::mat4 model = glm::translate(glm::mat4(1.0f), pointLightLampPost.lightPos);
	model = glm::scale(model, glm::vec3(0.01, 0.01f, 0.01f));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	lightCube.Draw(lightShader);
	

	//pt skybox
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));


	mySkyBox.Draw(skyboxShader, view, projection);


}

int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initFBOs();
	initModels();
	initShaders();
	initModelMatrices();
	initUniforms();	
	initSkyBox();
	glCheckError();
	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
