#include "TurnTableCamera.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "Log.h"

//======================================================================================================================

TurnTableCamera::TurnTableCamera(/*Transform & target*/)
    : TurnTableCamera(/*target, */Params{})
{
    // Temporary code, This is not the correct way to do it.
    _position = glm::vec3(0.0f, 0.0f, -5.0f);
    auto const translate = glm::translate(glm::identity<glm::mat4>(), _position);
    auto const rotationY = glm::rotate(glm::identity<glm::mat4>(), glm::radians(20.0f), Math::UpVec3);
    auto const rotationX = glm::rotate(glm::identity<glm::mat4>(), glm::radians(20.0f), Math::RightVec3);
    _viewMatrix = translate * rotationX * rotationY;
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

        // TODO Calculate the camera position and the view matrix here
    }
}

//======================================================================================================================

void TurnTableCamera::ChangeTheta(float const deltaTheta)
{
    // TODO
}

//======================================================================================================================

void TurnTableCamera::ChangePhi(float const deltaPhi)
{
    // TODO
}

//======================================================================================================================

void TurnTableCamera::ChangeRadius(float const deltaRadius)
{
    // TODO
}

//======================================================================================================================
