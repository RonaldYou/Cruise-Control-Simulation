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
     * Get actual framebuffer size (may differ from window size on Retina/HiDPI displays)
     *
     * On macOS Retina displays, framebuffer is typically 2x the window size.
     * glViewport() works in framebuffer pixels, so we must use framebuffer dimensions.
     */
    glfwGetFramebufferSize(window_, &width_, &height_);

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

    /* Create camera - use actual framebuffer dimensions for aspect ratio */
    float aspectRatio = static_cast<float>(width_) / static_cast<float>(height_);
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

    /* Ground plane: Large grass area surrounding the road
     * Must be larger than the view area (500m x 500m) */
    glm::vec3 grassColor(0.2f, 0.4f, 0.2f);  /* Green grass */
    groundMesh_ = std::make_unique<Mesh>(Mesh::createQuad(500.0f, 500.0f, grassColor));

    /* Sky background plane for side view
     * Light blue, positioned behind everything */
    glm::vec3 skyColor(0.4f, 0.6f, 0.9f);  /* Light blue sky */
    skyMesh_ = std::make_unique<Mesh>(Mesh::createQuad(500.0f, 200.0f, skyColor));

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
 * render() - Draw the complete scene with split-screen views
 *
 * Layout:
 * ┌─────────────────────────────────────┐
 * │         BIRD'S EYE VIEW             │  Top half (full width)
 * ├───────────────────────────────┬─────┤
 * │         SIDE VIEW             │ 3RD │  Bottom half split 75%/25%
 * └───────────────────────────────┴─────┘
 * ============================================================================= */
void Renderer::render(const glm::vec3& carPosition, float carSpeed, float targetSpeed,
                      float throttle, float terrainGrade,
                      const std::vector<float>& elevationHistory) {
    int halfHeight = height_ / 2;
    int sideViewWidth = width_ * 3 / 4;
    int pipWidth = width_ - sideViewWidth;

    /* =========================================================================
     * Bird's Eye View (top half - full width)
     * ========================================================================= */
    glViewport(0, halfHeight, width_, halfHeight);
    renderBirdEyeView(carPosition, terrainGrade);

    /* =========================================================================
     * Side View (bottom-left, 75% width)
     * ========================================================================= */
    glViewport(0, 0, sideViewWidth, halfHeight);
    renderSideView(carPosition, terrainGrade, elevationHistory);

    /* =========================================================================
     * 3rd Person PiP (bottom-right, 25% width)
     * ========================================================================= */
    glViewport(sideViewWidth, 0, pipWidth, halfHeight);
    renderThirdPersonView(carPosition, terrainGrade);

    /* =========================================================================
     * Cleanup - Restore full viewport
     * ========================================================================= */
    glViewport(0, 0, width_, height_);

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
 * renderBirdEyeView() - Render the top-down bird's eye view
 *
 * Shows the road from directly above. Road runs horizontally (left-to-right)
 * on screen. Uses orthographic projection for consistent scale.
 * ============================================================================= */
void Renderer::renderBirdEyeView(const glm::vec3& carPosition, float grade) {
    /* Configure camera for bird's eye view */
    float viewWidth = 200.0f;   /* 200 meters visible along road (horizontal) */
    float halfHeight = static_cast<float>(height_) / 2.0f;
    float aspectRatio = static_cast<float>(width_) / halfHeight;

    /* Position camera directly above the car, looking down
     * Up vector is (1, 0, 0) so:
     *   - +Z (forward) appears as RIGHT on screen
     *   - +X (right lane) appears as TOP on screen
     *   - -X (left lane) appears as BOTTOM on screen
     * This matches 3rd person view where left lane is on the left/bottom */
    glm::vec3 cameraPos(carPosition.x, 100.0f, carPosition.z);
    glm::vec3 cameraTarget(carPosition.x, 0.0f, carPosition.z);
    glm::vec3 cameraUp(1.0f, 0.0f, 0.0f);  /* +X is up, so left lane (-X) is at bottom */

    camera_->setPositionAndTarget(cameraPos, cameraTarget, cameraUp);

    /* Orthographic bounds: horizontal is Z (road direction), vertical is X (lanes) */
    float viewHeight = viewWidth / aspectRatio;
    camera_->setOrthographic(
        -viewWidth / 2.0f, viewWidth / 2.0f,    /* left, right (Z in world = road) */
        -viewHeight / 2.0f, viewHeight / 2.0f,  /* bottom, top (X in world = lanes) */
        0.1f, 200.0f                             /* near, far */
    );

    /* Set up shader */
    shader_->use();
    shader_->setMat4("view", camera_->getViewMatrix());
    shader_->setMat4("projection", camera_->getProjectionMatrix());

    /* Lighting from above for bird's eye */
    shader_->setVec3("lightDir", glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)));
    shader_->setVec3("lightColor", glm::vec3(1.0f, 0.98f, 0.9f));
    shader_->setFloat("ambientStrength", 0.5f);

    /* Render ground plane first (grass) */
    glDisable(GL_CULL_FACE);
    glm::mat4 groundModel = glm::mat4(1.0f);
    groundModel = glm::translate(groundModel, glm::vec3(0.0f, -0.1f, carPosition.z));
    shader_->setMat4("model", groundModel);
    groundMesh_->draw();
    glEnable(GL_CULL_FACE);

    /* Render road on top of ground */
    renderRoad(carPosition, 0.0f);  /* No grade tilt in bird's eye view */

    /* Use flat car position (Y=0) - elevation should not affect bird's eye view */
    glm::vec3 flatCarPos(carPosition.x, 0.0f, carPosition.z);
    renderCar(flatCarPos);
}

/* =============================================================================
 * renderSideView() - Render the side profile view
 *
 * Shows the terrain elevation profile. The road appears as a curvy line
 * showing hills and valleys based on elevation history.
 * ============================================================================= */
void Renderer::renderSideView(const glm::vec3& carPosition, float grade,
                               const std::vector<float>& elevationHistory) {
    /* Configure camera for side view - looking along X axis at the YZ plane */
    float viewWidth = 200.0f;   /* 200 meters visible along road (Z) */
    float viewHeightM = 30.0f;  /* 30 meters vertical range (Y) */
    float sideViewWidth = static_cast<float>(width_) * 0.75f;
    float halfHeight = static_cast<float>(height_) / 2.0f;
    float aspectRatio = sideViewWidth / halfHeight;

    /* Adjust view height based on aspect ratio */
    float adjustedViewHeight = viewWidth / aspectRatio;
    if (adjustedViewHeight < viewHeightM) {
        adjustedViewHeight = viewHeightM;
    }

    /* Get current elevation to center camera on the car */
    float currentElevation = elevationHistory.empty() ? 0.0f : elevationHistory.back();
    float cameraY = currentElevation + 5.0f;  /* Slightly above car for better view */

    /* Position camera to the left, looking at the road from the side
     * Camera sees: Z as horizontal (left-right), Y as vertical (up-down)
     * Camera Y follows current elevation so terrain stays in view */
    glm::vec3 cameraPos(-100.0f, cameraY, carPosition.z);
    glm::vec3 cameraTarget(0.0f, cameraY, carPosition.z);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);  /* Y is up */

    camera_->setPositionAndTarget(cameraPos, cameraTarget, cameraUp);
    camera_->setOrthographic(
        -viewWidth / 2.0f, viewWidth / 2.0f,              /* left, right (Z in world) */
        -adjustedViewHeight / 2.0f, adjustedViewHeight / 2.0f,  /* bottom, top (Y in world) */
        0.1f, 200.0f                                       /* near, far */
    );

    /* Set up shader */
    shader_->use();
    shader_->setMat4("view", camera_->getViewMatrix());
    shader_->setMat4("projection", camera_->getProjectionMatrix());

    /* Lighting from the side */
    shader_->setVec3("lightDir", glm::normalize(glm::vec3(1.0f, -0.3f, 0.2f)));
    shader_->setVec3("lightColor", glm::vec3(1.0f, 0.98f, 0.9f));
    shader_->setFloat("ambientStrength", 0.5f);

    /* Render sky background first (furthest back)
     * Position it at X=50 (far from camera at X=-100, behind terrain at X=0)
     * Sky follows camera Y so it's always visible in background */
    glDisable(GL_CULL_FACE);
    glm::mat4 skyModel = glm::mat4(1.0f);
    skyModel = glm::translate(skyModel, glm::vec3(50.0f, cameraY + 45.0f, carPosition.z));
    skyModel = glm::rotate(skyModel, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));  /* Rotate to face camera */
    shader_->setMat4("model", skyModel);
    skyMesh_->draw();
    glEnable(GL_CULL_FACE);

    /* Render terrain profile if we have elevation data */
    if (!elevationHistory.empty()) {
        renderTerrainProfile(carPosition, elevationHistory);
    }

    /* Render car at its elevation */
    float carElevation = elevationHistory.empty() ? 0.0f : elevationHistory.back();
    glm::vec3 carPosWithElevation(carPosition.x, carElevation + 0.75f, carPosition.z);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, carPosWithElevation);
    shader_->setMat4("model", model);
    carMesh_->draw();
}

/* =============================================================================
 * renderThirdPersonView() - Render the classic 3rd-person follow camera
 *
 * The original view - camera follows behind the car with perspective.
 * Rendered in a small picture-in-picture window.
 * ============================================================================= */
void Renderer::renderThirdPersonView(const glm::vec3& carPosition, float grade) {
    /* Configure camera for 3rd-person view */
    float pipWidth = static_cast<float>(width_) * 0.25f;
    float pipHeight = static_cast<float>(height_) / 2.0f;
    float aspectRatio = pipWidth / pipHeight;

    /* Use flat car position (Y=0) - elevation should not affect 3rd person view */
    glm::vec3 flatCarPos(carPosition.x, 0.0f, carPosition.z);

    /* Use the follow camera (original behavior) */
    glm::vec3 carDirection(0.0f, 0.0f, 1.0f);
    camera_->setPerspective(45.0f, aspectRatio);
    camera_->update(flatCarPos, carDirection);

    /* Set up shader */
    shader_->use();
    shader_->setMat4("view", camera_->getViewMatrix());
    shader_->setMat4("projection", camera_->getProjectionMatrix());

    /* Standard lighting */
    shader_->setVec3("lightDir", glm::normalize(glm::vec3(0.3f, -1.0f, 0.2f)));
    shader_->setVec3("lightColor", glm::vec3(1.0f, 0.98f, 0.9f));
    shader_->setFloat("ambientStrength", 0.3f);

    /* Render ground plane first */
    glDisable(GL_CULL_FACE);
    glm::mat4 groundModel = glm::mat4(1.0f);
    groundModel = glm::translate(groundModel, glm::vec3(0.0f, -0.1f, carPosition.z));
    shader_->setMat4("model", groundModel);
    groundMesh_->draw();
    glEnable(GL_CULL_FACE);

    /* Render scene */
    renderRoad(carPosition, grade);
    renderCar(flatCarPos);
}

/* =============================================================================
 * renderTerrainProfile() - Render the terrain elevation profile
 *
 * Creates a visual representation of terrain as a filled polygon showing
 * the elevation history. Each point in elevationHistory represents the
 * elevation at a position along the road.
 * ============================================================================= */
void Renderer::renderTerrainProfile(const glm::vec3& carPosition,
                                     const std::vector<float>& elevationHistory) {
    if (elevationHistory.size() < 2) return;

    /* Each elevation point represents 1 meter of travel
     * We'll render the terrain as a series of quads */
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 terrainColor(0.4f, 0.6f, 0.3f);  /* Green-brown for terrain */
    glm::vec3 roadColor(0.3f, 0.3f, 0.35f);     /* Dark gray for road surface */

    float metersPerPoint = 1.0f;  /* Each history point = 1 meter */
    size_t numPoints = elevationHistory.size();

    /* Calculate Z position for each point
     * The last point is at carPosition.z, earlier points are behind */
    float startZ = carPosition.z - (numPoints - 1) * metersPerPoint;

    /* Create terrain mesh - a series of connected quads */
    float bottomY = -10.0f;  /* Below visible area */

    for (size_t i = 0; i < numPoints; ++i) {
        float z = startZ + i * metersPerPoint;
        float y = elevationHistory[i];

        /* Top vertex (terrain surface) */
        vertices.push_back({{0.0f, y, z}, {-1.0f, 0.0f, 0.0f}, terrainColor});
        /* Bottom vertex (below ground) */
        vertices.push_back({{0.0f, bottomY, z}, {-1.0f, 0.0f, 0.0f}, terrainColor});
    }

    /* Create indices for quads between consecutive points */
    for (size_t i = 0; i < numPoints - 1; ++i) {
        unsigned int topLeft = i * 2;
        unsigned int bottomLeft = i * 2 + 1;
        unsigned int topRight = (i + 1) * 2;
        unsigned int bottomRight = (i + 1) * 2 + 1;

        /* First triangle */
        indices.push_back(topLeft);
        indices.push_back(bottomLeft);
        indices.push_back(topRight);

        /* Second triangle */
        indices.push_back(topRight);
        indices.push_back(bottomLeft);
        indices.push_back(bottomRight);
    }

    /* Create and render the terrain mesh */
    if (!vertices.empty() && !indices.empty()) {
        Mesh terrainMesh(vertices, indices);

        glDisable(GL_CULL_FACE);  /* Render both sides */

        glm::mat4 model = glm::mat4(1.0f);
        shader_->setMat4("model", model);
        terrainMesh.draw();

        glEnable(GL_CULL_FACE);
    }

    /* Also render a road surface line on top of terrain */
    std::vector<Vertex> roadVertices;
    std::vector<unsigned int> roadIndices;

    float roadThickness = 0.5f;

    for (size_t i = 0; i < numPoints; ++i) {
        float z = startZ + i * metersPerPoint;
        float y = elevationHistory[i];

        /* Road surface vertices */
        roadVertices.push_back({{0.0f, y + roadThickness, z}, {-1.0f, 0.0f, 0.0f}, roadColor});
        roadVertices.push_back({{0.0f, y, z}, {-1.0f, 0.0f, 0.0f}, roadColor});
    }

    for (size_t i = 0; i < numPoints - 1; ++i) {
        unsigned int topLeft = i * 2;
        unsigned int bottomLeft = i * 2 + 1;
        unsigned int topRight = (i + 1) * 2;
        unsigned int bottomRight = (i + 1) * 2 + 1;

        roadIndices.push_back(topLeft);
        roadIndices.push_back(bottomLeft);
        roadIndices.push_back(topRight);

        roadIndices.push_back(topRight);
        roadIndices.push_back(bottomLeft);
        roadIndices.push_back(bottomRight);
    }

    if (!roadVertices.empty() && !roadIndices.empty()) {
        Mesh roadProfileMesh(roadVertices, roadIndices);

        glDisable(GL_CULL_FACE);

        glm::mat4 model = glm::mat4(1.0f);
        shader_->setMat4("model", model);
        roadProfileMesh.draw();

        glEnable(GL_CULL_FACE);
    }
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
