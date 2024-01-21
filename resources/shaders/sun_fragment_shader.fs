#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;

void main()
{
    //simple shader that just returns the ambient value of lighting; used to render the sun
    FragColor = 3*vec4(vec3(texture(texture_diffuse1, TexCoords)), 1.0);
}