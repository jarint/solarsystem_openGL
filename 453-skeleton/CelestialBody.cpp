#include "CelestialBody.hpp"
#include "ShapeGenerator.hpp"
#include "AssetPath.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

CelestialBody::CelestialBody()
    : texture(AssetPath::Instance()->Get("textures/2k_moon.jpg"), GL_NEAREST)
{}

void CelestialBody::initialize_geometry(float radius, int slices, int stacks)
{
    cpu_geometry = ShapeGenerator::Sphere(radius, slices, stacks);
}

void CelestialBody::set_texture(std::string const& texture_path)
{
    texture = Texture(AssetPath::Instance()->Get(texture_path), GL_NEAREST);
}

void CelestialBody::set_parent(CelestialBody* parent_body)
{
    parent = parent_body;
}

void CelestialBody::set_geometry(const CPU_Geometry& geom)
{
    cpu_geometry = geom;
    gpu_geometry.Update(cpu_geometry);
}

void CelestialBody::upload_to_gpu()
{
    gpu_geometry.Update(cpu_geometry);
}

void CelestialBody::update_transform(float delta_time)
{
    axis_rotation_angle = std::fmod(axis_rotation_angle + axis_rotation_speed * delta_time, glm::two_pi<float>());
    orbit_rotation_angle = std::fmod(orbit_rotation_angle + orbit_rotation_speed * delta_time, glm::two_pi<float>());
    compute_model_matrix();
}

void CelestialBody::reset_rotation()
{
    axis_rotation_angle = 0.0f;
    orbit_rotation_angle = 0.0f;
    compute_model_matrix();
}

void CelestialBody::compute_model_matrix()
{
    glm::mat4 parent_matrix = parent ? parent->model_matrix : glm::mat4(1.0f);
    float parent_axis_rotation = parent ? parent->axis_rotation_angle : 0.0f;

    glm::vec3 orbit_position = glm::vec3(
        glm::rotate(glm::mat4(1.0f), orbit_rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(orbit_radius, 0.0f, 0.0f, 1.0f)
    );

    model_matrix = parent_matrix *
                   glm::rotate(glm::mat4(1.0f), -parent_axis_rotation, glm::vec3(0.0f, 1.0f, 0.0f)) *
                   glm::rotate(glm::mat4(1.0f), orbit_tilt, glm::vec3(1.0f, 0.0f, 0.0f)) *
                   glm::translate(glm::mat4(1.0f), orbit_position) *
                   glm::rotate(glm::mat4(1.0f), axis_tilt, glm::vec3(1.0f, 0.0f, 0.0f)) *
                   glm::rotate(glm::mat4(1.0f), axis_rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f)) *
                   glm::scale(glm::mat4(1.0f), glm::vec3(scale));
}

glm::mat4 const& CelestialBody::get_model_matrix() const
{
    return model_matrix;
}

Texture& CelestialBody::get_texture() {
    return texture;
}

GPU_Geometry& CelestialBody::get_geometry() {
    return gpu_geometry;
}
