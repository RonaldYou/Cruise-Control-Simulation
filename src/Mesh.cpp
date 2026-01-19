/*
 * Mesh.cpp - 3D Geometry Implementation
 */

#include "Mesh.h"
#include <cstddef>  /* For offsetof macro */

/* =============================================================================
 * Constructor - Upload vertex data to GPU
 * ============================================================================= */
Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : VAO_(0), VBO_(0), EBO_(0)
    , vertexCount_(static_cast<GLsizei>(vertices.size()))
    , indexCount_(static_cast<GLsizei>(indices.size()))
    , useIndices_(!indices.empty())
{
    setupMesh(vertices, indices);
}

/* =============================================================================
 * Destructor - Release GPU resources
 * ============================================================================= */
Mesh::~Mesh() {
    if (VAO_ != 0) {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        if (useIndices_) {
            glDeleteBuffers(1, &EBO_);
        }
    }
}

/* =============================================================================
 * Move Constructor
 * ============================================================================= */
Mesh::Mesh(Mesh&& other) noexcept
    : VAO_(other.VAO_), VBO_(other.VBO_), EBO_(other.EBO_)
    , vertexCount_(other.vertexCount_), indexCount_(other.indexCount_)
    , useIndices_(other.useIndices_)
{
    other.VAO_ = 0;
    other.VBO_ = 0;
    other.EBO_ = 0;
}

/* =============================================================================
 * Move Assignment
 * ============================================================================= */
Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        /* Clean up our resources */
        if (VAO_ != 0) {
            glDeleteVertexArrays(1, &VAO_);
            glDeleteBuffers(1, &VBO_);
            if (useIndices_) glDeleteBuffers(1, &EBO_);
        }
        /* Take other's resources */
        VAO_ = other.VAO_;
        VBO_ = other.VBO_;
        EBO_ = other.EBO_;
        vertexCount_ = other.vertexCount_;
        indexCount_ = other.indexCount_;
        useIndices_ = other.useIndices_;
        other.VAO_ = 0;
        other.VBO_ = 0;
        other.EBO_ = 0;
    }
    return *this;
}

/* =============================================================================
 * setupMesh() - Create GPU buffers and configure vertex attributes
 *
 * This is where we:
 * 1. Create buffer objects (VBO, VAO, optionally EBO)
 * 2. Upload vertex data to GPU
 * 3. Tell OpenGL how to interpret the vertex data
 * ============================================================================= */
void Mesh::setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    /*
     * Step 1: Generate buffer names (IDs)
     *
     * OpenGL uses integer "names" to identify objects. glGen* creates these IDs.
     * The objects don't exist yet - just the names are reserved.
     */
    glGenVertexArrays(1, &VAO_);  /* Create 1 VAO name */
    glGenBuffers(1, &VBO_);       /* Create 1 VBO name */

    /*
     * Step 2: Bind the VAO
     *
     * "Binding" makes an object "current". All subsequent vertex attribute
     * configuration will be stored in this VAO.
     *
     * Think of it like selecting a document to edit - all changes go to that doc.
     */
    glBindVertexArray(VAO_);

    /*
     * Step 3: Bind VBO and upload vertex data
     *
     * GL_ARRAY_BUFFER is the target for vertex attribute data.
     * glBufferData copies our vertex array from CPU RAM to GPU VRAM.
     *
     * GL_STATIC_DRAW hints that we'll set data once and use it many times.
     * (Other options: GL_DYNAMIC_DRAW for frequently changing data)
     */
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER,                           /* Target */
                 vertices.size() * sizeof(Vertex),          /* Size in bytes */
                 vertices.data(),                           /* Pointer to data */
                 GL_STATIC_DRAW);                           /* Usage hint */

    /*
     * Step 4: (Optional) Bind EBO and upload index data
     *
     * Indices let us reuse vertices. Instead of duplicating vertex data,
     * we list which vertices form each triangle.
     *
     * Without indices: 6 vertices for a quad (2 triangles, 3 verts each)
     * With indices: 4 vertices + 6 indices (corners reused)
     */
    if (useIndices_) {
        glGenBuffers(1, &EBO_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     indices.size() * sizeof(unsigned int),
                     indices.data(),
                     GL_STATIC_DRAW);
    }

    /*
     * Step 5: Configure vertex attributes
     *
     * We need to tell OpenGL how our Vertex struct is laid out in memory:
     *
     *   Vertex structure (36 bytes total):
     *   ┌──────────────┬──────────────┬──────────────┐
     *   │   position   │    normal    │    color     │
     *   │  (12 bytes)  │  (12 bytes)  │  (12 bytes)  │
     *   │  offset: 0   │  offset: 12  │  offset: 24  │
     *   └──────────────┴──────────────┴──────────────┘
     *
     * glVertexAttribPointer parameters:
     *   index:      Attribute location (matches shader: layout(location = X))
     *   size:       Number of components (3 for vec3)
     *   type:       Data type (GL_FLOAT)
     *   normalized: Should integers be normalized to [0,1]? (No for floats)
     *   stride:     Bytes between consecutive vertices (sizeof(Vertex))
     *   pointer:    Byte offset of this attribute within the vertex
     */

    /* Attribute 0: Position (location = 0 in shader) */
    glVertexAttribPointer(0,                              /* Attribute index */
                          3,                              /* 3 components (x,y,z) */
                          GL_FLOAT,                       /* Data type */
                          GL_FALSE,                       /* Don't normalize */
                          sizeof(Vertex),                 /* Stride: bytes to next vertex */
                          (void*)offsetof(Vertex, position)); /* Offset within vertex */
    glEnableVertexAttribArray(0);  /* Enable this attribute slot */

    /* Attribute 1: Normal (location = 1 in shader) */
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    /* Attribute 2: Color (location = 2 in shader) */
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);

    /*
     * Step 6: Unbind VAO (optional but good practice)
     *
     * Prevents accidental modification of this VAO.
     */
    glBindVertexArray(0);
}

/* =============================================================================
 * draw() - Render this mesh
 *
 * Binds the VAO (which remembers all our vertex format settings) and
 * issues a draw call.
 * ============================================================================= */
void Mesh::draw() const {
    glBindVertexArray(VAO_);

    if (useIndices_) {
        /*
         * glDrawElements: Draw using index buffer
         *
         * Parameters:
         *   mode:    Primitive type (triangles)
         *   count:   Number of indices to use
         *   type:    Data type of indices
         *   indices: Offset into index buffer (0 = start)
         */
        glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
    } else {
        /*
         * glDrawArrays: Draw vertices directly
         *
         * Parameters:
         *   mode:  Primitive type
         *   first: Starting vertex index
         *   count: Number of vertices to draw
         */
        glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
    }

    glBindVertexArray(0);
}

/* =============================================================================
 * createBox() - Generate a box mesh (for the car)
 *
 * Creates a 3D box centered at origin with the given dimensions.
 * Each face has outward-pointing normals for proper lighting.
 * ============================================================================= */
Mesh Mesh::createBox(float width, float height, float depth, const glm::vec3& color) {
    float w = width / 2.0f;   /* Half-width */
    float h = height / 2.0f;  /* Half-height */
    float d = depth / 2.0f;   /* Half-depth */

    /*
     * A box has 6 faces, each face needs 4 vertices (can't share because normals differ).
     * Total: 24 vertices, 36 indices (6 faces * 2 triangles * 3 vertices)
     */
    std::vector<Vertex> vertices = {
        /* Front face (normal pointing towards +Z) */
        {{-w, -h,  d}, { 0,  0,  1}, color},  /* Bottom-left */
        {{ w, -h,  d}, { 0,  0,  1}, color},  /* Bottom-right */
        {{ w,  h,  d}, { 0,  0,  1}, color},  /* Top-right */
        {{-w,  h,  d}, { 0,  0,  1}, color},  /* Top-left */

        /* Back face (normal pointing towards -Z) */
        {{ w, -h, -d}, { 0,  0, -1}, color},
        {{-w, -h, -d}, { 0,  0, -1}, color},
        {{-w,  h, -d}, { 0,  0, -1}, color},
        {{ w,  h, -d}, { 0,  0, -1}, color},

        /* Top face (normal pointing towards +Y) */
        {{-w,  h,  d}, { 0,  1,  0}, color},
        {{ w,  h,  d}, { 0,  1,  0}, color},
        {{ w,  h, -d}, { 0,  1,  0}, color},
        {{-w,  h, -d}, { 0,  1,  0}, color},

        /* Bottom face (normal pointing towards -Y) */
        {{-w, -h, -d}, { 0, -1,  0}, color},
        {{ w, -h, -d}, { 0, -1,  0}, color},
        {{ w, -h,  d}, { 0, -1,  0}, color},
        {{-w, -h,  d}, { 0, -1,  0}, color},

        /* Right face (normal pointing towards +X) */
        {{ w, -h,  d}, { 1,  0,  0}, color},
        {{ w, -h, -d}, { 1,  0,  0}, color},
        {{ w,  h, -d}, { 1,  0,  0}, color},
        {{ w,  h,  d}, { 1,  0,  0}, color},

        /* Left face (normal pointing towards -X) */
        {{-w, -h, -d}, {-1,  0,  0}, color},
        {{-w, -h,  d}, {-1,  0,  0}, color},
        {{-w,  h,  d}, {-1,  0,  0}, color},
        {{-w,  h, -d}, {-1,  0,  0}, color},
    };

    /* Indices: 2 triangles per face, 6 faces */
    std::vector<unsigned int> indices;
    for (unsigned int face = 0; face < 6; ++face) {
        unsigned int base = face * 4;
        /* First triangle of face */
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        /* Second triangle of face */
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    return Mesh(vertices, indices);
}

/* =============================================================================
 * createQuad() - Generate a flat quad lying on the XZ plane
 *
 * Useful for road segments, ground planes, etc.
 * The quad is centered at origin, lying flat (Y=0), facing up.
 * ============================================================================= */
Mesh Mesh::createQuad(float width, float depth, const glm::vec3& color) {
    float w = width / 2.0f;
    float d = depth / 2.0f;

    std::vector<Vertex> vertices = {
        {{-w, 0, -d}, {0, 1, 0}, color},  /* Back-left */
        {{ w, 0, -d}, {0, 1, 0}, color},  /* Back-right */
        {{ w, 0,  d}, {0, 1, 0}, color},  /* Front-right */
        {{-w, 0,  d}, {0, 1, 0}, color},  /* Front-left */
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,  /* First triangle */
        0, 2, 3   /* Second triangle */
    };

    return Mesh(vertices, indices);
}

/* =============================================================================
 * createRoadSegment() - Generate a road with 2 lanes
 *
 * Creates a road segment with:
 * - Dark gray asphalt base
 * - White dashed center line (implied by color variation)
 * - Edges slightly darker
 * ============================================================================= */
Mesh Mesh::createRoadSegment(float width, float length, const glm::vec3& roadColor) {
    float w = width / 2.0f;
    float l = length / 2.0f;

    /* Road surface - simple flat quad for now */
    std::vector<Vertex> vertices = {
        {{-w, 0, -l}, {0, 1, 0}, roadColor},
        {{ w, 0, -l}, {0, 1, 0}, roadColor},
        {{ w, 0,  l}, {0, 1, 0}, roadColor},
        {{-w, 0,  l}, {0, 1, 0}, roadColor},
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,
        0, 2, 3
    };

    return Mesh(vertices, indices);
}

/* =============================================================================
 * createDashedLine() - Generate a dashed center line for the road
 *
 * Creates multiple small quads along the Z axis to form dashed line pattern.
 * Each dash is a small rectangle lying flat on the road.
 * ============================================================================= */
Mesh Mesh::createDashedLine(float length, float dashLength, float gapLength,
                             float width, float yOffset, const glm::vec3& color) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float w = width / 2.0f;
    float halfLength = length / 2.0f;
    float z = -halfLength;
    unsigned int vertexIndex = 0;

    /* Create dashes along the length of the road */
    while (z < halfLength) {
        float dashEnd = std::min(z + dashLength, halfLength);

        /* Each dash is a quad (4 vertices, 2 triangles) */
        vertices.push_back({{-w, yOffset, z},       {0, 1, 0}, color});
        vertices.push_back({{ w, yOffset, z},       {0, 1, 0}, color});
        vertices.push_back({{ w, yOffset, dashEnd}, {0, 1, 0}, color});
        vertices.push_back({{-w, yOffset, dashEnd}, {0, 1, 0}, color});

        /* Two triangles for the quad */
        indices.push_back(vertexIndex + 0);
        indices.push_back(vertexIndex + 1);
        indices.push_back(vertexIndex + 2);
        indices.push_back(vertexIndex + 0);
        indices.push_back(vertexIndex + 2);
        indices.push_back(vertexIndex + 3);

        vertexIndex += 4;
        z += dashLength + gapLength;  /* Move to next dash position */
    }

    return Mesh(vertices, indices);
}

/* =============================================================================
 * createSolidLine() - Generate a solid edge line for the road
 *
 * Creates a single long rectangle along the Z axis at a given X offset.
 * ============================================================================= */
Mesh Mesh::createSolidLine(float length, float width, float xOffset,
                            float yOffset, const glm::vec3& color) {
    float w = width / 2.0f;
    float l = length / 2.0f;

    std::vector<Vertex> vertices = {
        {{xOffset - w, yOffset, -l}, {0, 1, 0}, color},
        {{xOffset + w, yOffset, -l}, {0, 1, 0}, color},
        {{xOffset + w, yOffset,  l}, {0, 1, 0}, color},
        {{xOffset - w, yOffset,  l}, {0, 1, 0}, color},
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,
        0, 2, 3
    };

    return Mesh(vertices, indices);
}

/* =============================================================================
 * createQuad2D() - Generate a 2D quad for UI elements
 *
 * Creates a quad in normalized device coordinates (-1 to 1) for 2D UI.
 * The quad is specified with bottom-left position (x, y) and dimensions.
 * Coordinates are in screen space: x,y in [-1,1], origin at center.
 * ============================================================================= */
Mesh Mesh::createQuad2D(float x, float y, float width, float height, const glm::vec3& color) {
    /* 2D quad lies in XY plane at Z=0 */
    std::vector<Vertex> vertices = {
        {{x,         y,          0}, {0, 0, 1}, color},  /* Bottom-left */
        {{x + width, y,          0}, {0, 0, 1}, color},  /* Bottom-right */
        {{x + width, y + height, 0}, {0, 0, 1}, color},  /* Top-right */
        {{x,         y + height, 0}, {0, 0, 1}, color},  /* Top-left */
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,
        0, 2, 3
    };

    return Mesh(vertices, indices);
}
