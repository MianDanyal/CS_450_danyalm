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
uniform float metallic;
uniform float roughness;
const float PI = 3.14159265359;

vec3 getFresnelAtAngleZero(vec3 albedo, float metallic)
{
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic); 
    return F0;
}

vec3 getFresnel(vec3 F0, vec3 L, vec3 H)
{
    float cosAngle = max(0.0, dot(L, H));
    return F0 + (1.0 - F0) * pow(1.0 - cosAngle, 5.0);
}

float getNDF(vec3 H, vec3 N, float roughness)
{
    float alpha = roughness * roughness;
    float cosThetaH = dot(H, N);
    float cosThetaH2 = cosThetaH * cosThetaH;
    float alpha2 = alpha * alpha;

    float nom = alpha2;
    float denom = (cosThetaH2 * (alpha2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / denom;
}

float getSchlickGeo(vec3 B, vec3 N, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return dot(N, B) / (dot(N, B) * (1.0 - k) + k);
}

float getGF(vec3 L, vec3 V, vec3 N, float roughness)
{
    float GL = getSchlickGeo(L, N, roughness);
    float GV = getSchlickGeo(V, N, roughness);
    return GL * GV;
}

void main()
{
    vec3 N = normalize(interNormal);
	vec3 lightDir = normalize(vec3(light.pos - interPos));
	vec3 viewDir = normalize(-vec3(interPos));

	vec3 F0 = getFresnelAtAngleZero(vec3(vertexColor), metallic); 
    vec3 H = normalize(lightDir + viewDir); 
    vec3 F = getFresnel(F0, lightDir, H); 

	float diffuseCoefficient = max(0.0, dot(N, lightDir));
	vec3 diffColor = max(0.0, diffuseCoefficient) * vec3(vertexColor) * vec3(light.color);

	vec3 kS = F;

	vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - metallic); 
    kD *= vec3(vertexColor) / PI;

	// Calculate NDF using getNDF(H, N, roughness)
    float NDF = getNDF(H, N, roughness);

    // Calculate G using getGF(L, V, N, roughness)
    float G = getGF(lightDir, viewDir, N, roughness);

    // Calculate specular reflection
    kS *= NDF * G;
    kS /= (4.0 * max(0.0, dot(N, lightDir)) * max(0.0, dot(N, viewDir))) + 0.0001;

	vec3 finalColor = (kD + kS) * vec3(light.color) * max(0.0, dot(N, lightDir));

	float shininess = 10.0;

	vec3 R = reflect(-lightDir, N);
	vec3 V = normalize(-vec3(interPos));

	float specularCoefficient = pow(max(dot(R, V), 0.0), shininess) * diffuseCoefficient;
	vec3 specularColor = specularCoefficient * vec3(light.color);
	
	out_color = vec4(finalColor, 1.0);
}

