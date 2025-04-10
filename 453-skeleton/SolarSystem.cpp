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

    // Load both shaders
    m_phong_shader = std::make_unique<ShaderProgram>(
        mPath->Get("shaders/test.vert"),
        mPath->Get("shaders/phong.frag")
    );

    m_unlit_shader = std::make_unique<ShaderProgram>(
        mPath->Get("shaders/test.vert"),
        mPath->Get("shaders/test.frag")
    );

    // in hindsight I could have just used this but whatever.
    // mBasicShader = std::make_unique<ShaderProgram>(
    //     mPath->Get("shaders/test.vert"),
    //     mPath->Get("shaders/test.frag")
    // );

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
        //glClearColor(0.2f, 0.6f, 0.8f, 1.0f);
        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // https://www.viewsonic.com/library/creative-work/srgb-vs-adobe-rgb-which-one-to-use/
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear render screen (all zero) and depth (all max depth)
        //glViewport(0, 0, mWindow->getWidth(), mWindow->getHeight());
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

void SolarSystem::Update(float const deltaTime)
{
    // === Mouse / Camera Rotation ===
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

    if (mInputManager->IsKeyboardButtonDown(GLFW_KEY_SPACE)) {
        m_paused = true;
    }
    if (mInputManager->IsKeyboardButtonDown(GLFW_KEY_R)) {
        m_reset = true;
    }
    if (mInputManager->IsKeyboardButtonDown(GLFW_KEY_W)) {
        m_animation_speed = std::min(m_animation_speed + 0.5f, 20.0f);
        m_paused = false;
    }
    if (mInputManager->IsKeyboardButtonDown(GLFW_KEY_S)) {
        m_animation_speed = std::max(m_animation_speed - 0.5f, 0.1f);
        m_paused = false;
    }

    for (const auto& body_ptr : m_bodies)
    {
        CelestialBody* body = body_ptr.get();
        if (m_reset) {
            body->reset_rotation();
        }
        else if (!m_paused) {
            body->update_transform(deltaTime * m_animation_speed);
        }
    }

    if (!mInputManager->IsKeyboardButtonDown(GLFW_KEY_R)) {
        m_reset = false;
    }
}

//======================================================================================================================

void SolarSystem::Render()
{
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    float const aspect_ratio = static_cast<float>(mWindow->getWidth()) / static_cast<float>(mWindow->getHeight());
    glm::mat4 const projection_matrix = glm::perspective(mFovY, aspect_ratio, mZNear, mZFar);
    glm::mat4 const view_matrix = mTurnTableCamera->ViewMatrix();
    glm::vec3 const camera_position = mTurnTableCamera->Position();

    for (const auto& body_ptr : m_bodies)
    {
        CelestialBody* body = body_ptr.get();
        if (body == m_stars)
        {
            // glDisable(GL_CULL_FACE);  // Disable face culling
            // glDepthMask(GL_FALSE);    // Optional: prevent depth writing
        
            // m_unlit_shader->use();
        
            // glUniformMatrix4fv(glGetUniformLocation(*m_unlit_shader, "model_matrix"), 1, GL_FALSE, glm::value_ptr(body->get_model_matrix()));
            // glUniformMatrix4fv(glGetUniformLocation(*m_unlit_shader, "view_matrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
            // glUniformMatrix4fv(glGetUniformLocation(*m_unlit_shader, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
        
            // glActiveTexture(GL_TEXTURE0);
            // glUniform1i(glGetUniformLocation(*m_unlit_shader, "texture_sampler"), 0);
            // body->get_texture().bind();
            // body->get_geometry().bind();
            // glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(body->get_geometry().vertex_count()));
            // body->get_texture().unbind();
        
            // glDepthMask(GL_TRUE);
            // glEnable(GL_CULL_FACE); // Restore culling after stars
            // === Render Star Background (unlit shader, disable culling and depth writes) ===
            glDisable(GL_CULL_FACE);
            // glDepthMask(GL_FALSE);
                    
            m_unlit_shader->use();
            glUniformMatrix4fv(glGetUniformLocation(*m_unlit_shader, "model_matrix"), 1, GL_FALSE, glm::value_ptr(m_stars->get_model_matrix()));
            glUniformMatrix4fv(glGetUniformLocation(*m_unlit_shader, "view_matrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
            glUniformMatrix4fv(glGetUniformLocation(*m_unlit_shader, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
            glUniform1i(glGetUniformLocation(*m_unlit_shader, "texture_sampler"), 0);
                    
            glActiveTexture(GL_TEXTURE0);
            m_stars->get_texture().bind();
            m_stars->get_geometry().bind();
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_stars->get_geometry().vertex_count()));
            m_stars->get_texture().unbind();
                    
            // === Restore default depth + culling ===
            // glDepthMask(GL_TRUE);
            glEnable(GL_CULL_FACE);
        }
        if (body == m_sun)
        {
            m_unlit_shader->use();
    
            glUniformMatrix4fv(glGetUniformLocation(*m_unlit_shader, "model_matrix"), 1, GL_FALSE, glm::value_ptr(body->get_model_matrix()));
            glUniformMatrix4fv(glGetUniformLocation(*m_unlit_shader, "view_matrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
            glUniformMatrix4fv(glGetUniformLocation(*m_unlit_shader, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
    
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(*m_unlit_shader, "texture_sampler"), 0);
            body->get_texture().bind();
            body->get_geometry().bind();
    
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(body->get_geometry().vertex_count()));
            body->get_texture().unbind();
        }
        else
        {
            m_phong_shader->use();
    
            glUniformMatrix4fv(glGetUniformLocation(*m_phong_shader, "view_matrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
            glUniformMatrix4fv(glGetUniformLocation(*m_phong_shader, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
            glUniform3fv(glGetUniformLocation(*m_phong_shader, "camera_position"), 1, glm::value_ptr(camera_position));
            glUniform1i(glGetUniformLocation(*m_phong_shader, "texture_sampler"), 0);
    
            glUniformMatrix4fv(glGetUniformLocation(*m_phong_shader, "model_matrix"), 1, GL_FALSE, glm::value_ptr(body->get_model_matrix()));
            glActiveTexture(GL_TEXTURE0);
            body->get_texture().bind();
            body->get_geometry().bind();
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(body->get_geometry().vertex_count()));
            body->get_texture().unbind();
        }
    }

}

//======================================================================================================================

void SolarSystem::UI()
{
    ImGui::Begin("FPS Counter");
    ImGui::Text("FPS: %f", 1.0f / mTime->DeltaTimeSec());
    ImGui::End();
}

//======================================================================================================================
void SolarSystem::PrepareSphereGeometry()
{
    // === STARS ===
    auto stars = std::make_unique<CelestialBody>();
    stars->initialize_geometry(1.0f, 40, 40);
    stars->set_texture("textures/8k_stars_milky_way.jpg");
    stars->scale = 100.0f;
    stars->axis_rotation_speed = 0.0f;
    stars->orbit_rotation_speed = 0.0f;
    stars->upload_to_gpu();

    m_stars = stars.get();
    m_bodies.push_back(std::move(stars));


    // === SUN ===
    auto sun = std::make_unique<CelestialBody>();
    sun->initialize_geometry(1.0f, 40, 40);
    sun->set_texture("textures/8k_sun.jpg");
    sun->scale = 1.0f;
    sun->axis_rotation_speed = 0.4f;
    sun->orbit_rotation_speed = 0.0f;
    sun->upload_to_gpu();

    m_sun = sun.get();
    m_bodies.push_back(std::move(sun));

    // === EARTH ===
    auto earth = std::make_unique<CelestialBody>();
    earth->initialize_geometry(1.0f, 40, 40);
    earth->set_texture("textures/8k_earth_daymap.jpg");
    earth->scale = 0.45f;
    earth->axis_tilt = glm::radians(23.4f);
    earth->orbit_radius = 4.0f;
    earth->axis_rotation_speed = 5.0f;
    earth->orbit_rotation_speed = 0.5f;
    earth->set_parent(m_sun);
    earth->upload_to_gpu();

    CelestialBody* earth_ptr = earth.get(); // Store pointer to use for Moon
    m_bodies.push_back(std::move(earth));

    // === MOON ===
    auto moon = std::make_unique<CelestialBody>();
    moon->initialize_geometry(1.0f, 40, 40);
    moon->set_texture("textures/2k_moon.jpg");
    moon->scale = 0.35f;
    moon->axis_tilt = glm::radians(6.68f);
    moon->orbit_tilt = glm::radians(-28.5f);
    moon->orbit_radius = 2.0f;
    moon->axis_rotation_speed = 0.5f;
    moon->orbit_rotation_speed = 2.0f;
    moon->set_parent(earth_ptr);
    moon->upload_to_gpu();

    m_bodies.push_back(std::move(moon));
}
//======================================================================================================================

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
