#version 330 core

// input data : sent from main program
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexColor;
layout (location = 2) in vec3 normal;
uniform mat4 MVP;
uniform mat4 model;
// output data : used by fragment shader
out vec3 fragColor;
out vec3 Normal;
out vec3 FragPos;
void main ()
{
    vec4 v = vec4(vertexPosition, 1.0f); // Transform an homogeneous 4D vector
    fragColor = vertexColor;
    FragPos = vec3(model*v);
    Normal  = mat3(transpose(inverse(model)))*normal;
    //Normal = normal;
    gl_Position = MVP * v;
}
