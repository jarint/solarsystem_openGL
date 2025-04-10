#pragma once

#include "Geometry.h"
#include "Texture.h"

#include <glm/glm.hpp>
#include <memory>
#include <string>

class CelestialBody
{
public:
    CelestialBody();
    void initialize_geometry(float radius, int slices, int stacks);
    void set_texture(std::string const& texture_path);
    void set_parent(CelestialBody* parent_body);
    void set_geometry(const CPU_Geometry& geom);

    void update_transform(float delta_time);
    void reset_rotation();
    void compute_model_matrix();

    [[nodiscard]] glm::mat4 const& get_model_matrix() const;
    void upload_to_gpu();
    Texture& get_texture();
    GPU_Geometry& get_geometry();

public:
    float scale = 1.0f;
    float axis_tilt = 0.0f;
    float orbit_tilt = 0.0f;
    float orbit_radius = 0.0f;

    float axis_rotation_speed = 1.0f;
    float orbit_rotation_speed = 1.0f;

private:
    GPU_Geometry gpu_geometry;
    CPU_Geometry cpu_geometry;
    Texture texture;

    float axis_rotation_angle = 0.0f;
    float orbit_rotation_angle = 0.0f;

    glm::mat4 model_matrix = glm::mat4(1.0f);
    CelestialBody* parent = nullptr;
};
