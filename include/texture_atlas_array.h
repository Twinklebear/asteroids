#ifndef TEXTURE_ATLAS_ARRAY_H
#define TEXTURE_ATLAS_ARRAY_H

#include <string>
#include <unordered_map>
#include <array>
#include <vector>
#include <utility>
#include <initializer_list>
#include <SDL.h>
#include <glm/glm.hpp>
#include "gl_core_3_3.h"

/*
 * Support for working with arrays of texture atlases described by the
 * xml document produced by the packing tool used by Kenny.nl (texture packer?)
 * It's assumed that the sprite locations in the document use the top-left
 * corner as [0, 0] and the appropriate flip is applied to return
 * the correct OpenGL uv coords through the uvs member
 * The textures will be loaded into a 2d texture array in the order they were
 * passed to the constructor
 * The document should be XML like so:
 *
 * <TextureAtlas imagePath="image.png">
 *     <SubTexture name="sprite_1" x="0" y="50" width="100" height="125" />
 *     ...
 * </TextureAtlas>
 *
 * Or the generic TexturePacker format (with comments and xml header removed)
 *
 * <TextureAtlas imagePath="image.png">
 *     <sprite n="sprite_1" x="0" y="50" w="100" h="125" />
 *     ...
 * </TextureAtlas>
 *
 * Note that the actual names of the tags don't matter, only that the
 * correct attributes are there. Only the first TextureAtlas child will
 * be read in if multiple ones exist
 */
class TextureAtlasArray {
	GLuint texture;
	size_t width, height;
	std::unordered_map<std::string, std::pair<SDL_Rect, int>> images;

public:
	/*
	 * Load the texture atlases described by the xml files
	 * it's assumed that the imagePath attribute of the
	 * TextureAtlas element refers to an image in the same
	 * folder as the xml document being loaded
	 */
	TextureAtlasArray(const std::vector<std::string> &files);
	TextureAtlasArray(const std::initializer_list<std::string> &files);
	/*
	 * Destroy the OpenGL texture being referenced
	 */
	~TextureAtlasArray();
	/*
	 * Get the pixel coordinate rect for the location of some
	 * image within the atlas array, by name
	 * returns { [0, 0, 0, 0], -1} if the name isn't found
	 */
	std::pair<SDL_Rect, int> rect(const std::string &name) const;
	/*
	 * Get the floating point uv coordinates for the location
	 * of some image within the atlas array, by name
	 * uvs will be { bottom left, bottom right, top left, top right }
	 * returns all 0s with negative array index (z value) if not found
	 */
	std::array<glm::vec3, 4> uvs(const std::string &name) const;
	/*
	 * Check if an image with some name is contained in this atlas
	 */
	bool has_image(const std::string &name) const;

private:
	/*
	 * Load the texture atlas described by the xml file
	 * img refers to the index in the array of textures of
	 * the atlas being loaded
	 * returns the name of the image file to load for this atlas
	 */
	std::string load(const std::string &file, int img);
	/*
	 * Load the information about the images in the atlas from
	 * the children of the <TextureAtlas> node passed
	 * img refers to the index in the array of textures of
	 * the atlas being loaded
	 */
	void load(tinyxml2::XMLNode *node, int img);
};

#endif

