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
// --- OpenGL 2.0 functions
// --------------------------------------------------

extern bool MYLLY_GL_2_0;

typedef void ( EXTAPIENTRY *glDrawBuffers_t )( GLsizei n, const GLenum* bufs );

glDrawBuffers_t __glDrawBuffers;

#ifdef _WIN32
#define glDrawBuffers __glDrawBuffers
#endif
