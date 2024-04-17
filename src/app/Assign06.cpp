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

struct PointLight {
    glm::vec4 pos;
    glm::vec4 color;
};


PointLight light;

float rotAngle = 0.0f;

glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 1.0f); 
glm::vec3 cameraLookAt = glm::vec3(0.0f, 0.0f, 0.0f); 
glm::vec2 lastMousePos; 

glm::mat4 makeRotateZ(glm::vec3 offset) {

    glm::mat4 translate1 = glm::translate(glm::mat4(1.0f), -offset);
    glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 translate2 = glm::translate(glm::mat4(1.0f), offset);

    return translate2 * rotateZ * translate1;
}

glm::mat4 makeLocalRotate(glm::vec3 offset, glm::vec3 axis, float angle) {
    angle = glm::radians(angle);

    glm::mat4 translate1 = glm::translate(glm::mat4(1.0f), -offset);
    glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), angle, axis);
    glm::mat4 translate2 = glm::translate(glm::mat4(1.0f), offset);

    return translate2 * rotate * translate1;
}

static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos) {
    glm::vec2 relMouse = -(glm::vec2(xpos, ypos) - lastMousePos);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    if (width > 0 && height > 0) {
        relMouse.x /= width;
        relMouse.y /= height;

        glm::vec3 cameraDirection = cameraLookAt - cameraPosition;

        glm::vec4 lookAtVec = glm::vec4(cameraLookAt, 1.0);
        //glm::vec4 eyeVec = glm::vec4(cameraPosition, 1.0);
        glm::mat4 rotateY = makeLocalRotate(cameraPosition, glm::vec3(0, 1, 0), 30.0f * relMouse.x);
        lookAtVec = rotateY * lookAtVec;
        //eyeVec = rotateY * eyeVec;
        cameraLookAt = glm::vec3(lookAtVec);
        //cameraPosition = glm::vec3(eyeVec);

        glm::vec3 xAxis = glm::cross(cameraDirection, glm::vec3(0, 1, 0));
        glm::mat4 rotateX = makeLocalRotate(cameraPosition, xAxis, 30.0f * relMouse.y);
        lookAtVec = rotateX * glm::vec4(cameraLookAt, 1.0);
        //eyeVec = rotateX * glm::vec4(cameraPosition, 1.0);
        cameraLookAt = glm::vec3(lookAtVec);
        //cameraPosition = glm::vec3(eyeVec);

        lastMousePos = glm::vec2(xpos, ypos);
    }
}

void renderScene(vector<MeshGL> &allMeshes, aiNode *node, glm::mat4 parentMat, GLint modelMatLoc, int level, GLint normMatLoc, glm::mat4 viewMat) {


    aiMatrix4x4 aiMat = node->mTransformation;
    glm::mat4 nodeT;
    aiMatToGLM4(aiMat, nodeT);
    glm::mat4 modelMat = parentMat*nodeT;
    glm::vec3 pos = glm::vec3(modelMat[3]);
    glm::mat4 R = makeRotateZ(pos);
    glm::mat4 tmpModel = R * modelMat;
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(viewMat * tmpModel)));

	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(tmpModel));

    glUniformMatrix3fv(normMatLoc, 1, false, glm::value_ptr(normalMatrix));


    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        int index = node->mMeshes[i];
        drawMesh(allMeshes.at(index));
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        renderScene(allMeshes, node->mChildren[i], modelMat, modelMatLoc, level + 1, normMatLoc, viewMat);
    }
	
}


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    
		glm::vec3 cameraDirection = glm::normalize(cameraLookAt - cameraPosition);
        glm::vec3 xAxis = glm::normalize(glm::cross(cameraDirection, glm::vec3(0, 1, 0)));
		float speed = 0.1f;

		switch (key) {
            case GLFW_KEY_1:
                light.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // White
                break;
            case GLFW_KEY_2:
                light.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
                break;
            case GLFW_KEY_3:
                light.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
                break;
            case GLFW_KEY_4:
                light.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // Blue
                break;
            default:
                break;
        }


        if (key == GLFW_KEY_ESCAPE) {
           
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        else if (key == GLFW_KEY_J) {
          
            rotAngle += 1.0f;
        }
        else if (key == GLFW_KEY_K) {
       
            rotAngle -= 1.0f;
        }
		else if (key == GLFW_KEY_W) {
            cameraPosition += cameraDirection * speed;
            cameraLookAt += cameraDirection * speed;
        }
        else if (key == GLFW_KEY_S) {
            cameraPosition -= cameraDirection * speed;
            cameraLookAt -= cameraDirection * speed;
        }
        else if (key == GLFW_KEY_D) {
            cameraPosition += xAxis * speed;
            cameraLookAt += xAxis * speed;
        }
        else if (key == GLFW_KEY_A) {
            cameraPosition -= xAxis * speed;
            cameraLookAt -= xAxis * speed;
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
			vertex.color = glm::vec4(1.0, 1.0, 0.0, 1.0);
			vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

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


	GLFWwindow* window = setupGLFW("Assign06: danyalm", 4, 3, 800, 800, DEBUG_MODE);

	setupGLEW(window);

	checkOpenGLVersion();

	if(DEBUG_MODE) checkAndSetupOpenGLDebugging();

	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);	

	GLuint programID = 0;
	try {		

		string vertexCode = readFileToString("./shaders/Assign06/Basic.vs");
		string fragCode = readFileToString("./shaders/Assign06/Basic.fs");

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

	double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    lastMousePos = glm::vec2(mx, my);

	glfwSetCursorPosCallback(window, mouse_position_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	PointLight light;
    light.pos = glm::vec4(0.5, 0.5, 0.5, 1.0);
    light.color = glm::vec4(1.0, 1.0, 1.0, 1.0);
    GLint lightPosLoc = glGetUniformLocation(programID, "light.pos");
    GLint lightColorLoc = glGetUniformLocation(programID, "light.color");
    GLint normMatLoc = glGetUniformLocation(programID, "normMat");
	
	GLuint viewMatLoc = glGetUniformLocation(programID, "viewMat");
	GLuint projMatLoc = glGetUniformLocation(programID, "projMat");
	while (!glfwWindowShouldClose(window)) {

		int fwidth, fheight;
		glfwGetFramebufferSize(window, &fwidth, &fheight);
		glViewport(0, 0, fwidth, fheight);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
		glUseProgram(programID);

    	glm::mat4 viewMat = glm::lookAt(cameraPosition, cameraLookAt, glm::vec3(0, 1, 0));
    	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
    	float aspectRatio = (fheight > 0) ? static_cast<float>(fwidth) / fheight : 1.0f;
    	glm::mat4 projMat = glm::perspective(glm::radians(90.0f), aspectRatio, 0.01f, 50.0f);
    	glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, glm::value_ptr(projMat));

		//drawMesh(mgl);	

		glm::vec4 lightPosView = viewMat * light.pos;
        glUniform4fv(lightPosLoc, 1, glm::value_ptr(lightPosView));
        glUniform4fv(lightColorLoc, 1, glm::value_ptr(light.color));
	
		renderScene(meshes, scene->mRootNode, glm::mat4(1.0), modelMatLoc, 0, normMatLoc, viewMat);

		glfwSwapBuffers(window);
		glfwPollEvents();

		
		this_thread::sleep_for(chrono::milliseconds(15));
	}


	//cleanupMesh(mgl);
	cout << projMatLoc << endl;
	cout << modelMatLoc << endl;
	cout << normMatLoc << endl;
	cout << lightColorLoc << endl;
	cout << lightPosLoc << endl;
	cout << viewMatLoc << endl;


	for (auto &mesh : meshes) {
    cleanupMesh(mesh);
	}	
	meshes.clear();


	glUseProgram(0);
	glDeleteProgram(programID);

	cleanupGLFW(window);

	return 0;
}


