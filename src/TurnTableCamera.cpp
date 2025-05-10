#include "TurnTableCamera.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "Log.h"

//======================================================================================================================

TurnTableCamera::TurnTableCamera(/*Transform & target*/)
    : TurnTableCamera(/*target, */Params{})
{
}

//======================================================================================================================

TurnTableCamera::TurnTableCamera(/*Transform & target, */Params const &params)
{
    // _target = &target;

    _distance = params.defaultDistance;
    _minDistance = params.minDistance;
    _maxDistance = params.maxDistance;
}

//======================================================================================================================

// void TurnTableCamera::ChangeTarget(Transform &target)
// {
//     _target = &target;
// }

//======================================================================================================================

glm::mat4 TurnTableCamera::ViewMatrix()
{
    UpdateViewMatrix();
    return _viewMatrix;
}

//======================================================================================================================

glm::vec3 TurnTableCamera::Position()
{
    UpdateViewMatrix();
    return _position;
}

//======================================================================================================================

void TurnTableCamera::UpdateViewMatrix()
{
    if (_isDirty == true)
    {
        _isDirty = false;

        auto const hRot = glm::rotate(glm::mat4(1.0f), _theta, Math::UpVec3);
        auto const vRot = glm::rotate(glm::mat4(1.0f), _phi, Math::RightVec3);

        _position = glm::vec3(hRot * vRot * glm::vec4{Math::ForwardVec3, 0.0f}) * _distance;

        _viewMatrix = glm::lookAt(_position, glm::vec3{}, Math::UpVec3);
    }
}

//======================================================================================================================

void TurnTableCamera::ChangeTheta(float const deltaTheta)
{
    auto newTheta = _theta + deltaTheta;
    if (newTheta != _theta)
    {
        _theta = newTheta;
        _isDirty = true;
    }
}

//======================================================================================================================

void TurnTableCamera::ChangePhi(float const deltaPhi)
{
    float const newPhi = std::clamp(
        _phi + deltaPhi,
        -glm::pi<float>() * 0.49f,
        glm::pi<float>() * 0.49f
    );
    if (newPhi != _phi)
    {
        _isDirty = true;
        _phi = newPhi;
    }
}

//======================================================================================================================

void TurnTableCamera::ChangeRadius(float const deltaRadius)
{
    float const newDistance =  std::clamp(_distance + deltaRadius, _minDistance, _maxDistance);
    if (newDistance != _distance)
    {
        _isDirty = true;
        _distance = newDistance;
    }
}

//======================================================================================================================
