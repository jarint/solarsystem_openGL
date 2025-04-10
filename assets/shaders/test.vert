#version 330 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 vertex_texture_coordinates;
layout (location = 2) in vec3 vertex_normal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec3 fragment_position;
out vec2 texture_coordinates;
out vec3 normal_vector;

void main()
{
    vec4 model_pos = model_matrix * vec4(vertex_position, 1.0);
    fragment_position = vec3(model_pos);                    
    texture_coordinates = vertex_texture_coordinates;       
    normal_vector = vec3(model_matrix * vec4(vertex_normal, 0.0)); 

    gl_Position = projection_matrix * view_matrix * model_pos;     
}
