#version 410 core

out vec4 fColor;
uniform sampler2D specularTexture;
in vec2 passTexture;

void main() 
{    
    fColor = texture(specularTexture, passTexture);
}
