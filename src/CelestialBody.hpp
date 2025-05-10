#pragma once

//------------------------------------------------------------------------------
// This file (and the associated .cpp) defines my CelestialBody class. It 
// modularizes representations of celestial bodies in the solar system. Each body 
// handles its own geometry, texture, transform logic, and optional parent 
// (for orbits). Everything gets bundled and uploaded to the GPU from here.
//------------------------------------------------------------------------------

#include "Geometry.h"
#include "Texture.h"

#include <glm/glm.hpp>
#include <memory>
#include <string>

class CelestialBody
{
public:
    CelestialBody();

    // just calls the sphere method in ShapeGenerator.cpp
    void initialize_geometry(float radius, int slices, int stacks);

    // give filepath for texture
    void set_texture(std::string const& texture_path);

    // link to a parent body
    void set_parent(CelestialBody* parent_body);

    // advance the animation based on delta_time
    void update_transform(float delta_time);

    // reset simulation to initial state
    void reset_rotation();

    // computes model matrix
    void compute_model_matrix();

    // gets the current model matrix
    [[nodiscard]] glm::mat4 const& get_model_matrix() const;

    // push cpu geometry to GPU
    void upload_to_gpu();

    // getter methods from texture and geometry
    Texture& get_texture();
    GPU_Geometry& get_geometry();

public:
    // transform parameters
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

    // used in hierarchical orbits
    CelestialBody* parent = nullptr;
};
