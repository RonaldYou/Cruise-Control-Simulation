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
 * getProjectionMatrix() - Create the perspective projection matrix
 *
 * glm::perspective() creates a matrix that:
 * 1. Applies perspective division (farther = smaller)
 * 2. Maps the viewing frustum to a normalized cube [-1, 1]
 *
 * Parameters:
 *   fov:    Field of view angle (in radians) - how "wide" the view is
 *           45 degrees is standard; larger = wider/more distorted
 *   aspect: Width/height ratio - prevents stretching
 *   near:   Near clipping plane - objects closer are invisible
 *   far:    Far clipping plane - objects farther are invisible
 *
 * The "frustum" is a truncated pyramid shape representing visible space:
 *
 *              Far plane
 *           _______________
 *          /               \
 *         /                 \
 *        /                   \
 *       /                     \
 *      /_______________________\
 *      |       Near plane      |
 *      |         (eye)         |
 *
 * Everything inside this shape is visible; everything outside is clipped.
 * ============================================================================= */
glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(
        glm::radians(fov_),  /* Convert degrees to radians (OpenGL uses radians) */
        aspectRatio_,         /* Prevent distortion based on window shape */
        nearPlane_,           /* Near clip distance */
        farPlane_             /* Far clip distance */
    );
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
