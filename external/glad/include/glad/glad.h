/*
 * GLAD: OpenGL Loader Generator
 * Generated from https://glad.dav1d.de/
 *
 * This header declares all OpenGL 3.3 Core functions. OpenGL functions aren't
 * exported directly by drivers - they must be loaded at runtime via function
 * pointers. GLAD handles this loading process.
 *
 * OpenGL 3.3 Core is chosen because:
 * - It's widely supported (2010+)
 * - Removes deprecated "fixed function" pipeline
 * - Forces modern shader-based rendering
 */

#ifndef GLAD_H
#define GLAD_H

#ifdef __cplusplus
extern "C" {
#endif

/* KHR platform header provides cross-platform type definitions */
#include <KHR/khrplatform.h>

/* =============================================================================
 * OpenGL Type Definitions
 * These match the types used in the OpenGL specification
 * ============================================================================= */
typedef void GLvoid;
typedef unsigned int GLenum;           /* Enumeration type (GL_TRIANGLES, etc.) */
typedef float GLfloat;                 /* 32-bit float */
typedef int GLint;                     /* 32-bit integer */
typedef int GLsizei;                   /* Size type (widths, heights, counts) */
typedef unsigned int GLbitfield;       /* Bit flags (GL_COLOR_BUFFER_BIT, etc.) */
typedef double GLdouble;               /* 64-bit float (rarely used) */
typedef unsigned int GLuint;           /* Unsigned 32-bit (object IDs) */
typedef unsigned char GLboolean;       /* Boolean (GL_TRUE/GL_FALSE) */
typedef khronos_uint8_t GLubyte;       /* Unsigned byte (0-255) */
typedef khronos_ssize_t GLsizeiptr;    /* Pointer-sized signed (buffer sizes) */
typedef khronos_intptr_t GLintptr;     /* Pointer-sized integer (offsets) */
typedef char GLchar;                   /* Character (shader source, logs) */
typedef khronos_int16_t GLshort;       /* 16-bit signed */
typedef khronos_int8_t GLbyte;         /* 8-bit signed */
typedef khronos_uint16_t GLushort;     /* 16-bit unsigned */
typedef khronos_uint16_t GLhalf;       /* 16-bit float */

/* =============================================================================
 * Boolean Constants
 * ============================================================================= */
#define GL_FALSE 0
#define GL_TRUE 1

/* =============================================================================
 * OpenGL Constants (Enums)
 * These are the "magic numbers" used throughout OpenGL
 * ============================================================================= */

/* Data types for vertex attributes and uniforms */
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406

/* Primitive types: what shape to draw from vertices */
#define GL_POINTS                         0x0000  /* Individual points */
#define GL_LINES                          0x0001  /* Pairs of vertices = lines */
#define GL_LINE_LOOP                      0x0002  /* Connected lines, last connects to first */
#define GL_LINE_STRIP                     0x0003  /* Connected lines */
#define GL_TRIANGLES                      0x0004  /* Every 3 vertices = triangle */
#define GL_TRIANGLE_STRIP                 0x0005  /* Each vertex after first 2 forms triangle */
#define GL_TRIANGLE_FAN                   0x0006  /* Fan of triangles from first vertex */

/* Buffer bit masks: which buffers to clear */
#define GL_DEPTH_BUFFER_BIT               0x00000100  /* Z-buffer */
#define GL_STENCIL_BUFFER_BIT             0x00000400  /* Stencil buffer */
#define GL_COLOR_BUFFER_BIT               0x00004000  /* Color buffer (what you see) */

/* Enable/Disable capabilities */
#define GL_DEPTH_TEST                     0x0B71  /* Z-buffer testing */
#define GL_BLEND                          0x0BE2  /* Alpha blending */
#define GL_CULL_FACE                      0x0B44  /* Back-face culling */

/* Face culling */
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_FRONT_AND_BACK                 0x0408

/* Blend factors */
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303

/* Depth function comparisons */
#define GL_LESS                           0x0201
#define GL_LEQUAL                         0x0203

/* Polygon modes */
#define GL_LINE                           0x1B01
#define GL_FILL                           0x1B02

/* Texture targets */
#define GL_TEXTURE_2D                     0x0DE1

/* Texture parameters */
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_REPEAT                         0x2901
#define GL_CLAMP_TO_EDGE                  0x812F

/* Texture formats */
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908

/* Shader types */
#define GL_VERTEX_SHADER                  0x8B31  /* Processes each vertex */
#define GL_FRAGMENT_SHADER                0x8B30  /* Processes each pixel */

/* Shader/Program parameters for glGetShaderiv/glGetProgramiv */
#define GL_COMPILE_STATUS                 0x8B81  /* Did shader compile? */
#define GL_LINK_STATUS                    0x8B82  /* Did program link? */
#define GL_INFO_LOG_LENGTH                0x8B84  /* Length of error log */

/* Buffer targets: what kind of buffer is this? */
#define GL_ARRAY_BUFFER                   0x8892  /* Vertex data */
#define GL_ELEMENT_ARRAY_BUFFER           0x8893  /* Index data */

/* Buffer usage hints: how will we use this data? */
#define GL_STATIC_DRAW                    0x88E4  /* Set once, draw many times */
#define GL_DYNAMIC_DRAW                   0x88E8  /* Modified often, drawn often */

/* Framebuffer targets */
#define GL_FRAMEBUFFER                    0x8D40
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9

/* Error codes */
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505

/* String queries */
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02

/* =============================================================================
 * OpenGL Function Declarations
 * These are function pointers that get populated by gladLoadGLLoader()
 * ============================================================================= */

/* Function pointer type definitions */
typedef void (*PFNGLCLEARPROC)(GLbitfield mask);
typedef void (*PFNGLCLEARCOLORPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (*PFNGLVIEWPORTPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (*PFNGLENABLEPROC)(GLenum cap);
typedef void (*PFNGLDISABLEPROC)(GLenum cap);
typedef void (*PFNGLBLENDFUNCPROC)(GLenum sfactor, GLenum dfactor);
typedef void (*PFNGLDEPTHFUNCPROC)(GLenum func);
typedef void (*PFNGLCULLFACEPROC)(GLenum mode);
typedef void (*PFNGLPOLYGONMODEPROC)(GLenum face, GLenum mode);
typedef GLenum (*PFNGLGETERRORPROC)(void);
typedef const GLubyte* (*PFNGLGETSTRINGPROC)(GLenum name);

/* Buffer functions */
typedef void (*PFNGLGENBUFFERSPROC)(GLsizei n, GLuint *buffers);
typedef void (*PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint *buffers);
typedef void (*PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (*PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (*PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);

/* Vertex Array Object functions */
typedef void (*PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
typedef void (*PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint *arrays);
typedef void (*PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void (*PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (*PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void (*PFNGLDISABLEVERTEXATTRIBARRAYPROC)(GLuint index);

/* Drawing functions */
typedef void (*PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);
typedef void (*PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices);

/* Shader functions */
typedef GLuint (*PFNGLCREATESHADERPROC)(GLenum type);
typedef void (*PFNGLDELETESHADERPROC)(GLuint shader);
typedef void (*PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (*PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void (*PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint *params);
typedef void (*PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

/* Program functions */
typedef GLuint (*PFNGLCREATEPROGRAMPROC)(void);
typedef void (*PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void (*PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (*PFNGLDETACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (*PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void (*PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void (*PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint *params);
typedef void (*PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

/* Uniform functions (passing data to shaders) */
typedef GLint (*PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar *name);
typedef void (*PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
typedef void (*PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);
typedef void (*PFNGLUNIFORM2FPROC)(GLint location, GLfloat v0, GLfloat v1);
typedef void (*PFNGLUNIFORM3FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (*PFNGLUNIFORM4FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (*PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

/* Texture functions */
typedef void (*PFNGLGENTEXTURESPROC)(GLsizei n, GLuint *textures);
typedef void (*PFNGLDELETETEXTURESPROC)(GLsizei n, const GLuint *textures);
typedef void (*PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
typedef void (*PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname, GLint param);
typedef void (*PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (*PFNGLACTIVETEXTUREPROC)(GLenum texture);
#define GL_TEXTURE0                       0x84C0

/* Declare the actual function pointers (defined in glad.c) */
extern PFNGLCLEARPROC glClear;
extern PFNGLCLEARCOLORPROC glClearColor;
extern PFNGLVIEWPORTPROC glViewport;
extern PFNGLENABLEPROC glEnable;
extern PFNGLDISABLEPROC glDisable;
extern PFNGLBLENDFUNCPROC glBlendFunc;
extern PFNGLDEPTHFUNCPROC glDepthFunc;
extern PFNGLCULLFACEPROC glCullFace;
extern PFNGLPOLYGONMODEPROC glPolygonMode;
extern PFNGLGETERRORPROC glGetError;
extern PFNGLGETSTRINGPROC glGetString;

extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;

extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;

extern PFNGLDRAWARRAYSPROC glDrawArrays;
extern PFNGLDRAWELEMENTSPROC glDrawElements;

extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;

extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLDETACHSHADERPROC glDetachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;

extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM2FPROC glUniform2f;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLUNIFORM4FPROC glUniform4f;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

extern PFNGLGENTEXTURESPROC glGenTextures;
extern PFNGLDELETETEXTURESPROC glDeleteTextures;
extern PFNGLBINDTEXTUREPROC glBindTexture;
extern PFNGLTEXPARAMETERIPROC glTexParameteri;
extern PFNGLTEXIMAGE2DPROC glTexImage2D;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;

/* =============================================================================
 * GLAD Initialization Function
 * Call this after creating an OpenGL context to load all function pointers
 *
 * Usage:
 *   glfwMakeContextCurrent(window);
 *   gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
 *
 * The loader function (glfwGetProcAddress) queries the driver for each
 * function's memory address.
 * ============================================================================= */
typedef void* (*GLADloadproc)(const char *name);
int gladLoadGLLoader(GLADloadproc load);

#ifdef __cplusplus
}
#endif

#endif /* GLAD_H */
