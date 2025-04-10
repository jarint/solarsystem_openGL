#version 330 core

in vec3 fragment_position;
in vec2 texture_coordinates;
in vec3 normal_vector;

uniform vec3 camera_position;
uniform sampler2D texture_sampler;

out vec4 output_color;

void main()
{
    vec4 texture_color = texture(texture_sampler, texture_coordinates);
    vec3 unit_normal = normalize(normal_vector);
    vec3 light_direction = normalize(-fragment_position); // assume sun at origin

    float ambient_strength = 0.01;
    float diffuse_intensity = max(dot(light_direction, unit_normal), 0.0);
    float specular_strength = 0.5;

    vec3 view_direction = normalize(camera_position - fragment_position);
    vec3 reflection_direction = reflect(-light_direction, unit_normal);
    float specular_intensity = pow(max(dot(view_direction, reflection_direction), 0.0), 8);

    float lighting = ambient_strength + diffuse_intensity + specular_strength * specular_intensity;
    vec3 final_color = texture_color.rgb * lighting;

    output_color = vec4(final_color, texture_color.a);
}
