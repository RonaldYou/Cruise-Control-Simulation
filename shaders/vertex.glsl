/*
 * Vertex Shader - Runs once per vertex
 *
 * This shader transforms vertex positions from model space to screen space
 * and passes data to the fragment shader.
 *
 * The transformation pipeline:
 *   Model Space (object's local coordinates)
 *        ↓ model matrix
 *   World Space (where the object is in the world)
 *        ↓ view matrix
 *   View/Camera Space (relative to camera)
 *        ↓ projection matrix
 *   Clip Space (normalized device coordinates)
 *        ↓ (automatic by GPU)
 *   Screen Space (pixels)
 */

#version 330 core
/* GLSL version 330 corresponds to OpenGL 3.3
 * "core" means we're using the core profile (no deprecated features) */

/* =============================================================================
 * Input Attributes (from vertex buffer)
 *
 * layout(location = X) specifies which attribute slot this corresponds to.
 * These must match the glVertexAttribPointer calls in Mesh.cpp:
 *   location 0 = position
 *   location 1 = normal
 *   location 2 = color
 * ============================================================================= */
layout (location = 0) in vec3 aPos;      /* Vertex position (model space) */
layout (location = 1) in vec3 aNormal;   /* Surface normal (for lighting) */
layout (location = 2) in vec3 aColor;    /* Vertex color */

/* =============================================================================
 * Output to Fragment Shader
 *
 * These are "interpolated" across the triangle's surface.
 * Each fragment (pixel) gets a smoothly interpolated value based on its
 * position relative to the three vertices.
 * ============================================================================= */
out vec3 FragPos;    /* Fragment position in world space (for lighting) */
out vec3 Normal;     /* Interpolated normal (for lighting) */
out vec3 VertexColor;/* Interpolated color */

/* =============================================================================
 * Uniforms (constant for all vertices in a draw call)
 *
 * These are set from C++ via shader.setMat4("model", matrix);
 * ============================================================================= */
uniform mat4 model;      /* Model matrix: local -> world space */
uniform mat4 view;       /* View matrix: world -> camera space */
uniform mat4 projection; /* Projection matrix: camera -> clip space */

/* =============================================================================
 * main() - Entry point, runs for each vertex
 * ============================================================================= */
void main()
{
    /*
     * Calculate world-space position
     *
     * Multiply position by model matrix to get world coordinates.
     * vec4(aPos, 1.0) converts vec3 to vec4 with w=1 (position, not direction).
     * .xyz extracts the vec3 back out.
     */
    FragPos = vec3(model * vec4(aPos, 1.0));

    /*
     * Transform normal to world space
     *
     * Normals require special handling when the model matrix has non-uniform
     * scaling. We should use the "normal matrix" (inverse transpose of model).
     * For now, we assume uniform scaling so this works:
     */
    Normal = mat3(model) * aNormal;

    /* Pass vertex color through unchanged */
    VertexColor = aColor;

    /*
     * Calculate final clip-space position
     *
     * This is the mandatory output - gl_Position determines where on screen
     * this vertex appears.
     *
     * Matrix multiplication order matters! In GLSL, it's right-to-left:
     *   projection * view * model * position
     *     ↑          ↑       ↑        ↑
     *   last      applied   first   original
     *
     * So the position is first transformed by model, then view, then projection.
     */
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
