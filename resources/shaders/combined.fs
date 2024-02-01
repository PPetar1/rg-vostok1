#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform bool HDR;
uniform float exposure;

void main()//code just copied from learnopengl but really no changes are necessary
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    if(bloom)
        hdrColor += bloomColor; // additive blending
    // tone mapping
    vec3 result = hdrColor;
    if (HDR){
        result = vec3(1.0) - exp(-result * exposure);
    }
    // gamma correction
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}