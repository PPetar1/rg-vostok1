#version 330 core
out vec4 FragColor;

struct PointLight {
    vec3 position;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;

    float shininess;
};
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform PointLight pointLight;
uniform Material material;

uniform vec3 viewPosition;

uniform bool enable_fong = false;

// calculates the color when using a point light.
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    float spec = 0.0;
    if (enable_fong){
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    else{
        if (dot(lightDir, normal) > 0.0){

            vec3 halfwayDir = normalize(lightDir + viewDir);
            spec = pow(max(dot(normal, halfwayDir), 0.0), 3*material.shininess);
        }
    }

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords).xxx);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return vec4((ambient + diffuse + specular), texture(material.texture_diffuse1, TexCoords).a);
}

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec4 result = CalcPointLight(pointLight, normal, FragPos, viewDir);
    FragColor = result;
}