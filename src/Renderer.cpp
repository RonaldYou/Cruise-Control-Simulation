/*
 * Renderer.cpp - OpenGL Rendering Implementation
 */

#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <stdexcept>

/* =============================================================================
 * Constructor - Set up the entire rendering system
 * ============================================================================= */
Renderer::Renderer(int width, int height, const std::string& title)
    : window_(nullptr)
    , width_(width)
    , height_(height)
{
    initGLFW();

    /* Create the window
     *
     * Parameters:
     *   width, height: Window size in pixels
     *   title: Window title bar text
     *   monitor: NULL for windowed mode, or a monitor for fullscreen
     *   share: NULL, or another window to share OpenGL resources with
     */
    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    /*
     * Make this window's OpenGL context "current"
     *
     * OpenGL is a state machine. All GL calls affect the "current" context.
     * Each window has its own context, and we must make ours current before
     * calling any GL functions.
     */
    glfwMakeContextCurrent(window_);

    /*
     * Store 'this' pointer in window for use in callbacks
     *
     * GLFW callbacks are C functions (no 'this' pointer). We store our
     * Renderer pointer in the window's "user pointer" so callbacks can
     * access instance data.
     */
    glfwSetWindowUserPointer(window_, this);

    /* Set up window resize callback */
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);

    /* Initialize OpenGL function pointers via GLAD */
    initOpenGL();

    /* Load shaders */
    initShaders();

    /* Create geometry */
    initGeometry();

    /* Create camera */
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    camera_ = std::make_unique<Camera>(15.0f, 8.0f, aspectRatio);
}

/* =============================================================================
 * Destructor - Clean up resources in reverse order of creation
 * ============================================================================= */
Renderer::~Renderer() {
    /* unique_ptrs automatically delete their objects */
    carMesh_.reset();
    roadMesh_.reset();
    shader_.reset();
    camera_.reset();

    /* Destroy window and terminate GLFW */
    if (window_) {
        glfwDestroyWindow(window_);
    }
    glfwTerminate();
}

/* =============================================================================
 * initGLFW() - Initialize the GLFW library
 *
 * GLFW handles:
 * - Window creation
 * - OpenGL context creation
 * - Input (keyboard, mouse, gamepad)
 * - Event handling
 * ============================================================================= */
void Renderer::initGLFW() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    /*
     * Set OpenGL version hints
     *
     * These must be set BEFORE creating the window. They tell GLFW what
     * kind of OpenGL context to create.
     *
     * 3.3 Core Profile:
     * - Version 3.3 is widely supported (2010+)
     * - Core Profile removes deprecated functions (cleaner API)
     * - Forward Compatible means no deprecated functionality at all
     */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* macOS requires this for OpenGL 3.2+ contexts */
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
}

/* =============================================================================
 * initOpenGL() - Load OpenGL functions via GLAD
 * ============================================================================= */
void Renderer::initOpenGL() {
    /*
     * GLAD loads OpenGL function pointers
     *
     * glfwGetProcAddress is a function that looks up GL functions by name.
     * GLAD uses it to populate all the gl* function pointers.
     */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    /* Print OpenGL info for debugging */
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    /*
     * Configure OpenGL state
     */

    /* Enable depth testing - closer objects occlude farther ones */
    glEnable(GL_DEPTH_TEST);

    /* Set the background color (dark sky blue) */
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    /* Enable back-face culling - don't render faces pointing away from camera */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

/* =============================================================================
 * initShaders() - Load and compile shader programs
 * ============================================================================= */
void Renderer::initShaders() {
    shader_ = std::make_unique<Shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
}

/* =============================================================================
 * initGeometry() - Create meshes for scene objects
 * ============================================================================= */
void Renderer::initGeometry() {
    /* Car: A simple box, slightly longer than wide
     * Low-poly styled car in blue */
    glm::vec3 carColor(0.2f, 0.4f, 0.8f);  /* Blue */
    carMesh_ = std::make_unique<Mesh>(Mesh::createBox(2.0f, 1.5f, 4.0f, carColor));

    /* Road dimensions */
    const float roadWidth = 12.0f;
    const float roadLength = 300.0f;
    const float lineYOffset = 0.1f;  /* 5cm above road to prevent z-fighting at shallow angles */

    /* Road: Wide and long road segment
     * Medium gray - visible against dark background */
    glm::vec3 roadColor(0.3f, 0.3f, 0.35f);  /* Dark gray asphalt */
    roadMesh_ = std::make_unique<Mesh>(Mesh::createRoadSegment(roadWidth, roadLength, roadColor));

    /* Center dashed line - yellow
     * Dash: 3m long, Gap: 6m, Width: 0.2m */
    glm::vec3 centerLineColor(1.0f, 0.9f, 0.3f);  /* Yellow */
    centerLineMesh_ = std::make_unique<Mesh>(
        Mesh::createDashedLine(roadLength, 3.0f, 6.0f, 0.2f, lineYOffset, centerLineColor));

    /* Edge lines - solid white, positioned at road edges */
    glm::vec3 edgeLineColor(1.0f, 1.0f, 1.0f);  /* White */
    float edgeOffset = (roadWidth / 2.0f) - 0.3f;  /* 30cm from edge */
    leftEdgeLineMesh_ = std::make_unique<Mesh>(
        Mesh::createSolidLine(roadLength, 0.2f, -edgeOffset, lineYOffset, edgeLineColor));
    rightEdgeLineMesh_ = std::make_unique<Mesh>(
        Mesh::createSolidLine(roadLength, 0.2f, edgeOffset, lineYOffset, edgeLineColor));
}

/* =============================================================================
 * shouldClose() - Check if window should close
 * ============================================================================= */
bool Renderer::shouldClose() const {
    return glfwWindowShouldClose(window_);
}

/* =============================================================================
 * beginFrame() - Prepare for rendering a new frame
 *
 * Clears the color and depth buffers to start fresh.
 * ============================================================================= */
void Renderer::beginFrame() {
    /*
     * Clear buffers
     *
     * GL_COLOR_BUFFER_BIT: Clear the color buffer (what you see)
     * GL_DEPTH_BUFFER_BIT: Clear the depth buffer (Z values)
     *
     * The depth buffer stores how far each pixel is from the camera.
     * It must be cleared each frame or old depth values cause artifacts.
     */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/* =============================================================================
 * endFrame() - Finish rendering and display the frame
 * ============================================================================= */
void Renderer::endFrame() {
    /*
     * Swap front and back buffers
     *
     * OpenGL uses "double buffering":
     * - Back buffer: Where we draw (invisible)
     * - Front buffer: What's displayed on screen
     *
     * glfwSwapBuffers swaps them, making our rendered frame visible.
     * This prevents flickering/tearing.
     */
    glfwSwapBuffers(window_);

    /*
     * Poll for events
     *
     * Processes window events (resize, close, input).
     * Must be called regularly or the window becomes unresponsive.
     */
    glfwPollEvents();
}

/* =============================================================================
 * render() - Draw the complete scene
 * ============================================================================= */
void Renderer::render(const glm::vec3& carPosition, float carSpeed, float targetSpeed,
                      float throttle, float terrainGrade) {
    /* Update camera to follow car */
    glm::vec3 carDirection(0.0f, 0.0f, 1.0f);  /* Car always faces +Z for now */
    camera_->update(carPosition, carDirection);

    /* Activate shader */
    shader_->use();

    /* Set camera matrices */
    shader_->setMat4("view", camera_->getViewMatrix());
    shader_->setMat4("projection", camera_->getProjectionMatrix());

    /* Set lighting (simple directional light from above-right) */
    shader_->setVec3("lightDir", glm::normalize(glm::vec3(0.3f, -1.0f, 0.2f)));
    shader_->setVec3("lightColor", glm::vec3(1.0f, 0.98f, 0.9f));  /* Warm white */
    shader_->setFloat("ambientStrength", 0.3f);

    /* Render scene objects */
    renderRoad(carPosition, terrainGrade);
    renderCar(carPosition);

    /* Print stats to console */
    printStats(carSpeed, targetSpeed, throttle, terrainGrade);
}

/* =============================================================================
 * renderRoad() - Draw the road surface and lane markings
 * ============================================================================= */
void Renderer::renderRoad(const glm::vec3& carPosition, float grade) {
    /* Disable face culling for road surfaces */
    glDisable(GL_CULL_FACE);

    /* Rotate road to match terrain grade (pitch around X axis) */
    float pitchAngle = glm::atan(grade);

    /* Road surface - follows car position */
    glm::mat4 roadModel = glm::mat4(1.0f);
    roadModel = glm::translate(roadModel, glm::vec3(0.0f, 0.0f, carPosition.z));
    roadModel = glm::rotate(roadModel, pitchAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    shader_->setMat4("model", roadModel);
    roadMesh_->draw();

    /* Disable depth testing for road lines
     *
     * At shallow camera angles, even with Y offset, depth buffer precision issues
     * cause z-fighting (flickering/disappearing lines). By disabling depth testing,
     * lines always render on top of the road since they're drawn after it.
     *
     * This is safe because:
     * 1. Lines have a Y offset (are physically above the road)
     * 2. Lines are drawn after the road surface
     * 3. The car is drawn after lines (with depth test re-enabled)
     */
    glDisable(GL_DEPTH_TEST);

    /* Edge lines - solid, follow car (no scroll needed) */
    leftEdgeLineMesh_->draw();
    rightEdgeLineMesh_->draw();

    /* Center dashed line - needs to scroll to show movement
     * Calculate offset so dashes appear to move past the car
     * Dash pattern repeats every (dashLength + gapLength) = 9 meters */
    const float patternLength = 9.0f;  /* 3m dash + 6m gap */
    float scrollOffset = fmod(carPosition.z, patternLength);

    glm::mat4 lineModel = glm::mat4(1.0f);
    /* Offset the dashed line backwards by scroll amount so it appears stationary
     * while the car moves forward */
    lineModel = glm::translate(lineModel, glm::vec3(0.0f, 0.0f, carPosition.z - scrollOffset));
    lineModel = glm::rotate(lineModel, pitchAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    shader_->setMat4("model", lineModel);
    centerLineMesh_->draw();

    /* Re-enable depth testing for subsequent objects (car) */
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
}

/* =============================================================================
 * renderCar() - Draw the car
 * ============================================================================= */
void Renderer::renderCar(const glm::vec3& carPosition) {
    glm::mat4 model = glm::mat4(1.0f);

    /* Position car at its world position, raised slightly above road */
    model = glm::translate(model, carPosition + glm::vec3(0.0f, 0.75f, 0.0f));

    shader_->setMat4("model", model);
    carMesh_->draw();
}

/* =============================================================================
 * printStats() - Print simulation stats to console
 *
 * Simple text output showing speed, target, throttle, and grade.
 * ============================================================================= */
void Renderer::printStats(float speed, float targetSpeed, float throttle, float grade) {
    /* Only update console every ~10 frames to reduce flicker */
    static int frameCount = 0;

    /* Convert to display units */
    float speedKmh = speed * 3.6f;
    float targetKmh = targetSpeed * 3.6f;
    float gradePercent = grade * 100.0f;
    float throttlePercent = throttle * 100.0f;

    /* Print on single line with carriage return to overwrite */
    std::cout << "\rSpeed: " << std::fixed << std::setprecision(1) << std::setw(6) << speedKmh
              << " km/h | Target: " << std::setw(6) << targetKmh
              << " km/h | Throttle: " << std::setw(5) << throttlePercent
              << "% | Grade: " << std::setw(5) << gradePercent << "%   " << std::flush;
}

/* =============================================================================
 * framebufferSizeCallback() - Handle window resize
 *
 * Called by GLFW when the window is resized. Updates the viewport and
 * camera aspect ratio.
 * ============================================================================= */
void Renderer::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    /* Get our Renderer instance from the window's user pointer */
    Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));

    /* Update stored dimensions */
    renderer->width_ = width;
    renderer->height_ = height;

    /*
     * Update OpenGL viewport
     *
     * The viewport defines which part of the window OpenGL renders to.
     * Usually it's the entire window.
     */
    glViewport(0, 0, width, height);

    /* Update camera aspect ratio to prevent distortion */
    if (height > 0) {
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        renderer->camera_->setAspectRatio(aspectRatio);
    }
}
