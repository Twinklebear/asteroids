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
class TextureAtlas {
	GLuint texture;
	size_t width, height;
	//Map of the uvs for each subtexture, accessed by subtexture name from
	//the XML document
	std::unordered_map<std::string, std::array<glm::vec2, 4>> images;

public:
	using const_iterator =
		std::unordered_map<std::string, std::array<glm::vec2, 4>>::const_iterator;

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
	 * Bind the texure to the texture 2d target
	 */
	void bind();
	/*
	 * Get the floating point uv coordinates for the location
	 * of some image within the atlas, by name
	 * uvs will be { bottom left, bottom right, top left, top right }
	 * returns all -1 if the name isn't found
	 */
	std::array<glm::vec2, 4> uvs(const std::string &name) const;
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
	 */
	void load(const std::string &file);
	/*
	 * Load the information about the images in the atlas from
	 * the children of the <TextureAtlas> node passed
	 */
	void load(tinyxml2::XMLNode *node);
};
/*
 * Map tinyxml errors to printable strings
 */
std::string get_xml_error(const tinyxml2::XMLError &error);

#endif

