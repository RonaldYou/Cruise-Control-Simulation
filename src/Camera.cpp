/*
 * Camera.cpp - 3rd Person Follow Camera Implementation
 */

#include "Camera.h"

/* =============================================================================
 * Constructor - Initialize camera with default values
 * ============================================================================= */
Camera::Camera(float followDistance, float height, float aspectRatio)
    : position_(0.0f, height, -followDistance)  /* Start behind origin */
    , target_(0.0f, 0.0f, 0.0f)                 /* Looking at origin */
    , up_(0.0f, 1.0f, 0.0f)                     /* Y-axis is up */
    , followDistance_(followDistance)
    , height_(height)
    , aspectRatio_(aspectRatio)
    , fov_(45.0f)                               /* 45 degrees - standard for games */
    , nearPlane_(0.1f)                          /* Don't render closer than 0.1 units */
    , farPlane_(1000.0f)                        /* Don't render farther than 1000 units */
{
}

/* =============================================================================
 * update() - Position camera behind and above the target
 *
 * The camera follows the car from behind. We calculate the camera position
 * by going backwards from the car's position in the opposite of its facing
 * direction, then adding height.
 *
 * Example:
 *   Car at (0, 0, 50) facing direction (0, 0, 1) [forward along +Z]
 *   followDistance = 10, height = 5
 *
 *   Camera position = (0, 0, 50) - (0, 0, 1) * 10 + (0, 5, 0)
 *                   = (0, 0, 50) - (0, 0, 10) + (0, 5, 0)
 *                   = (0, 5, 40)  [behind and above the car]
 * ============================================================================= */
void Camera::update(const glm::vec3& targetPos, const glm::vec3& targetDir, float dt) {
    (void)dt;  /* Not using smooth interpolation for now, but parameter kept for future use */

    /*
     * Calculate where the camera should be:
     * 1. Start at target position
     * 2. Move backwards (opposite of target direction) by followDistance
     * 3. Move up by height
     */
    glm::vec3 offset = -targetDir * followDistance_;  /* Go backwards from target direction */
    offset.y += height_;                               /* Add height */

    position_ = targetPos + offset;  /* Final camera position */
    target_ = targetPos;             /* Camera looks at the car */

    /*
     * Note: For smoother camera movement, you could interpolate:
     *   position_ = glm::mix(position_, desiredPosition, smoothFactor * dt);
     * This prevents jarring camera movement when the car turns quickly.
     */
}

/* =============================================================================
 * getViewMatrix() - Create the view transformation matrix
 *
 * glm::lookAt() creates a matrix that:
 * 1. Translates the world so the camera is at the origin
 * 2. Rotates the world so the camera looks down the -Z axis
 *
 * Parameters:
 *   eye:    Camera position in world space
 *   center: Point the camera is looking at
 *   up:     Which direction is "up" (prevents camera roll)
 *
 * The resulting matrix transforms world coordinates to "view space"
 * (also called "camera space" or "eye space").
 * ============================================================================= */
glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position_, target_, up_);
}

/* =============================================================================
 * getProjectionMatrix() - Create the projection matrix
 *
 * Returns either perspective or orthographic projection based on mode.
 *
 * PERSPECTIVE (default):
 * glm::perspective() creates a matrix that applies perspective division
 * (farther objects appear smaller). Used for 3rd-person view.
 *
 * ORTHOGRAPHIC:
 * glm::ortho() creates a matrix where parallel lines stay parallel
 * (no perspective distortion). Used for bird's eye and side views.
 * ============================================================================= */
glm::mat4 Camera::getProjectionMatrix() const {
    if (isOrthographic_) {
        return glm::ortho(
            orthoLeft_, orthoRight_,   /* Left, right bounds */
            orthoBottom_, orthoTop_,   /* Bottom, top bounds */
            nearPlane_, farPlane_      /* Near, far clip planes */
        );
    } else {
        return glm::perspective(
            glm::radians(fov_),  /* Convert degrees to radians */
            aspectRatio_,         /* Prevent distortion */
            nearPlane_,           /* Near clip distance */
            farPlane_             /* Far clip distance */
        );
    }
}

/* =============================================================================
 * setAspectRatio() - Update aspect ratio (call when window is resized)
 *
 * If the window changes size, the aspect ratio must be updated or the
 * image will appear stretched or squished.
 * ============================================================================= */
void Camera::setAspectRatio(float ratio) {
    aspectRatio_ = ratio;
}

/* =============================================================================
 * setOrthographic() - Configure orthographic projection
 *
 * Orthographic projection has no perspective (parallel lines stay parallel).
 * Useful for top-down and side views where consistent scale is important.
 * ============================================================================= */
void Camera::setOrthographic(float left, float right, float bottom, float top, float near, float far) {
    isOrthographic_ = true;
    orthoLeft_ = left;
    orthoRight_ = right;
    orthoBottom_ = bottom;
    orthoTop_ = top;
    nearPlane_ = near;
    farPlane_ = far;
}

/* =============================================================================
 * setPerspective() - Configure perspective projection
 *
 * Returns to standard perspective projection (distant objects appear smaller).
 * ============================================================================= */
void Camera::setPerspective(float fov, float aspectRatio) {
    isOrthographic_ = false;
    fov_ = fov;
    aspectRatio_ = aspectRatio;
}

/* =============================================================================
 * setPositionAndTarget() - Manually position the camera
 *
 * Used for fixed-angle views (bird's eye, side view) where the camera
 * doesn't follow behind a target.
 * ============================================================================= */
void Camera::setPositionAndTarget(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
    position_ = position;
    target_ = target;
    up_ = up;
}
