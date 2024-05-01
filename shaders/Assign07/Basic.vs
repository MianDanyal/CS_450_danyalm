#version 430 core
// Change to 410 for macOS

layout(location=0) in vec3 position;
layout(location=1) in vec4 color;
layout(location=2) in vec3 normal;

uniform mat4 modelMat;
uniform mat4 viewMat; 
uniform mat4 projMat; 
uniform mat3 normMat;

out vec4 vertexColor;
out vec4 interPos;
out vec3 interNormal;
void main()
{		
	vec4 objPos = vec4(position, 1.0);

	vec4 worldPos = modelMat * objPos;
    vec4 viewPos = viewMat * worldPos; 
	gl_Position = projMat * viewPos; 
    interPos = viewPos; 

	vec3 worldNormal = normalize(normMat * normal);
    interNormal = worldNormal;
	
	vertexColor = color;
}
