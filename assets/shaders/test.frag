#version 330 core

in vec3 fragment_position;
in vec2 texture_coordinates;
in vec3 normal_vector;

uniform sampler2D texture_sampler;

out vec4 output_color;

void main()
{
    output_color = texture(texture_sampler, texture_coordinates);
}
