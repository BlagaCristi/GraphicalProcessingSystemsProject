#version 410 core

in vec3 normal;
in vec4 fragPosEye;
in vec4 fragPosLightSpace;
in vec2 fragTexCoords;
in vec3 fragPos;

out vec4 fColor;

//lighting
uniform	mat3 normalMatrix;
uniform mat3 lightDirMatrix;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

uniform vec3 flameLightPos;
uniform float constantPointLight1;
uniform float linearPointLight1;
uniform float quadraticPointLight1;
uniform float fogDensity;
uniform float lightOn;

float shininess = 64.0f;
struct directionalLightStruct{
	float ambient, specular;
	vec3 lightColor;
	vec3 lightPos;
};
uniform directionalLightStruct directionalLight;

struct pointLightStruct{
	vec3 ambient, specular, diffuse;
	float constant, linear, quadratic;
	vec3 lightPos,lightColor;
};
uniform pointLightStruct pointLight, pointLightLampPost;

struct spotLightStruct {
	float cutoff1, cutoff2, constant, linear, quadratic;
	vec3 lightPos, lightColor, ambient, specular, diffuse, direction;
};
uniform spotLightStruct spotLight;

float computeShadow()
{	
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
	float currentDepth = normalizedCoords.z;
	float bias = 0.005;
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, normalizedCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;

    return shadow;	
}

vec3 computeLightDirComponents()
{	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDirMatrix * directionalLight.lightPos);	

	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
		
	//compute ambient light
	ambient = directionalLight.ambient * directionalLight.lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * directionalLight.lightColor;
	
	//compute specular light
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	specular = directionalLight.specular * specCoeff * directionalLight.lightColor;

	float shadow = computeShadow();

	//modulate with diffuse map
	ambient *= vec3(texture(diffuseTexture, fragTexCoords));
	diffuse *= vec3(texture(diffuseTexture, fragTexCoords));
	//modulate woth specular map
	specular *= vec3(texture(specularTexture, fragTexCoords));

	return min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
}

vec3 computeLightPosComponents() {
	vec3 cameraPosEye = vec3(0.0f);
	vec3 lightDir = normalize(pointLight.lightPos - fragPos);
	vec3 normalEye = normalize(normalMatrix * normal);
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
	vec3 halfVector = normalize(lightDirN + viewDirN);

	float diff = max(dot(normal, lightDir), 0.0f);
	float spec = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	float distance = length(pointLight.lightPos - fragPos);
	float attenuation = 1.0f / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * distance * distance);
	vec3 ambient = pointLight.lightColor * pointLight.ambient * vec3(texture(diffuseTexture,fragTexCoords));
	vec3 diffuse = pointLight.lightColor * diff * pointLight.diffuse * vec3(texture(diffuseTexture,fragTexCoords));
	vec3 specular = pointLight.lightColor * spec * pointLight.specular* vec3(texture(specularTexture,fragTexCoords));
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	return (ambient + diffuse + specular);
}

vec3 computeLightPosLampPostComponents() {
	vec3 cameraPosEye = vec3(0.0f);
	vec3 lightDir = normalize(pointLightLampPost.lightPos - fragPos);
	vec3 normalEye = normalize(normalMatrix * normal);
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
	vec3 halfVector = normalize(lightDirN + viewDirN);

	float diff = max(dot(normal, lightDir), 0.0f);
	float spec = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	float distance = length(pointLightLampPost.lightPos - fragPos);
	float attenuation = 1.0f / (pointLightLampPost.constant + pointLightLampPost.linear * distance + pointLightLampPost.quadratic * distance * distance);
	vec3 ambient = pointLightLampPost.lightColor * pointLightLampPost.ambient * vec3(texture(diffuseTexture,fragTexCoords));
	vec3 diffuse = pointLightLampPost.lightColor * diff * pointLightLampPost.diffuse * vec3(texture(diffuseTexture,fragTexCoords));
	vec3 specular = pointLightLampPost.lightColor * spec * pointLightLampPost.specular* vec3(texture(specularTexture,fragTexCoords));
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	return (ambient + diffuse + specular);
}

vec3 computeLightSpotComponents() {
	vec3 cameraPosEye = vec3(0.0f);
	vec3 lightDir = normalize(spotLight.lightPos - fragPos);
	vec3 normalEye = normalize(normalMatrix * normal);
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
	vec3 halfVector = normalize(lightDirN + viewDirN);

	float diff = max(dot(normal, lightDir), 0.0f);
	float spec = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	float distance = length(spotLight.lightPos - fragPos);
	float attenuation = 1.0f / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * distance * distance);

	float theta = dot(lightDir, normalize(-spotLight.direction));
	float epsilon = spotLight.cutoff1 - spotLight.cutoff2;
	float intensity = clamp((theta - spotLight.cutoff2)/epsilon, 0.0, 1.0);

	vec3 ambient = spotLight.lightColor * spotLight.ambient * vec3(texture(diffuseTexture, fragTexCoords));
	vec3 diffuse = spotLight.lightColor * spotLight.diffuse * diff * vec3(texture(diffuseTexture, fragTexCoords));
	vec3 specular = spotLight.lightColor * spotLight.specular * spec * vec3(texture(specularTexture, fragTexCoords));
	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;
	
	return ambient + diffuse + specular;
}

float computeFog()
{
 float fragmentDistance = length(fragPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
	vec3 color1 = computeLightPosComponents();
	vec3 color2 = computeLightDirComponents();
	vec3 color3;
	if(lightOn == 1.0f) {
		color3 = computeLightSpotComponents();
	} else {
		color3 = vec3(0.0f);
	}
	vec3 color4 = computeLightPosLampPostComponents();
	//color2 = vec3(0.0f);
    //color3 = vec3(0.0f);
   // fColor = vec4(color, 1.0f);
    //fColor = vec4(o, 1.0f);

	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f,0.5f,0.5f,1.0f);
	fColor = fogColor*(1-fogFactor)+vec4(color1 + color2 + color3 + color4,1.0f)*fogFactor;
	//fColor = mix(fogColor, color, fogFactor);

}
