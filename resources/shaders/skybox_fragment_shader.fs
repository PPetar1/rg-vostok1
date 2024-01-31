#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    //FragColor = texture(skybox, TexCoords);
    //FragColor = pow(texture(skybox, TexCoords), vec4(4.0,4.0,4.0,1.0));
    FragColor = texture(skybox, TexCoords).xxxw;//Taking just the red component looks much nicer
                                                  //It still doesnt look good but i couldnt find another skybox so it has to do
}