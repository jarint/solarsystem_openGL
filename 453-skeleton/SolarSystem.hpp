#pragma once

#include "AssetPath.h"
#include "Geometry.h"
#include "InputManager.hpp"
#include "ShaderProgram.h"
#include "Texture.h"
#include "Time.hpp"
#include "TurnTableCamera.hpp"

class SolarSystem
{
public:

    explicit SolarSystem();

    ~SolarSystem();

    void Run();

private:

    void Update(float deltaTime);

    void Render();

    void UI();

    void PrepareUnitSphereGeometry();

    void OnResize(int width, int height);

    void OnMouseWheelChange(double xOffset, double yOffset) const;

    std::shared_ptr<AssetPath> mPath{};
    std::shared_ptr<Time> mTime{};
    std::unique_ptr<Window> mWindow;
    std::shared_ptr<InputManager> mInputManager{};

    std::unique_ptr<ShaderProgram> mBasicShader{};

    std::unique_ptr<GPU_Geometry> mUnitCubeGeometry;
    int mUnitCubeIndexCount{};

    std::unique_ptr<TurnTableCamera> mTurnTableCamera{};
    glm::dvec2 mPreviousCursorPosition {};
    bool mCursorPositionIsSetOnce = false;

    glm::mat4 mProjectionMatrix{};

    float mFovY = 120.0f;
    float mZNear = 0.01f;
    float mZFar = 100.0f;
    float mZoomSpeed = 20.0f;
    float mRotationSpeed = 0.25f;
};
