#version 430 core
// Change to 410 for macOS

layout(location=0) in vec3 position;
layout(location=1) in vec4 color;

out vec4 vertexColor;
uniform mat4 modelMat;
uniform mat4 viewMat; 
uniform mat4 projMat; 
void main()
{		
	// Get position of vertex (object space)
	vec4 objPos = vec4(position, 1.0);

	// For now, just pass along vertex position (no transformations)
	vec4 worldPos = modelMat * objPos;
    vec4 viewPos = viewMat * worldPos; 
    gl_Position = projMat * viewPos; 

	// Output per-vertex color
	vertexColor = color;
}
