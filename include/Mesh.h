/*
 * Mesh.h - 3D Geometry Container
 *
 * A Mesh stores vertices (3D points with attributes) and optionally indices
 * (which vertices form triangles). It handles uploading this data to the GPU.
 *
 * Vertex data is stored in a Vertex Buffer Object (VBO) on the GPU.
 * The vertex format is stored in a Vertex Array Object (VAO).
 *
 * Our vertex format (per vertex):
 *   - Position: 3 floats (x, y, z)
 *   - Normal:   3 floats (nx, ny, nz) - for lighting
 *   - Color:    3 floats (r, g, b)    - vertex color
 *
 * Total: 9 floats = 36 bytes per vertex
 */

#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

/* =============================================================================
 * Vertex Structure
 *
 * Each vertex has:
 *   - position: Where the vertex is in 3D space
 *   - normal:   Direction perpendicular to surface (for lighting)
 *   - color:    RGB color of this vertex
 * ============================================================================= */
struct Vertex {
    glm::vec3 position;  /* 3D position (x, y, z) */
    glm::vec3 normal;    /* Surface normal for lighting calculations */
    glm::vec3 color;     /* RGB color (0.0 to 1.0 per channel) */
};

class Mesh {
public:
    /* =========================================================================
     * Constructor - Create mesh from vertex data
     *
     * Parameters:
     *   vertices - Array of vertex data
     *   indices  - (Optional) Which vertices form triangles
     *              If empty, vertices are used directly (every 3 = triangle)
     * ========================================================================= */
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices = {});

    /* Destructor - Clean up GPU resources */
    ~Mesh();

    /* Delete copy (GPU resources shouldn't be duplicated) */
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    /* Allow move */
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    /* =========================================================================
     * draw() - Render this mesh
     *
     * Binds the VAO and issues a draw call. The currently active shader
     * will process these vertices.
     * ========================================================================= */
    void draw() const;

    /* =========================================================================
     * Factory Methods - Create common shapes
     *
     * These static methods create meshes for common primitives.
     * ========================================================================= */

    /* Create a simple box (for the car) */
    static Mesh createBox(float width, float height, float depth, const glm::vec3& color);

    /* Create a flat quad (for road segments) */
    static Mesh createQuad(float width, float depth, const glm::vec3& color);

    /* Create a road segment with lane markings */
    static Mesh createRoadSegment(float width, float length, const glm::vec3& roadColor);

    /* Create dashed center line for road */
    static Mesh createDashedLine(float length, float dashLength, float gapLength,
                                  float width, float yOffset, const glm::vec3& color);

    /* Create solid edge line for road */
    static Mesh createSolidLine(float length, float width, float xOffset,
                                 float yOffset, const glm::vec3& color);

    /* Create a 2D quad for UI elements (screen space) */
    static Mesh createQuad2D(float x, float y, float width, float height, const glm::vec3& color);

private:
    GLuint VAO_;           /* Vertex Array Object - stores vertex format */
    GLuint VBO_;           /* Vertex Buffer Object - stores vertex data */
    GLuint EBO_;           /* Element Buffer Object - stores indices (optional) */
    GLsizei vertexCount_;  /* Number of vertices (for glDrawArrays) */
    GLsizei indexCount_;   /* Number of indices (for glDrawElements) */
    bool useIndices_;      /* Whether to use indexed drawing */

    /* Set up the VAO with our vertex format */
    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
};

#endif // MESH_H
