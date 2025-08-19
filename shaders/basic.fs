#version 330 core

struct Material {
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewD);

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform DirLight dirLight;

Material material;

void main() {
    vec3 Color = vec3(0.5f, 0.5f, 0.5f);

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    material.diffuse = vec3(Color);
    material.specular = vec3(0.8f, 0.8f, 0.8f);
    material.shininess = 32.0f;

    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    result += calcDirLight(dirLight, norm, viewDir);

    FragColor = vec4(result, 0.0f);
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewD) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewD, reflectDir), 0.0f), material.shininess);

    vec3 ambient = light.ambient * material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;

    return ambient + diffuse + specular;
}

