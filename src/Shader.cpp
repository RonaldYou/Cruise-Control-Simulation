/*
 * Shader.cpp - GLSL Shader Program Implementation
 */

#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>  /* glm::value_ptr - get raw float* from matrices */
#include <fstream>
#include <sstream>
#include <iostream>

/* =============================================================================
 * Constructor - Load, compile, and link shaders
 * ============================================================================= */
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    : programID_(0)
{
    /* Step 1: Load source code from files */
    std::string vertexSource = loadShaderSource(vertexPath);
    std::string fragmentSource = loadShaderSource(fragmentPath);

    /* Step 2: Compile each shader
     *
     * Compilation converts GLSL text into GPU-executable bytecode.
     * Each shader is compiled independently at this stage.
     */
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    /* Step 3: Create a "program" and link the shaders
     *
     * A program combines vertex + fragment shaders into a complete pipeline.
     * Linking resolves connections between shaders (vertex outputs -> fragment inputs).
     */
    programID_ = glCreateProgram();
    glAttachShader(programID_, vertexShader);   /* Add vertex shader to program */
    glAttachShader(programID_, fragmentShader); /* Add fragment shader to program */
    glLinkProgram(programID_);                  /* Link them together */

    /* Check for linking errors */
    checkCompileErrors(programID_, "PROGRAM");

    /* Step 4: Clean up individual shaders
     *
     * After linking, the compiled shader objects are no longer needed.
     * The program contains the final linked code.
     */
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

/* =============================================================================
 * Destructor - Release OpenGL resources
 * ============================================================================= */
Shader::~Shader() {
    if (programID_ != 0) {
        glDeleteProgram(programID_);
    }
}

/* =============================================================================
 * Move Constructor - Transfer ownership of OpenGL resources
 * ============================================================================= */
Shader::Shader(Shader&& other) noexcept
    : programID_(other.programID_)
{
    other.programID_ = 0;  /* Prevent other's destructor from deleting */
}

/* =============================================================================
 * Move Assignment - Transfer ownership
 * ============================================================================= */
Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        /* Clean up our current resources */
        if (programID_ != 0) {
            glDeleteProgram(programID_);
        }
        /* Take ownership of other's resources */
        programID_ = other.programID_;
        other.programID_ = 0;
    }
    return *this;
}

/* =============================================================================
 * use() - Activate this shader program
 *
 * OpenGL maintains a "current" shader program. All subsequent draw calls
 * use this shader until you call glUseProgram with a different ID.
 * ============================================================================= */
void Shader::use() const {
    glUseProgram(programID_);
}

/* =============================================================================
 * Uniform Setters
 *
 * Each setter:
 * 1. Queries the uniform's "location" (an integer index assigned by OpenGL)
 * 2. Uploads the value to that location
 *
 * glGetUniformLocation returns -1 if the uniform doesn't exist or was
 * optimized out by the compiler (unused uniforms are removed).
 * ============================================================================= */

void Shader::setBool(const std::string& name, bool value) const {
    /* Booleans are passed as integers (0 = false, 1 = true) */
    glUniform1i(glGetUniformLocation(programID_, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const {
    /* Used for texture unit indices, integer flags, etc. */
    glUniform1i(glGetUniformLocation(programID_, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    /* Single float: time, intensity, etc. */
    glUniform1f(glGetUniformLocation(programID_, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    /* 2D vector: texture coordinates, 2D positions */
    glUniform2f(glGetUniformLocation(programID_, name.c_str()), value.x, value.y);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    /* 3D vector: positions, colors (RGB), directions */
    glUniform3f(glGetUniformLocation(programID_, name.c_str()), value.x, value.y, value.z);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    /* 4D vector: colors with alpha (RGBA), homogeneous coordinates */
    glUniform4f(glGetUniformLocation(programID_, name.c_str()), value.x, value.y, value.z, value.w);
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    /*
     * 4x4 matrix: transformations (model, view, projection)
     *
     * Parameters:
     *   location: Where to put it
     *   count: 1 (we're setting one matrix)
     *   transpose: GL_FALSE (GLM matrices are already in column-major order)
     *   value: Pointer to the 16 floats
     *
     * glm::value_ptr() returns a pointer to the first element of the matrix
     */
    glUniformMatrix4fv(glGetUniformLocation(programID_, name.c_str()),
                       1, GL_FALSE, glm::value_ptr(value));
}

/* =============================================================================
 * loadShaderSource - Read shader code from file
 * ============================================================================= */
std::string Shader::loadShaderSource(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR::SHADER::FILE_NOT_FOUND: " << path << std::endl;
        return "";
    }

    /* Read entire file into a stringstream, then convert to string */
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/* =============================================================================
 * compileShader - Compile GLSL source into a shader object
 * ============================================================================= */
GLuint Shader::compileShader(const std::string& source, GLenum shaderType) {
    /* Create a shader object of the specified type */
    GLuint shader = glCreateShader(shaderType);

    /* Set the source code
     *
     * Parameters:
     *   shader: The shader object to modify
     *   count: Number of strings (1 - we have one source string)
     *   string: Array of pointers to source strings
     *   length: Array of lengths, or NULL for null-terminated strings
     */
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);

    /* Compile the source code into GPU bytecode */
    glCompileShader(shader);

    /* Check for errors */
    std::string type = (shaderType == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    checkCompileErrors(shader, type);

    return shader;
}

/* =============================================================================
 * checkCompileErrors - Query and print any compilation/linking errors
 * ============================================================================= */
void Shader::checkCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    char infoLog[1024];

    if (type != "PROGRAM") {
        /* Check shader compilation status */
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                      << infoLog << "\n"
                      << "-- --------------------------------------------------- --"
                      << std::endl;
        }
    } else {
        /* Check program linking status */
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR\n"
                      << infoLog << "\n"
                      << "-- --------------------------------------------------- --"
                      << std::endl;
        }
    }
}
