#include "SolarSystem.hpp"

#include <filesystem>

#include "GLDebug.h"
#include "Log.h"

#include <glm/gtc/type_ptr.hpp>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include "ShapeGenerator.hpp"

// Step 1: Create a sphere with positions, indices, and uv values
// Step 2: Create the solar system with sun, earth and moon
// Step 3: Add cube map texture for background and

//======================================================================================================================

SolarSystem::SolarSystem()
{
    mPath = AssetPath::Instance();
    mTime = Time::Instance();

    glfwWindowHint(GLFW_SAMPLES, 32);
    mWindow = std::make_unique<Window>(800, 800, "Solar system");

    // Standard ImGui/GLFW middleware
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(mWindow->getGLFWwindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330 core");


    glEnable(GL_MULTISAMPLE);
    int samples = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    Log::info("MSAA Samples: {0}", samples);

    GLDebug::enable(); // ON Submission you may comments this out to avoid unnecessary prints to the console

    mInputManager = std::make_shared<InputManager>(
        [this](int const width, int const height)->void {OnResize(width, height);},
        [this](double const xOffset, double const yOffset)->void {OnMouseWheelChange(xOffset, yOffset);}
    );

    mWindow->setCallbacks(mInputManager);

    // phong shader
    phong_shader = std::make_unique<ShaderProgram>(
        mPath->Get("shaders/test.vert"),
        mPath->Get("shaders/phong.frag")
    );

    // provided basic shader
    mBasicShader = std::make_unique<ShaderProgram>(
        mPath->Get("shaders/test.vert"),
        mPath->Get("shaders/test.frag")
    );

    TurnTableCamera::Params cam_params;
    cam_params.defaultDistance = 20.0f;
    cam_params.minDistance = 5.0f;
    cam_params.maxDistance = 200.0f;

    mTurnTableCamera = std::make_unique<TurnTableCamera>(cam_params);

    PrepareSphereGeometry();
}

//======================================================================================================================

SolarSystem::~SolarSystem()
{
    // ImGui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

//======================================================================================================================

void SolarSystem::Run()
{
    while (mWindow->shouldClose() == false)
    {
        glfwPollEvents(); // Propagate events to the callback class

        mTime->Update();
        Update(mTime->DeltaTimeSec());

        glEnable(GL_FRAMEBUFFER_SRGB); // Expect Colour to be encoded in sRGB standard (as opposed to RGB)

        // https://www.viewsonic.com/library/creative-work/srgb-vs-adobe-rgb-which-one-to-use/
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear render screen (all zero) and depth (all max depth)

        // macOS window size is different than framebuffer. 
        // I think this still works with linux/windows, but if there are window sizing issues
        // they probably arise here.
        int framebuffer_width, framebuffer_height;
        glfwGetFramebufferSize(mWindow->getGLFWwindow(), &framebuffer_width, &framebuffer_height);
        glViewport(0, 0, framebuffer_width, framebuffer_height);

        Render();

        glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui (if used)

        // Starting the new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        UI();

        ImGui::Render(); // Render the ImGui window
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Some middleware thing

        mWindow->swapBuffers(); // Swap the buffers while displaying the previous
    }
}

//======================================================================================================================

// Handle user input and animations
void SolarSystem::Update(float const deltaTime)
{
    auto const cursorPosition = mInputManager->CursorPosition();

    if (mCursorPositionIsSetOnce == true)
    {
        if (mInputManager->IsMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT) == true)
        {
            auto const deltaPosition = cursorPosition - mPreviousCursorPosition;
            mTurnTableCamera->ChangeTheta(-static_cast<float>(deltaPosition.x) * mRotationSpeed * deltaTime);
            mTurnTableCamera->ChangePhi(-static_cast<float>(deltaPosition.y) * mRotationSpeed * deltaTime);
        }
    }

    mCursorPositionIsSetOnce = true;
    mPreviousCursorPosition = cursorPosition;

    // press space bar to toggle pausing on and off 
    static bool space_was_down_last_frame = false;
    bool space_is_down_now = mInputManager->IsKeyboardButtonDown(GLFW_KEY_SPACE);

    if (space_is_down_now && !space_was_down_last_frame) {
        paused = !paused;
    }
    space_was_down_last_frame = space_is_down_now;

    // reset the scene with 'R'
    if (mInputManager->IsKeyboardButtonDown(GLFW_KEY_R)) {
        reset = true;
    }

    // increase the animation speed up to a limit with 'W'
    if (mInputManager->IsKeyboardButtonDown(GLFW_KEY_W)) {
        animation_speed = std::min(animation_speed + deltaTime * 5.0f, 20.0f);
        paused = false;
    }

    // decrease the animation speed to a limit with 'S'
    if (mInputManager->IsKeyboardButtonDown(GLFW_KEY_S)) {
        animation_speed = std::max(animation_speed - deltaTime * 5.0f, 0.1f);
        paused = false;
    }

    // apply the changes to each celestial body
    for (const auto& body_ptr : m_bodies)
    {
        CelestialBody* body = body_ptr.get();
        if (reset) {
            body->reset_rotation();
        }
        else if (!paused) {
            body->update_transform(deltaTime * animation_speed);
        }
    }

    // needed to reset the reset flag
    if (!mInputManager->IsKeyboardButtonDown(GLFW_KEY_R)) {
        reset = false;
    }
}


//======================================================================================================================

// Renders all the celestial bodies in the scene with either phong/basic shader.
void SolarSystem::Render()
{
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK); // removes the back faces
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // projection and view matrices for this frame
    float const aspect_ratio = static_cast<float>(mWindow->getWidth()) / static_cast<float>(mWindow->getHeight());
    glm::mat4 const projection_matrix = glm::perspective(mFovY, aspect_ratio, mZNear, mZFar);
    glm::mat4 const view_matrix = mTurnTableCamera->ViewMatrix();
    glm::vec3 const camera_position = mTurnTableCamera->Position();

    glActiveTexture(GL_TEXTURE0);

    // render loop for each celestial body
    for (const auto& body_ptr : m_bodies)
    {
        CelestialBody* body = body_ptr.get();

        // star background
        if (body == m_stars)
        {
            glDisable(GL_CULL_FACE); // disabled back face culling so that texture appears on the inside of this sphere
            glDepthMask(GL_FALSE);

            mBasicShader->use(); 
            glUniform1i(glGetUniformLocation(*mBasicShader, "texture_sampler"), 0);
            glUniformMatrix4fv(glGetUniformLocation(*mBasicShader, "model_matrix"), 1, GL_FALSE, glm::value_ptr(m_stars->get_model_matrix()));
            glUniformMatrix4fv(glGetUniformLocation(*mBasicShader, "view_matrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
            glUniformMatrix4fv(glGetUniformLocation(*mBasicShader, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));

            m_stars->get_texture().bind();
            m_stars->get_geometry().bind();
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_stars->get_geometry().vertex_count()));
            m_stars->get_texture().unbind();

            glDepthMask(GL_TRUE);
            glEnable(GL_CULL_FACE); // re-enabled for other spheres
        }
        // render the sun - needed separate conditional because of back face culling
        else if (body == m_sun)
        {
            // sun emits light (i.e. not phong shaded)
            mBasicShader->use();
            glUniform1i(glGetUniformLocation(*mBasicShader, "texture_sampler"), 0);
            glUniformMatrix4fv(glGetUniformLocation(*mBasicShader, "model_matrix"), 1, GL_FALSE, glm::value_ptr(body->get_model_matrix()));
            glUniformMatrix4fv(glGetUniformLocation(*mBasicShader, "view_matrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
            glUniformMatrix4fv(glGetUniformLocation(*mBasicShader, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));

            body->get_texture().bind();
            body->get_geometry().bind();
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(body->get_geometry().vertex_count()));
            body->get_texture().unbind();
        }
        // earth and moon - phong shaded
        else
        {
            phong_shader->use();
            glUniform1i(glGetUniformLocation(*phong_shader, "texture_sampler"), 0);
            glUniformMatrix4fv(glGetUniformLocation(*phong_shader, "view_matrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
            glUniformMatrix4fv(glGetUniformLocation(*phong_shader, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
            glUniform3fv(glGetUniformLocation(*phong_shader, "camera_position"), 1, glm::value_ptr(camera_position));
            glUniformMatrix4fv(glGetUniformLocation(*phong_shader, "model_matrix"), 1, GL_FALSE, glm::value_ptr(body->get_model_matrix()));

            body->get_texture().bind();
            body->get_geometry().bind();
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(body->get_geometry().vertex_count()));
            body->get_texture().unbind();
        }
    }
}

//======================================================================================================================

// I'm keeping this to monitor performance
void SolarSystem::UI()
{
    ImGui::Begin("FPS Counter");
    ImGui::Text("FPS: %f", 1.0f / mTime->DeltaTimeSec());
    ImGui::End();
}

//======================================================================================================================

// Setup all the celestial bodies in the scene.
// I removed and replaced the old PrepareUnitSphereGeometry();
// method. I made a custom class CelestialBody to
// handle sphere properties instead

// basically just declare the bodies, initialize geometries,
// pass in texture filepath names and then set the basic properties 
// (like tilt, rotation speed, etc). The logic is handled in 
// ShapeGenerator::Sphere and the methods in CelestialBody.cpp/hpp
void SolarSystem::PrepareSphereGeometry()
{
    // STARS
    auto stars = std::make_unique<CelestialBody>();
    stars->initialize_geometry(1.0f, 40, 40);
    stars->set_texture("textures/8k_stars_milky_way.jpg");
    stars->scale = 100.0f;
    stars->axis_rotation_speed = 0.0f;
    stars->orbit_rotation_speed = 0.0f;
    stars->upload_to_gpu();

    m_stars = stars.get();
    m_bodies.push_back(std::move(stars));


    // SUN
    auto sun = std::make_unique<CelestialBody>();
    sun->initialize_geometry(1.0f, 40, 40);
    sun->set_texture("textures/8k_sun.jpg");
    sun->scale = 1.0f;
    sun->axis_rotation_speed = 0.4f;
    sun->orbit_rotation_speed = 0.0f;
    sun->upload_to_gpu();

    m_sun = sun.get();
    m_bodies.push_back(std::move(sun));

    // EARTH
    auto earth = std::make_unique<CelestialBody>();
    earth->initialize_geometry(1.0f, 40, 40);
    earth->set_texture("textures/8k_earth_daymap.jpg");
    earth->scale = 0.45f;
    earth->axis_tilt = glm::radians(23.4f);
    earth->orbit_tilt = glm::radians(15.0f);
    earth->orbit_radius = 4.0f;
    earth->axis_rotation_speed = 5.0f;
    earth->orbit_rotation_speed = 0.5f;
    earth->set_parent(m_sun);
    earth->upload_to_gpu();

    CelestialBody* earth_ptr = earth.get();
    m_bodies.push_back(std::move(earth));

    // looked into getting the cloud rendering working, but
    // I don't think I'll have time to get transparency working
    // EARTH CLOUDS
    // auto earth_clouds = std::make_unique<CelestialBody>();
    // earth_clouds->initialize_geometry(1.0f, 40, 40);
    // earth_clouds->set_texture("textures/8k_earth_clouds.jpg");
    // earth_clouds->set_parent(earth_ptr);
    // earth_clouds->scale = 1.01f;
    // earth_clouds->axis_rotation_speed = 4.8f; 
    // earth_clouds->orbit_rotation_speed = 0.5f; 
    // earth_clouds->upload_to_gpu();
    // m_bodies.push_back(std::move(earth_clouds));

    // MOON
    auto moon = std::make_unique<CelestialBody>();
    moon->initialize_geometry(1.0f, 40, 40);
    moon->set_texture("textures/2k_moon.jpg");
    moon->scale = 0.35f;
    moon->axis_tilt = glm::radians(7.00f);
    moon->orbit_tilt = glm::radians(-30.0f);
    moon->orbit_radius = 2.0f;
    moon->axis_rotation_speed = 0.5f;
    moon->orbit_rotation_speed = 2.0f;
    moon->set_parent(earth_ptr);
    moon->upload_to_gpu();

    m_bodies.push_back(std::move(moon));
}
//======================================================================================================================

// I'm doing this on MacOS, so the window size differs from the actual framebuffer size.
// glfwGetFramebufferSize gets correct pixel dimensions for setting the viewport.
void SolarSystem::OnResize(int width, int height)
{
    int fb_width, fb_height;
    glfwGetFramebufferSize(mWindow->getGLFWwindow(), &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);
}

//======================================================================================================================

void SolarSystem::OnMouseWheelChange(double const xOffset, double const yOffset) const
{
    mTurnTableCamera->ChangeRadius(-static_cast<float>(yOffset) * mZoomSpeed * mTime->DeltaTimeSec());
}

//======================================================================================================================
