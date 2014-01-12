/**********************************************************************
 *
 * PROJECT:		Mylly GUI - OpenGL Renderer
 * FILE:		Extensions.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		An OpenGL reference renderer for Mylly GUI.
 *				Code to load OpenGL extensions.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#include "stdtypes.h"
#include <gl/GL.h>
#include <gl/glext.h>
#include <gl/GLU.h>

#ifdef _WIN32
#define EXTAPIENTRY APIENTRY
#else
#define EXTAPIENTRY
#endif

// Extension initialization procedures

void mgui_opengl_initialize_extensions( void );

// --------------------------------------------------
// --- EXT_framebuffer_object
// --------------------------------------------------

extern bool MYLLY_EXT_framebuffer_object;

typedef void ( EXTAPIENTRY *glGenFramebuffersEXT_t )( GLsizei n, GLuint* framebuffers );
typedef void ( EXTAPIENTRY *glBindFramebufferEXT_t )( GLenum target, GLuint framebuffer );
typedef void ( EXTAPIENTRY *glDeleteFramebuffersEXT_t )( GLsizei n, const GLuint* framebuffers );
typedef void ( EXTAPIENTRY *glFramebufferTextureEXT_t )( GLenum target, GLenum attachment, GLuint texture, GLint level );
typedef GLenum ( EXTAPIENTRY *glCheckFramebufferStatusEXT_t )( GLenum target );

glGenFramebuffersEXT_t __glGenFramebuffersEXT;
glBindFramebufferEXT_t __glBindFramebufferEXT;
glDeleteFramebuffersEXT_t __glDeleteFramebuffersEXT;
glFramebufferTextureEXT_t __glFramebufferTextureEXT;
glCheckFramebufferStatusEXT_t __glCheckFramebufferStatusEXT;

#define glGenFramebuffersEXT __glGenFramebuffersEXT
#define glBindFramebufferEXT __glBindFramebufferEXT
#define glDeleteFramebuffersEXT __glDeleteFramebuffersEXT
#define glFramebufferTextureEXT __glFramebufferTextureEXT
#define glCheckFramebufferStatusEXT __glCheckFramebufferStatusEXT


// --------------------------------------------------
// --- ARB_vertex_buffer_object
// --------------------------------------------------

extern bool MYLLY_ARB_vertex_buffer_object;

typedef void ( EXTAPIENTRY *glGenBuffersARB_t )( GLsizei n, GLuint* ids );
typedef void ( EXTAPIENTRY *glBindBufferARB_t )( GLenum target, GLuint id );
typedef void ( EXTAPIENTRY *glDeleteBuffersARB_t )( GLsizei n, GLuint* ids );
typedef void ( EXTAPIENTRY *glBufferDataARB_t )( GLenum target, GLsizei size, const void* data, GLenum usage );
typedef void ( EXTAPIENTRY *glBufferSubDataARB_t )( GLenum target, GLint offset, GLsizei size, void* data );
typedef void* ( EXTAPIENTRY *glMapBufferARB_t )( GLenum target, GLenum access );
typedef void* ( EXTAPIENTRY *glMapBufferRangeARB_t )( GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access );
typedef GLboolean ( EXTAPIENTRY *glUnmapBufferARB_t )( GLenum target );
typedef void ( EXTAPIENTRY *glFlushMappedBufferRangeARB_t )( GLenum target, GLintptr offset, GLsizeiptr length );

glGenBuffersARB_t __glGenBuffersARB;
glBindBufferARB_t __glBindBufferARB;
glDeleteBuffersARB_t __glDeleteBuffersARB;
glBufferDataARB_t __glBufferDataARB;
glBufferSubDataARB_t __glBufferSubDataARB;
glMapBufferARB_t __glMapBufferARB;
glMapBufferRangeARB_t __glMapBufferRangeARB;
glUnmapBufferARB_t __glUnmapBufferARB;
glFlushMappedBufferRangeARB_t __glFlushMappedBufferRangeARB;

#define glGenBuffersARB __glGenBuffersARB
#define glBindBufferARB __glBindBufferARB
#define glDeleteBuffersARB __glDeleteBuffersARB
#define glBufferDataARB __glBufferDataARB
#define glBufferSubDataARB __glBufferSubDataARB
#define glMapBufferARB __glMapBufferARB
#define glMapBufferRangeARB __glMapBufferRangeARB
#define glUnmapBufferARB __glUnmapBufferARB
#define glFlushMappedBufferRangeARB __glFlushMappedBufferRangeARB


// --------------------------------------------------
// --- OpenGL 2.0 functions
// --------------------------------------------------

extern bool MYLLY_GL_2_0;

typedef void ( EXTAPIENTRY *glDrawBuffers_t )( GLsizei n, const GLenum* bufs );

glDrawBuffers_t __glDrawBuffers;

#ifdef _WIN32
#define glDrawBuffers __glDrawBuffers
#endif
