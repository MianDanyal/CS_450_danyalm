#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <GL/glew.h>					
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "MeshData.hpp"
#include "MeshGLData.hpp"
#include "GLSetup.hpp"
#include "Shader.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
using namespace std;

//i
// Create very simple mesh: a quad (4 vertices, 6 indices, 2 triangles)
void createSimpleQuad(Mesh &m) {
	// Clear out vertices and elements
	m.vertices.clear();
	m.indices.clear();

	// Create four corners
	Vertex upperLeft, upperRight;
	Vertex lowerLeft, lowerRight;

	// Set positions of vertices
	// Note: glm::vec3(x, y, z)
	upperLeft.position = glm::vec3(-0.5, 0.5, 0.0);
	upperRight.position = glm::vec3(0.5, 0.5, 0.0);
	lowerLeft.position = glm::vec3(-0.5, -0.5, 0.0);
	lowerRight.position = glm::vec3(0.5, -0.5, 0.0);

	// Set vertex colors (red, green, blue, white)
	// Note: glm::vec4(red, green, blue, alpha)
	upperLeft.color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	upperRight.color = glm::vec4(0.0, 1.0, 0.0, 1.0);
	lowerLeft.color = glm::vec4(0.0, 0.0, 1.0, 1.0);
	lowerRight.color = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Add to mesh's list of vertices
	m.vertices.push_back(upperLeft);
	m.vertices.push_back(upperRight);	
	m.vertices.push_back(lowerLeft);
	m.vertices.push_back(lowerRight);
	
	// Add indices for two triangles
	m.indices.push_back(0);
	m.indices.push_back(3);
	m.indices.push_back(1);

	m.indices.push_back(0);
	m.indices.push_back(2);
	m.indices.push_back(3);
}

// Create very simple mesh: a quad (4 vertices, 6 indices, 2 triangles)
void createSimplePentagon(Mesh &m) {
	// Clear out vertices and elements
	m.vertices.clear();
	m.indices.clear();

	// Create four corners
	Vertex upperLeft, upperRight;
	Vertex lowerLeft, lowerRight;
	Vertex fifthVertex;

	// Set positions of vertices
	// Note: glm::vec3(x, y, z)
	upperLeft.position = glm::vec3(-0.5, 0.5, 0.0);
	upperRight.position = glm::vec3(0.5, 0.5, 0.0);
	lowerLeft.position = glm::vec3(-0.5, -0.5, 0.0);
	lowerRight.position = glm::vec3(0.5, -0.5, 0.0);
	fifthVertex.position = glm::vec3(0, 0.8, 0.0);

	// Set vertex colors (red, green, blue, white)
	// Note: glm::vec4(red, green, blue, alpha)
	upperLeft.color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	upperRight.color = glm::vec4(0.0, 1.0, 0.0, 1.0);
	lowerLeft.color = glm::vec4(0.0, 0.0, 1.0, 1.0);
	lowerRight.color = glm::vec4(1.0, 1.0, 1.0, 1.0);
	fifthVertex.color = glm::vec4(1.0, 1.0, 0.0, 1.0);

	// Add to mesh's list of vertices
	m.vertices.push_back(upperLeft);
	m.vertices.push_back(upperRight);	
	m.vertices.push_back(lowerLeft);
	m.vertices.push_back(lowerRight);
	m.vertices.push_back(fifthVertex);
	
	// Add indices for two triangles
	m.indices.push_back(0);
	m.indices.push_back(3);
	m.indices.push_back(1);

	m.indices.push_back(0);
	m.indices.push_back(2);
	m.indices.push_back(3);

	m.indices.push_back(0);
	m.indices.push_back(4);
	m.indices.push_back(1);
}

void extractMeshData(aiMesh *mesh, Mesh &m) {
	m.vertices.clear();
    m.indices.clear();

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;

			vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			vertex.color = glm::vec4(0.0, 0.0, 1.0, 1.0);

			m.vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		for(unsigned int j = 0; j < face.mNumIndices; j++) {
			m.indices.push_back(face.mIndices[j]);
		}
	}
}

// Main 
int main(int argc, char **argv) {
	
	// Are we in debugging mode?
	bool DEBUG_MODE = true;

	string modelPath = "sampleModels/sphere.obj";

	if(argc >= 2) {
		modelPath = string(argv[1]);
	}

	Assimp::Importer importer;

	const aiScene *scene = importer.ReadFile(modelPath,
	aiProcess_Triangulate | aiProcess_FlipUVs |
	aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		cerr << "Error: " << importer.GetErrorString() << endl;
		exit(1);
	}

	vector<MeshGL> meshes;



	// GLFW setup
	// Switch to 4.1 if necessary for macOS
	GLFWwindow* window = setupGLFW("Assign03: danyalm", 4, 3, 800, 800, DEBUG_MODE);

	// GLEW setup
	setupGLEW(window);

	// Check OpenGL version
	checkOpenGLVersion();

	// Set up debugging (if requested)
	if(DEBUG_MODE) checkAndSetupOpenGLDebugging();

	// Set the background color to a shade of blue
	glClearColor(0.8f, 0.9f, 0.0f, 1.0f);	

	// Create and load shaders
	GLuint programID = 0;
	try {		
		// Load vertex shader code and fragment shader code
		string vertexCode = readFileToString("./shaders/Assign03/Basic.vs");
		string fragCode = readFileToString("./shaders/Assign03/Basic.fs");

		// Print out shader code, just to check
		if(DEBUG_MODE) printShaderCode(vertexCode, fragCode);

		// Create shader program from code
		programID = initShaderProgramFromSource(vertexCode, fragCode);
	}
	catch (exception e) {		
		// Close program
		cleanupGLFW(window);
		exit(EXIT_FAILURE);
	}

	// Create simple quad 
	
	Mesh m;
	//createSimplePentagon(m);

	// Create OpenGL mesh (VAO) from data
	MeshGL mgl;
	createMeshGL(m, mgl);
	
	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    Mesh m;
    extractMeshData(scene->mMeshes[i], m);
    MeshGL mgl;
    createMeshGL(m, mgl);
    meshes.push_back(mgl);
}

	while (!glfwWindowShouldClose(window)) {
		// Set viewport size
		int fwidth, fheight;
		glfwGetFramebufferSize(window, &fwidth, &fheight);
		glViewport(0, 0, fwidth, fheight);

		// Clear the framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use shader program
		glUseProgram(programID);

		// Draw object
		//drawMesh(mgl);	

		for (auto &mesh : meshes) {
        	drawMesh(mesh);
    	}
		// Swap buffers and poll for window events		
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Sleep for 15 ms
		this_thread::sleep_for(chrono::milliseconds(15));
	}

	// Clean up mesh
	//cleanupMesh(mgl);

	for (auto &mesh : meshes) {
    cleanupMesh(mesh);
	}	

	// Clean up shader programs
	glUseProgram(0);
	glDeleteProgram(programID);
		
	// Destroy window and stop GLFW
	cleanupGLFW(window);

	return 0;
}
