// VERTEX SHADER
#version 330

// Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixModelView;
uniform mat4 matrixView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

// Input variables
in vec3 aVertex;
in vec3 aNormal;
in vec3 aTangent;
in vec3 aBiTangent;
in vec2 aTexCoord;

// Output variables
out vec4 color;
out vec4 position;
out mat3 matrixTangent;
out vec2 texCoord0;

// Normal
vec3 normal;

// Fog variables
out float fogFactor;
uniform float fogDensity;

// Light functions
// Ambient light
struct AMBIENT {
	vec3 color;
};
uniform AMBIENT lightAmbient;

vec4 AmbientLight(AMBIENT light) {
	return vec4(materialAmbient * light.color, 1);
};

// Directional light
struct DIRECTIONAL {
	vec3 direction;
	vec3 diffuse;
};
uniform DIRECTIONAL lightDir;

vec4 DirectionalLight(DIRECTIONAL light) {
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normal, L);

	if (NdotL > 0) color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color;
};

void main(void)
{
	// calculate the position
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate the normal
	normal = normalize(mat3(matrixModelView) * aNormal);

	// calculate the local system transformation
	vec3 tangent = normalize(mat3(matrixModelView) * aTangent);
	vec3 biTangent = normalize(mat3(matrixModelView) * aBiTangent);
	matrixTangent = mat3(tangent, biTangent, normal);

	// Calculate lights
	color = vec4(0, 0, 0, 1);
	color += AmbientLight(lightAmbient);
	color += DirectionalLight(lightDir);

	// Create a fog effect
	fogFactor = exp2(-fogDensity * length(position));	

	// Calculate texturing
	texCoord0 = aTexCoord;
}