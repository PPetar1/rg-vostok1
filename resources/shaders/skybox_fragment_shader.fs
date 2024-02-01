#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = 0.25*texture(skybox, TexCoords).xxxw;//Taking just the red component looks much nicer, couldnt find better skybox online

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);//No need to add bloom to the stars since the immage already has it
}