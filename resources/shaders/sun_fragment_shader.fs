#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;

void main()
{
    //simple shader that just returns the ambient value of lighting; used to render the sun
    FragColor = 50*vec4(vec3(texture(texture_diffuse1, TexCoords)), 1.0);
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
      BrightColor = FragColor;
    else
      BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}