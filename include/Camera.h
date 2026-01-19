/*
 * Camera.h - 3rd Person Follow Camera
 *
 * This camera follows behind a target (the car) and looks at it.
 *
 * 3D Rendering requires two key matrices:
 *
 * 1. VIEW MATRIX: Transforms world coordinates to camera coordinates
 *    - "Where is the camera and what is it looking at?"
 *    - Moves the entire world so the camera is at origin looking down -Z
 *
 * 2. PROJECTION MATRIX: Transforms camera coordinates to clip coordinates
 *    - "How does the camera see?" (perspective vs orthographic)
 *    - Creates the 3D perspective effect (distant objects appear smaller)
 *
 * Together: WorldPosition -> View -> CameraSpace -> Projection -> ScreenPosition
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    /* =========================================================================
     * Constructor
     *
     * Parameters:
     *   followDistance - How far behind the target the camera sits
     *   height         - How high above the target the camera sits
     *   aspectRatio    - Window width / height (for correct perspective)
     * ========================================================================= */
    Camera(float followDistance = 10.0f, float height = 5.0f, float aspectRatio = 16.0f/9.0f);

    /* =========================================================================
     * update() - Update camera to follow a target position
     *
     * The camera positions itself behind the target based on the target's
     * direction of travel (or a fixed direction if stationary).
     *
     * Parameters:
     *   targetPos - The position to follow (car's position)
     *   targetDir - The direction the target is facing (normalized)
     *   dt        - Delta time for smooth interpolation (optional)
     * ========================================================================= */
    void update(const glm::vec3& targetPos, const glm::vec3& targetDir, float dt = 0.0f);

    /* =========================================================================
     * Matrix Getters
     *
     * getViewMatrix(): Returns the view matrix (world -> camera space)
     *   Used in shader as: gl_Position = projection * view * model * position;
     *
     * getProjectionMatrix(): Returns the projection matrix (camera -> clip space)
     *   Creates the perspective effect where far objects appear smaller.
     * ========================================================================= */
    [[nodiscard]] glm::mat4 getViewMatrix() const;
    [[nodiscard]] glm::mat4 getProjectionMatrix() const;

    /* =========================================================================
     * Configuration
     * ========================================================================= */
    void setAspectRatio(float ratio);
    void setFollowDistance(float distance) { followDistance_ = distance; }
    void setHeight(float height) { height_ = height; }

    /* Get camera position (for lighting calculations, etc.) */
    [[nodiscard]] glm::vec3 getPosition() const { return position_; }

private:
    /* Camera position and orientation */
    glm::vec3 position_;      /* Where the camera is in world space */
    glm::vec3 target_;        /* What point the camera is looking at */
    glm::vec3 up_;            /* Which way is "up" (usually Y-axis) */

    /* Follow camera parameters */
    float followDistance_;    /* Distance behind target */
    float height_;            /* Height above target */

    /* Projection parameters */
    float aspectRatio_;       /* Width / height of viewport */
    float fov_;               /* Field of view in degrees (how "wide" the view is) */
    float nearPlane_;         /* Closest visible distance (clips anything closer) */
    float farPlane_;          /* Farthest visible distance (clips anything beyond) */
};

#endif // CAMERA_H
