#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include <string>
#include <unordered_map>
#include <array>
#include <SDL.h>
#include <glm/glm.hpp>
#include "gl_core_3_3.h"

/*
 * Support for working with texture atlases described by the
 * xml document produced by the packing tool used by Kenny.nl (texture packer?)
 * It's assumed that the sprite locations in the document use the top-left
 * corner as [0, 0] and the appropriate flip is applied to return
 * the correct OpenGL uv coords through the uvs member
 * The document should be XML like so:
 *
 * <TextureAtlas imagePath="image.png">
 *     <SubTexture name="sprite_1" x="0" y="50" width="100" height="125" />
 *     ...
 * </TextureAtlas>
 *
 * Note that the actual names of the tags don't matter, only that the
 * correct attributes are there.
 */
class TextureAtlas {
	GLuint texture;
	size_t width, height;
	std::unordered_map<std::string, SDL_Rect> images;

public:
	/*
	 * Load the texture atlas described by the xml file
	 * it's assumed that the imagePath attribute of the
	 * TextureAtlas element refers to an image in the same
	 * folder as the xml document being loaded
	 */
	TextureAtlas(const std::string &file);
	/*
	 * Destroy the OpenGL texture being referenced
	 */
	~TextureAtlas();
	/*
	 * Get the pixel coordinate rect for the location of some
	 * image within the atlas, by name
	 * returns { 0, 0, 0, 0} if the name isn't found
	 */
	SDL_Rect rect(const std::string &name) const;
	/*
	 * Get the floating point uv coordinates for the location
	 * of some image within the atlas, by name
	 * uvs will be { bottom left, bottom right, top left, top right }
	 * returns all 0s if the name isn't found
	 */
	std::array<glm::vec2, 4> uvs(const std::string &name) const;
	/*
	 * Check if an image with some name is contained in this atlas
	 */
	bool has_image(const std::string &name) const;

private:
	/*
	 * Load the texture atlas described by the xml file
	 */
	void load(const std::string &file);
	/*
	 * Map tinyxml errors to printable strings
	 */
	static std::string get_xml_error(const tinyxml2::XMLError &error);
};

#endif

