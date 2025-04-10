//
// Created by moham on 3/15/2025.
//

#include "ShapeGenerator.hpp"

#include <glm/ext/matrix_transform.hpp>

#include <iostream>

//======================================================================================================================

CPU_Geometry ShapeGenerator::Sphere(float const radius, int const slices, int const stacks)
{
    CPU_Geometry geometry{};

    float phi_step = glm::pi<float>() / static_cast<float>(stacks);
    float theta_step = glm::two_pi<float>() / static_cast<float>(slices);

    for (int stack_index = 0; stack_index < stacks; ++stack_index)
    {
        float phi = stack_index * phi_step;
        float next_phi = (stack_index + 1) * phi_step;

        for (int slice_index = 0; slice_index < slices; ++slice_index)
        {
            float theta = slice_index * theta_step;
            float next_theta = (slice_index + 1) * theta_step;

            glm::vec3 vertex_1 = glm::vec3(
                -cos(theta) * sin(phi),
                -cos(phi),
                sin(theta) * sin(phi)
            ) * radius;

            glm::vec3 vertex_2 = glm::vec3(
                -cos(next_theta) * sin(phi),
                -cos(phi),
                sin(next_theta) * sin(phi)
            ) * radius;

            glm::vec3 vertex_3 = glm::vec3(
                -cos(next_theta) * sin(next_phi),
                -cos(next_phi),
                sin(next_theta) * sin(next_phi)
            ) * radius;

            glm::vec3 vertex_4 = glm::vec3(
                -cos(theta) * sin(next_phi),
                -cos(next_phi),
                sin(theta) * sin(next_phi)
            ) * radius;

            // First triangle
            geometry.positions.push_back(vertex_1);
            geometry.positions.push_back(vertex_2);
            geometry.positions.push_back(vertex_3);

            // Second triangle
            geometry.positions.push_back(vertex_1);
            geometry.positions.push_back(vertex_3);
            geometry.positions.push_back(vertex_4);

            // Normals
            geometry.normals.push_back(vertex_1);
            geometry.normals.push_back(vertex_2);
            geometry.normals.push_back(vertex_3);
            geometry.normals.push_back(vertex_1);
            geometry.normals.push_back(vertex_3);
            geometry.normals.push_back(vertex_4);

            // Texture coordinates
            geometry.uvs.push_back(glm::vec2(theta / glm::two_pi<float>(), phi / glm::pi<float>()));
            geometry.uvs.push_back(glm::vec2(next_theta / glm::two_pi<float>(), phi / glm::pi<float>()));
            geometry.uvs.push_back(glm::vec2(next_theta / glm::two_pi<float>(), next_phi / glm::pi<float>()));

            geometry.uvs.push_back(glm::vec2(theta / glm::two_pi<float>(), phi / glm::pi<float>()));
            geometry.uvs.push_back(glm::vec2(next_theta / glm::two_pi<float>(), next_phi / glm::pi<float>()));
            geometry.uvs.push_back(glm::vec2(theta / glm::two_pi<float>(), next_phi / glm::pi<float>()));
        }
    }

    // Fill dummy white color to satisfy GPU_Geometry::Update assertions
    geometry.colors.resize(geometry.positions.size(), glm::vec3(1.0f, 1.0f, 1.0f));

    return geometry;
}

//======================================================================================================================

static void colouredTriangles(CPU_Geometry &geom, glm::vec3 col);
static void positiveZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void positiveXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void negativeZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void negativeXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void positiveYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void negativeYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);

CPU_Geometry ShapeGenerator::UnitCube()
{
    std::vector<glm::vec3> originQuad{};
    originQuad.emplace_back(-0.5, 0.5, 0.0); // top-left
    originQuad.emplace_back(-0.5, -0.5, 0.0); // bottom-left
    originQuad.emplace_back(0.5, 0.5, 0.0); // top-right

    originQuad.emplace_back(-0.5, -0.5, 0.0); // bottom-left
    originQuad.emplace_back(0.5, -0.5, 0.0); // bottom-right
    originQuad.emplace_back(0.5, 0.5, 0.0); // top-right

    CPU_Geometry square{};

    positiveZFace(originQuad, square);
    colouredTriangles(square, {1.f, 1.f, 0.f});

    positiveXFace(originQuad, square);
    colouredTriangles(square, {1.f, 0.f, 0.f});

    negativeZFace(originQuad, square);
    colouredTriangles(square, {0.f, 1.f, 0.f});

    negativeXFace(originQuad, square);
    colouredTriangles(square, {0.f, 0.f, 1.f});

    positiveYFace(originQuad, square);
    colouredTriangles(square, {1.f, 0.f, 1.f});

    negativeYFace(originQuad, square);
    colouredTriangles(square, {0.f, 1.f, 1.f});

    return square;
}

void colouredTriangles(CPU_Geometry &geom, glm::vec3 col)
{
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
}

void positiveZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.5));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(T * glm::vec4((*i), 1.0));
    }
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
}

void positiveXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
}

void negativeZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
}

void negativeXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
}

void positiveYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
}

void negativeYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
}

//======================================================================================================================
