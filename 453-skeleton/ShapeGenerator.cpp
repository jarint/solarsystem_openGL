//
// Created by moham on 3/15/2025.
//

#include "ShapeGenerator.hpp"

#include <glm/ext/matrix_transform.hpp>

//======================================================================================================================

// CPU_Geometry ShapeGenerator::Sphere(float const radius, int const slices, int const stacks)
// {
//     CPU_Geometry geom{};
//     // TODO
//     return geom;
// }

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
