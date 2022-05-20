#include "Texture.h"

void CreateTexture(Texture& texture, GLenum unit) {
	glActiveTexture(unit);
	glGenTextures(1, &texture.id);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.tex.GetWidth(), texture.tex.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.tex.GetBits());
}