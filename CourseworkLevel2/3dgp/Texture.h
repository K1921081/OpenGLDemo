#ifndef TEXTURE_H
#define TEXTURE_H

#include "GL/3dgl.h"

struct Texture {
	_3dgl::C3dglBitmap tex;
	GLuint id;
};

void CreateTexture(Texture& texture, GLenum unit);

#endif