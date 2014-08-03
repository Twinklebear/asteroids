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
 *
 * The textures will be loaded into a 2d texture array in the order they were
 * passed to the constructor
 *
 * Names of subtextures/sprites must be unique throughout the entire array
 * to avoid name collisions
 *
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
	std::unordered_map<std::string, std::array<glm::vec3, 4>> images;

public:
	using const_iterator =
		std::unordered_map<std::string, std::array<glm::vec3, 4>>::const_iterator;
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
	 * Bind the texture to the 2d texture array target
	 */
	void bind();
	/*
	 * Get the floating point uv coordinates for the location
	 * of some image within the atlas array, by name
	 * uvs will be { bottom left, bottom right, top left, top right }
	 * returns all -1 if not found
	 */
	std::array<glm::vec3, 4> uvs(const std::string &name) const;
	/*
	 * Check if an image with some name is contained in this atlas
	 */
	bool has_image(const std::string &name) const;
	/*
	 * Get a const iterator to the beginning/end of the list of subtextures
	 */
	const_iterator cbegin() const;
	const_iterator cend() const;
	/*
	 * Get the number of subtextures in this texture atlas
	 */
	size_t size() const;

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
	/*
	 * Scale the sizes into normalized uv coordinates range
	 * and set the y axis to match OpenGL
	 * We need to defer this pass on the TextureAtlasArray
	 * since we have to wait til we've read all the XML documents
	 * passed to find the image files we're loading and thus
	 * find their size
	 */
	void scale_uvs();
};

#endif

