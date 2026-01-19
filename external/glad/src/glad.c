/*
 * GLAD Implementation
 *
 * This file defines all OpenGL function pointers and the loader function.
 * When you call gladLoadGLLoader(), it queries the graphics driver for each
 * function's address and stores it in these pointers.
 */

#include <glad/glad.h>
#include <stddef.h>  /* For NULL */

/* =============================================================================
 * Function Pointer Definitions
 *
 * These start as NULL. After gladLoadGLLoader() runs, they point to the
 * actual driver implementations. This is how OpenGL works - functions aren't
 * linked at compile time, they're loaded at runtime from the GPU driver.
 * ============================================================================= */

/* Core rendering functions */
PFNGLCLEARPROC glClear = NULL;                     /* Clear buffers (color, depth, stencil) */
PFNGLCLEARCOLORPROC glClearColor = NULL;           /* Set the clear color (background) */
PFNGLVIEWPORTPROC glViewport = NULL;               /* Set rendering viewport dimensions */
PFNGLENABLEPROC glEnable = NULL;                   /* Enable a capability (depth test, etc.) */
PFNGLDISABLEPROC glDisable = NULL;                 /* Disable a capability */
PFNGLBLENDFUNCPROC glBlendFunc = NULL;             /* Configure alpha blending */
PFNGLDEPTHFUNCPROC glDepthFunc = NULL;             /* Configure depth testing comparison */
PFNGLCULLFACEPROC glCullFace = NULL;               /* Set which faces to cull (front/back) */
PFNGLPOLYGONMODEPROC glPolygonMode = NULL;         /* Wireframe vs filled rendering */
PFNGLGETERRORPROC glGetError = NULL;               /* Check for OpenGL errors */
PFNGLGETSTRINGPROC glGetString = NULL;             /* Query OpenGL version, vendor, etc. */

/* Buffer Object functions - manage GPU memory */
PFNGLGENBUFFERSPROC glGenBuffers = NULL;           /* Create buffer object names (IDs) */
PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;     /* Delete buffer objects */
PFNGLBINDBUFFERPROC glBindBuffer = NULL;           /* Make a buffer the "current" one */
PFNGLBUFFERDATAPROC glBufferData = NULL;           /* Upload data to GPU buffer */
PFNGLBUFFERSUBDATAPROC glBufferSubData = NULL;     /* Update part of a buffer */

/* Vertex Array Object functions - store vertex format configuration */
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL; /* Create VAO names */
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL; /* Make a VAO current */
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL; /* Define vertex attribute format */
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;  /* Enable attribute slot */
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = NULL;

/* Drawing functions */
PFNGLDRAWARRAYSPROC glDrawArrays = NULL;           /* Draw primitives from array data */
PFNGLDRAWELEMENTSPROC glDrawElements = NULL;       /* Draw indexed primitives */

/* Shader functions - compile GPU programs */
PFNGLCREATESHADERPROC glCreateShader = NULL;       /* Create a shader object */
PFNGLDELETESHADERPROC glDeleteShader = NULL;       /* Delete a shader object */
PFNGLSHADERSOURCEPROC glShaderSource = NULL;       /* Set shader source code */
PFNGLCOMPILESHADERPROC glCompileShader = NULL;     /* Compile shader source to GPU code */
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;         /* Query shader properties */
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL; /* Get compilation error messages */

/* Program functions - link shaders into a usable program */
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;     /* Create a program object */
PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;     /* Delete a program object */
PFNGLATTACHSHADERPROC glAttachShader = NULL;       /* Attach shader to program */
PFNGLDETACHSHADERPROC glDetachShader = NULL;       /* Detach shader from program */
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;         /* Link attached shaders into program */
PFNGLUSEPROGRAMPROC glUseProgram = NULL;           /* Make program current for rendering */
PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;       /* Query program properties */
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL; /* Get linking error messages */

/* Uniform functions - pass data from CPU to shaders */
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL; /* Find uniform variable by name */
PFNGLUNIFORM1IPROC glUniform1i = NULL;             /* Set integer uniform (e.g., texture unit) */
PFNGLUNIFORM1FPROC glUniform1f = NULL;             /* Set float uniform */
PFNGLUNIFORM2FPROC glUniform2f = NULL;             /* Set vec2 uniform */
PFNGLUNIFORM3FPROC glUniform3f = NULL;             /* Set vec3 uniform (e.g., color, position) */
PFNGLUNIFORM4FPROC glUniform4f = NULL;             /* Set vec4 uniform */
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL; /* Set mat4 uniform (e.g., MVP matrix) */

/* Texture functions */
PFNGLGENTEXTURESPROC glGenTextures = NULL;         /* Create texture names */
PFNGLDELETETEXTURESPROC glDeleteTextures = NULL;   /* Delete textures */
PFNGLBINDTEXTUREPROC glBindTexture = NULL;         /* Make a texture current */
PFNGLTEXPARAMETERIPROC glTexParameteri = NULL;     /* Set texture parameters (filtering, etc.) */
PFNGLTEXIMAGE2DPROC glTexImage2D = NULL;           /* Upload image data to texture */
PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;     /* Select active texture unit */

/* =============================================================================
 * gladLoadGLLoader - The Main Loading Function
 *
 * This function takes a "loader" function as a parameter. The loader knows how
 * to ask the OS/driver for function addresses. GLFW provides glfwGetProcAddress
 * for this purpose.
 *
 * For each OpenGL function we need, we:
 * 1. Ask the loader for the function's address by name
 * 2. Cast it to the correct function pointer type
 * 3. Store it in our global variable
 *
 * Returns: 1 on success, 0 on failure
 * ============================================================================= */
int gladLoadGLLoader(GLADloadproc load) {
    /* The 'load' function takes a function name string and returns its address.
     * Example: load("glClear") returns a pointer to the driver's glClear implementation */

    if (load == NULL) {
        return 0;  /* No loader provided */
    }

    /* Load each function. The cast converts void* to the correct function pointer type.
     *
     * Why this pattern?
     * - OpenGL is a specification, not a library
     * - Each GPU vendor (NVIDIA, AMD, Intel) implements the functions differently
     * - The driver exposes functions through a lookup mechanism
     * - We query by name at runtime rather than linking at compile time
     */

    /* Core functions */
    glClear = (PFNGLCLEARPROC)load("glClear");
    glClearColor = (PFNGLCLEARCOLORPROC)load("glClearColor");
    glViewport = (PFNGLVIEWPORTPROC)load("glViewport");
    glEnable = (PFNGLENABLEPROC)load("glEnable");
    glDisable = (PFNGLDISABLEPROC)load("glDisable");
    glBlendFunc = (PFNGLBLENDFUNCPROC)load("glBlendFunc");
    glDepthFunc = (PFNGLDEPTHFUNCPROC)load("glDepthFunc");
    glCullFace = (PFNGLCULLFACEPROC)load("glCullFace");
    glPolygonMode = (PFNGLPOLYGONMODEPROC)load("glPolygonMode");
    glGetError = (PFNGLGETERRORPROC)load("glGetError");
    glGetString = (PFNGLGETSTRINGPROC)load("glGetString");

    /* Buffer functions */
    glGenBuffers = (PFNGLGENBUFFERSPROC)load("glGenBuffers");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)load("glDeleteBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)load("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)load("glBufferData");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)load("glBufferSubData");

    /* VAO functions */
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)load("glGenVertexArrays");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)load("glDeleteVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load("glBindVertexArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)load("glVertexAttribPointer");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)load("glEnableVertexAttribArray");
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)load("glDisableVertexAttribArray");

    /* Drawing functions */
    glDrawArrays = (PFNGLDRAWARRAYSPROC)load("glDrawArrays");
    glDrawElements = (PFNGLDRAWELEMENTSPROC)load("glDrawElements");

    /* Shader functions */
    glCreateShader = (PFNGLCREATESHADERPROC)load("glCreateShader");
    glDeleteShader = (PFNGLDELETESHADERPROC)load("glDeleteShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)load("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)load("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)load("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)load("glGetShaderInfoLog");

    /* Program functions */
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)load("glCreateProgram");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)load("glDeleteProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)load("glAttachShader");
    glDetachShader = (PFNGLDETACHSHADERPROC)load("glDetachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)load("glLinkProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)load("glUseProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)load("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)load("glGetProgramInfoLog");

    /* Uniform functions */
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)load("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)load("glUniform1i");
    glUniform1f = (PFNGLUNIFORM1FPROC)load("glUniform1f");
    glUniform2f = (PFNGLUNIFORM2FPROC)load("glUniform2f");
    glUniform3f = (PFNGLUNIFORM3FPROC)load("glUniform3f");
    glUniform4f = (PFNGLUNIFORM4FPROC)load("glUniform4f");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)load("glUniformMatrix4fv");

    /* Texture functions */
    glGenTextures = (PFNGLGENTEXTURESPROC)load("glGenTextures");
    glDeleteTextures = (PFNGLDELETETEXTURESPROC)load("glDeleteTextures");
    glBindTexture = (PFNGLBINDTEXTUREPROC)load("glBindTexture");
    glTexParameteri = (PFNGLTEXPARAMETERIPROC)load("glTexParameteri");
    glTexImage2D = (PFNGLTEXIMAGE2DPROC)load("glTexImage2D");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)load("glActiveTexture");

    /* Basic validation - check if essential functions loaded */
    if (glClear == NULL || glCreateShader == NULL || glCreateProgram == NULL) {
        return 0;  /* Critical functions missing - OpenGL context may be invalid */
    }

    return 1;  /* Success */
}
