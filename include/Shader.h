/*
 * Shader.h - GLSL Shader Program Wrapper
 *
 * This class encapsulates the OpenGL shader compilation and linking process.
 *
 * GLSL (OpenGL Shading Language) shaders are small programs that run on the GPU:
 * - Vertex Shader: Runs once per vertex, transforms 3D positions to screen positions
 * - Fragment Shader: Runs once per pixel, determines the final color
 *
 * The compilation pipeline:
 *   1. Load shader source code (text files)
 *   2. Compile each shader separately
 *   3. Link shaders into a "program"
 *   4. Use the program for rendering
 */

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    /* =========================================================================
     * Constructor
     *
     * Takes paths to vertex and fragment shader source files, loads them,
     * compiles them, and links them into a shader program.
     *
     * Parameters:
     *   vertexPath   - Path to .glsl file containing vertex shader code
     *   fragmentPath - Path to .glsl file containing fragment shader code
     *
     * Example:
     *   Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
     * ========================================================================= */
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    /* Destructor - cleans up OpenGL resources */
    ~Shader();

    /* Delete copy operations - OpenGL resources shouldn't be copied */
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    /* Allow move operations for transferring ownership */
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    /* =========================================================================
     * use() - Activate this shader for subsequent draw calls
     *
     * OpenGL is a state machine - you set the "current" shader, then draw.
     * All draw calls use whatever shader is currently active.
     *
     * Example:
     *   shader.use();
     *   glDrawArrays(...);  // Uses this shader
     * ========================================================================= */
    void use() const;

    /* =========================================================================
     * Uniform Setters - Pass data from CPU to GPU shader
     *
     * "Uniforms" are variables in shaders that stay constant for all vertices/
     * pixels in a single draw call, but can change between draw calls.
     *
     * Common uses:
     *   - Transformation matrices (model, view, projection)
     *   - Colors
     *   - Time values for animation
     *   - Texture unit indices
     *
     * The name parameter must match the variable name in the GLSL code exactly.
     * ========================================================================= */

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

    /* Get the OpenGL program ID (rarely needed directly) */
    [[nodiscard]] GLuint getID() const { return programID_; }

private:
    GLuint programID_;  /* OpenGL handle to the linked shader program */

    /* =========================================================================
     * Helper Functions
     * ========================================================================= */

    /* Load shader source code from a file into a string */
    static std::string loadShaderSource(const std::string& path);

    /* Compile a single shader (vertex or fragment) and return its ID */
    static GLuint compileShader(const std::string& source, GLenum shaderType);

    /* Check for compilation/linking errors and print them */
    static void checkCompileErrors(GLuint shader, const std::string& type);
};

#endif // SHADER_H
