#pragma once

#include "Math.hpp"

class TurnTableCamera
{
public:

    struct Params
    {
        float defaultRadius = 0.0f;
        float defaultTheta = 0.0f;
        float defaultPhi = 0.0f;

        float defaultDistance = 5.0f;
        float minDistance = 1.0f;
        float maxDistance = 20.0f;
    };

    // For the bonus camera needs to be able to follow a target

    // Angles are in radians
    explicit TurnTableCamera(/*Transform & target*/);

    // Angles are in radians
    explicit TurnTableCamera(/*Transform & target, */const Params &params);

    // void ChangeTarget(Transform & target);

    void ChangeTheta(float deltaTheta);

    void ChangePhi(float deltaPhi);

    void ChangeRadius(float deltaRadius);

    [[nodiscard]]
    glm::mat4 ViewMatrix();

    [[nodiscard]]
    glm::vec3 Position();

private:

    void UpdateViewMatrix();

    // Transform * _target;

    float _distance{};
    float _minDistance{};
    float _maxDistance{};

    float _theta {};
    float _phi {};

    bool _isDirty = true;

    glm::mat4 _viewMatrix {};
    glm::vec3 _position {};
};
