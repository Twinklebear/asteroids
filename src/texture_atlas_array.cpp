#include <iostream>
#include <string>
#include <unordered_map>
#include <array>
#include <vector>
#include <SDL.h>
#include <glm/glm.hpp>
#include <tinyxml2.h>
#include "gl_core_3_3.h"
#include "util.h"
#include "texture_atlas.h"
#include "texture_atlas_array.h"

TextureAtlasArray::TextureAtlasArray(const std::vector<std::string> &files){
	//Track all the image files we need to load into the array
	std::vector<std::string> img_files;
	for (size_t i = 0; i < files.size(); ++i){
		img_files.push_back(load(files.at(i), i));
	}
	texture = util::load_texture_array(img_files, &width, &height);
}
TextureAtlasArray::TextureAtlasArray(const std::initializer_list<std::string> &files){
	//Track all the image files we need to load into the array
	std::vector<std::string> img_files;
	size_t i = 0;
	for (const auto &f : files){
		img_files.push_back(load(f, i));
		++i;
	}
	texture = util::load_texture_array(img_files, &width, &height);
}
TextureAtlasArray::~TextureAtlasArray(){
	glDeleteTextures(1, &texture);
}
std::pair<SDL_Rect, int> TextureAtlasArray::rect(const std::string &name) const {
	auto f = images.find(name);
	if (f == images.end()){
		return std::make_pair(SDL_Rect{0, 0, 0, 0}, -1);
	}
	return f->second;
}
std::array<glm::vec3, 4> TextureAtlasArray::uvs(const std::string &name) const {
	auto f = images.find(name);
	std::array<glm::vec3, 4> arr;
	if (f == images.end()){
		arr.fill(glm::vec3{0, 0, -1});
		return arr;
	}
	int i = f->second.second;
	SDL_Rect r = f->second.first;
	glm::vec3 dim{width, height, 1};
	arr[0] = glm::vec3{r.x, dim.y - r.y - r.h, i} / dim;
	arr[1] = glm::vec3{r.x + r.w, dim.y - r.y - r.h, i} / dim;
	arr[2] = glm::vec3{r.x, dim.y - r.y, i} / dim;
	arr[3] = glm::vec3{r.x + r.w, dim.y - r.y, i} / dim;
	return arr;
}
bool TextureAtlasArray::has_image(const std::string &name) const {
	return images.find(name) != images.end();
}
std::string TextureAtlasArray::load(const std::string &file, int img){
	using namespace tinyxml2;
	XMLDocument doc;
	XMLError err = doc.LoadFile(file.c_str());
	if (err != XML_SUCCESS){
		std::cerr << "TextureAtlas error loading " << file << " - "
			<< get_xml_error(err) << std::endl;
		assert(err == XML_SUCCESS);
	}
	XMLNode *n = doc.FirstChildElement("TextureAtlas");
	if (n != nullptr){
		XMLElement *e = n->ToElement();
		if (!e->Attribute("imagePath")){
			std::cerr << "TextureAtlas error: loading unsupported format" << std::endl;
			assert(false);
		}
		std::string img_file = e->Attribute("imagePath");
		img_file = file.substr(0, file.rfind(util::PATH_SEP) + 1) + img_file;
		load(n, img);
		//Warn the user if there are any TextureAtlas elements following this one
		//since they'll be ignored
		if (n->NextSiblingElement("TextureAtlas")){
			std::cerr << "TextureAtlas warning: Ignoring other TextureAtlas definitions"
				<< " in " << file << std::endl;
		}
		//Return the image we need to load for this array entry
		return img_file;
	}
	else {
		std::cerr << "TextureAtlas error: " << file << " had no xml data\n";
		assert(false);
	}
	return "";
}
void TextureAtlasArray::load(tinyxml2::XMLNode *node, int img){
	using namespace tinyxml2;
	//Iterate over all the subtextures in the xml document and load their positions
	for (XMLNode *i = node->FirstChild(); i != nullptr; i = i->NextSibling()){
		//Support both the attribute format used by Kenny.NL and TexturePacker's
		//generic XML output
		XMLElement *e = i->ToElement();
		//The texture packer generic XML format
		if (e->Attribute("n") && e->Attribute("x") && e->Attribute("y")
			&& e->Attribute("w") && e->Attribute("h"))
		{
			images[e->Attribute("n")] = std::make_pair(SDL_Rect{e->IntAttribute("x"),
				e->IntAttribute("y"), e->IntAttribute("w"),
				e->IntAttribute("h")}, img);
		}
		//The format used by Kenny.NL
		else if (e->Attribute("name") && e->Attribute("x") && e->Attribute("y")
			&& e->Attribute("width") && e->Attribute("height"))
		{
			images[e->Attribute("name")] = std::make_pair(SDL_Rect{e->IntAttribute("x"),
				e->IntAttribute("y"), e->IntAttribute("width"),
				e->IntAttribute("height")}, img);
		}
		else {
			std::cerr << "TextureAtlas error: loading unsupported format" << std::endl;
			assert(false);
		}
	}
}

