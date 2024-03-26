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
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Utility.hpp"

using namespace std;

float rotAngle = 0.0f;

glm::mat4 makeRotateZ(glm::vec3 offset) {

    glm::mat4 translate1 = glm::translate(glm::mat4(1.0f), -offset);
    glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 translate2 = glm::translate(glm::mat4(1.0f), offset);

    return translate2 * rotateZ * translate1;
}

void renderScene(vector<MeshGL> &allMeshes, aiNode *node, glm::mat4 parentMat, GLint modelMatLoc, int level) {

    aiMatrix4x4 aiMat = node->mTransformation;

    glm::mat4 nodeT;
    aiMatToGLM4(aiMat, nodeT);

    glm::mat4 modelMat = parentMat*nodeT;

    glm::vec3 pos = glm::vec3(modelMat[3]);
    glm::mat4 R = makeRotateZ(pos);

    glm::mat4 tmpModel = R * modelMat;

    glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(tmpModel));

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        int index = node->mMeshes[i];
        drawMesh(allMeshes.at(index));
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        renderScene(allMeshes, node->mChildren[i], modelMat, modelMatLoc, level + 1);
    }
	
}


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    
        if (key == GLFW_KEY_ESCAPE) {
           
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        else if (key == GLFW_KEY_J) {
          
            rotAngle += 1.0f;
        }
        else if (key == GLFW_KEY_K) {
       
            rotAngle -= 1.0f;
        }
    }
}



void createSimpleQuad(Mesh &m) {

	m.vertices.clear();
	m.indices.clear();

	Vertex upperLeft, upperRight;
	Vertex lowerLeft, lowerRight;


	upperLeft.position = glm::vec3(-0.5, 0.5, 0.0);
	upperRight.position = glm::vec3(0.5, 0.5, 0.0);
	lowerLeft.position = glm::vec3(-0.5, -0.5, 0.0);
	lowerRight.position = glm::vec3(0.5, -0.5, 0.0);


	upperLeft.color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	upperRight.color = glm::vec4(0.0, 1.0, 0.0, 1.0);
	lowerLeft.color = glm::vec4(0.0, 0.0, 1.0, 1.0);
	lowerRight.color = glm::vec4(1.0, 1.0, 1.0, 1.0);

	m.vertices.push_back(upperLeft);
	m.vertices.push_back(upperRight);	
	m.vertices.push_back(lowerLeft);
	m.vertices.push_back(lowerRight);

	m.indices.push_back(0);
	m.indices.push_back(3);
	m.indices.push_back(1);

	m.indices.push_back(0);
	m.indices.push_back(2);
	m.indices.push_back(3);
}


void createSimplePentagon(Mesh &m) {

	m.vertices.clear();
	m.indices.clear();

	Vertex upperLeft, upperRight;
	Vertex lowerLeft, lowerRight;
	Vertex fifthVertex;

	upperLeft.position = glm::vec3(-0.5, 0.5, 0.0);
	upperRight.position = glm::vec3(0.5, 0.5, 0.0);
	lowerLeft.position = glm::vec3(-0.5, -0.5, 0.0);
	lowerRight.position = glm::vec3(0.5, -0.5, 0.0);
	fifthVertex.position = glm::vec3(0, 0.8, 0.0);

	upperLeft.color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	upperRight.color = glm::vec4(0.0, 1.0, 0.0, 1.0);
	lowerLeft.color = glm::vec4(0.0, 0.0, 1.0, 1.0);
	lowerRight.color = glm::vec4(1.0, 1.0, 1.0, 1.0);
	fifthVertex.color = glm::vec4(1.0, 1.0, 0.0, 1.0);

	m.vertices.push_back(upperLeft);
	m.vertices.push_back(upperRight);	
	m.vertices.push_back(lowerLeft);
	m.vertices.push_back(lowerRight);
	m.vertices.push_back(fifthVertex);

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

int main(int argc, char **argv) {
	
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


	GLFWwindow* window = setupGLFW("Assign04: danyalm", 4, 3, 800, 800, DEBUG_MODE);

	setupGLEW(window);

	checkOpenGLVersion();

	if(DEBUG_MODE) checkAndSetupOpenGLDebugging();

	glClearColor(0.8f, 0.9f, 0.0f, 1.0f);	

	GLuint programID = 0;
	try {		

		string vertexCode = readFileToString("./shaders/Assign04/Basic.vs");
		string fragCode = readFileToString("./shaders/Assign04/Basic.fs");

		if(DEBUG_MODE) printShaderCode(vertexCode, fragCode);

		programID = initShaderProgramFromSource(vertexCode, fragCode);

        
	}
	catch (exception e) {		

		cleanupGLFW(window);
		exit(EXIT_FAILURE);
	}

	Mesh m;
	//createSimplePentagon(m);


	MeshGL mgl;
	createMeshGL(m, mgl);

	glEnable(GL_DEPTH_TEST);

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    Mesh m;
    extractMeshData(scene->mMeshes[i], m);
    MeshGL mgl;
    createMeshGL(m, mgl);
    meshes.push_back(mgl);
}

	glfwSetKeyCallback(window, keyCallback);
	GLint modelMatLoc = glGetUniformLocation(programID, "modelMat");

	while (!glfwWindowShouldClose(window)) {

		int fwidth, fheight;
		glfwGetFramebufferSize(window, &fwidth, &fheight);
		glViewport(0, 0, fwidth, fheight);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
		glUseProgram(programID);


		//drawMesh(mgl);	
	
		renderScene(meshes, scene->mRootNode, glm::mat4(1.0), modelMatLoc, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();


		this_thread::sleep_for(chrono::milliseconds(15));
	}


	//cleanupMesh(mgl);

	for (auto &mesh : meshes) {
    cleanupMesh(mesh);
	}	


	glUseProgram(0);
	glDeleteProgram(programID);

	cleanupGLFW(window);

	return 0;
}
