/*
 * Renderer.h - OpenGL Rendering Manager
 *
 * This class manages the entire rendering pipeline:
 * 1. Creates and manages the GLFW window
 * 2. Initializes OpenGL via GLAD
 * 3. Loads shaders
 * 4. Creates geometry (car, road)
 * 5. Manages camera
 * 6. Renders each frame
 *
 * Usage:
 *   Renderer renderer(1280, 720, "My App");
 *   while (!renderer.shouldClose()) {
 *       renderer.beginFrame();
 *       renderer.render(carPosition, carSpeed, terrainGrade);
 *       renderer.endFrame();
 *   }
 */

#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>

/* Tell GLFW not to include OpenGL headers - we use GLAD instead.
 * On macOS, the system OpenGL headers conflict with GLAD's definitions.
 * This must be defined BEFORE including glfw3.h */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

/* Forward declarations (avoid including heavy headers) */
class Shader;
class Camera;
class Mesh;

class Renderer {
public:
    /* =========================================================================
     * Constructor - Initialize window and OpenGL
     *
     * Parameters:
     *   width, height - Window dimensions in pixels
     *   title        - Window title bar text
     * ========================================================================= */
    Renderer(int width, int height, const std::string& title);

    /* Destructor - Clean up GLFW and OpenGL resources */
    ~Renderer();

    /* No copying (OpenGL context is not copyable) */
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /* =========================================================================
     * Window Management
     * ========================================================================= */

    /* Returns true if the window should close (user clicked X, etc.) */
    [[nodiscard]] bool shouldClose() const;

    /* =========================================================================
     * Frame Lifecycle
     *
     * Each frame follows this pattern:
     *   beginFrame();   // Clear screen, prepare for rendering
     *   render(...);    // Draw the scene
     *   endFrame();     // Swap buffers, poll input
     * ========================================================================= */
    void beginFrame();
    void endFrame();

    /* =========================================================================
     * render() - Draw the simulation scene
     *
     * Parameters:
     *   carPosition   - World position of the car (x, y, z)
     *   carSpeed      - Current speed in m/s
     *   targetSpeed   - Target cruise control speed in m/s
     *   throttle      - Current throttle input (0.0 to 1.0)
     *   terrainGrade  - Current road grade (-1 to 1, for road tilt)
     * ========================================================================= */
    void render(const glm::vec3& carPosition, float carSpeed, float targetSpeed,
                float throttle, float terrainGrade);

    /* =========================================================================
     * Input Handling (for future use)
     * ========================================================================= */
    [[nodiscard]] GLFWwindow* getWindow() const { return window_; }

private:
    /* Window and context */
    GLFWwindow* window_;
    int width_;
    int height_;

    /* Rendering components (using unique_ptr for RAII) */
    std::unique_ptr<Shader> shader_;
    std::unique_ptr<Camera> camera_;

    /* Scene geometry */
    std::unique_ptr<Mesh> carMesh_;
    std::unique_ptr<Mesh> roadMesh_;
    std::unique_ptr<Mesh> centerLineMesh_;   /* Dashed yellow center line */
    std::unique_ptr<Mesh> leftEdgeLineMesh_; /* Solid white left edge */
    std::unique_ptr<Mesh> rightEdgeLineMesh_;/* Solid white right edge */

    /* =========================================================================
     * Private Initialization Methods
     * ========================================================================= */
    void initGLFW();
    void initOpenGL();
    void initShaders();
    void initGeometry();

    /* =========================================================================
     * Rendering Helpers
     * ========================================================================= */
    void renderRoad(const glm::vec3& carPosition, float grade);
    void renderCar(const glm::vec3& carPosition);
    void printStats(float speed, float targetSpeed, float throttle, float grade);

    /* GLFW callbacks (must be static, but can access instance via user pointer) */
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};

#endif // RENDERER_H
