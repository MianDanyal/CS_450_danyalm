#version 430 core
// Change to 410 for macOS

layout(location=0) out vec4 out_color;

in vec4 vertexColor; 
in vec4 interPos; 
in vec3 interNormal; 
 
struct PointLight {
    vec4 pos;
    vec4 color;
};

uniform PointLight light;

void main()
{	
	vec3 N = normalize(interNormal);
	vec3 lightDir = normalize(vec3(light.pos - interPos));

	float diffuseCoefficient = max(0.0, dot(N, lightDir));
	vec3 diffColor = max(0.0, diffuseCoefficient) * vec3(vertexColor) * vec3(light.color);

	float shininess = 10.0;

	vec3 R = reflect(-lightDir, N);
	vec3 V = normalize(-vec3(interPos));

	float specularCoefficient = pow(max(dot(R, V), 0.0), shininess) * diffuseCoefficient;
	vec3 specularColor = specularCoefficient * vec3(light.color);
	
	out_color = vec4(diffColor + specularColor, 1.0);

}
