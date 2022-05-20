// FRAGMENT SHADER
#version 330

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

// View matrix
uniform mat4 matrixView;

// Samplers
uniform sampler2D texture0;
uniform sampler2D textureNormal;

// Normal
vec3 normal;

// Input variables
in vec4 position;
in vec4 color;
in vec2 texCoord0;
in mat3 matrixTangent;

// Output variables
out vec4 outColor;

// Fog variables
uniform vec3 fogColor;
in float fogFactor;

// Animated texture variables
uniform float time;
uniform float speedX, speedY;

// Point light
struct POINT {
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};

vec4 PointLight(POINT light) {
	vec4 color = vec4(0, 0, 0, 1);
	vec3 D = vec3(normalize(matrixView * vec4(light.position, 1) - position)).xyz;
	float NdotD = dot(normal, D);

	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-D, normal);
	float RdotV = dot(R, V);

	if (NdotD > 0) color += vec4(materialDiffuse * light.diffuse, 1) * NdotD;
	if (NdotD > 0 && RdotV > 0) color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);
	return color;
};

// Specular light
struct SPECULAR {
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};

vec4 SpecularLight(SPECULAR light) {
	vec4 color = vec4(0, 0, 0, 1);
	vec3 D = vec3(normalize(matrixView * vec4(light.position, 1) - position)).xyz;
	float NdotD = dot(normal, D);

	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-D, normal);
	float RdotV = dot(R, V);

	if (NdotD > 0 && RdotV > 0) color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);
	return color;
};
uniform SPECULAR lightPoint1, lightPoint2;

// Spotlight
struct SPOT {
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	vec3 direction;
	float cutoff;
	float attenuation;
};
uniform SPOT lightSpot;

vec4 SpotLight(SPOT light) {
	vec4 color = vec4(0, 0, 0, 1);
	vec3 L = vec3(normalize(matrixView * vec4(light.position, 1) - position)).xyz;
	float NdotL = dot(normal, L);

	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);

	if (NdotL > 0) color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	if (NdotL > 0 && RdotV > 0) color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	vec3 D = normalize(mat3(matrixView) * light.direction);
	float spotFactor = dot(-L, D);
	float angle = acos(spotFactor);
	if (angle <= light.cutoff) spotFactor = pow(spotFactor, light.attenuation);
	else spotFactor = 0;

	return spotFactor * color;
};

// Attenuated light
struct ATTENUATED {
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	float att_const, att_linear, att_quadratic;
};
uniform ATTENUATED lightAtt;

vec4 AttenuatedLight(ATTENUATED light) {
	vec4 color = vec4(0, 0, 0, 1);
	vec3 L = vec3(normalize(matrixView * vec4(light.position, 1) - position)).xyz;
	float NdotL = dot(normal, L);

	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);

	if (NdotL > 0) color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	if (NdotL > 0 && RdotV > 0) color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	float dist = length(matrixView * vec4(light.position, 1) - position);
	float att = 1 / (light.att_const + light.att_linear * dist + light.att_quadratic * dist * dist);

	return color * att;
};

// Rim light effect
struct RIM {
	vec3 diffuse;
};
uniform RIM lightRim;

vec4 RimLight(RIM light) {
	vec4 color = vec4(0, 0, 0, 1);
	vec3 V = normalize(-position.xyz);
	float rim = 1.0f - dot(V, normal);
	rim = smoothstep(0.6f, 1.0f, rim);
	color += vec4(materialDiffuse * light.diffuse * rim, 1);
	return color;
};

void main(void)
{
	outColor = color;

	// Normal
	normal = 2.0 * texture(textureNormal, texCoord0).xyz - vec3(1.0, 1.0, 1.0);
	normal = normalize(matrixTangent * normal);

	// Add lights
	outColor += SpecularLight(lightPoint1);
	outColor += SpecularLight(lightPoint2);
	outColor += SpotLight(lightSpot);
	outColor += AttenuatedLight(lightAtt);
	outColor += RimLight(lightRim);

	// Animate the texture
	float xScroll = speedX * time;
	float yScroll = speedY * time;

	// Texturing
	outColor *= texture(texture0, texCoord0 + vec2(xScroll, yScroll));

	// Fog
	outColor = mix(vec4(fogColor, 1), outColor, fogFactor);
}