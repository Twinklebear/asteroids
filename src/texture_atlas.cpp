#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <array>
#include <utility>
#include <initializer_list>
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <tinyxml2.h>
#include "gl_core_3_3.h"
#include "util.h"
#include "texture_atlas.h"

TextureAtlas::TextureAtlas(const std::string &file){
	load(file);
}
TextureAtlas::~TextureAtlas(){
	glDeleteTextures(1, &texture);
}
std::array<glm::vec2, 4> TextureAtlas::uvs(const std::string &name) const {
	auto f = images.find(name);
	if (f == images.end()){
		std::array<glm::vec2, 4> arr;
		arr.fill(glm::vec2{-1, -1});
		return arr;
	}
	return f->second;
}
bool TextureAtlas::has_image(const std::string &name) const {
	return images.find(name) != images.end();
}
TextureAtlas::const_iterator TextureAtlas::cbegin() const {
	return images.cbegin();
}
TextureAtlas::const_iterator TextureAtlas::cend() const {
	return images.cend();
}
void TextureAtlas::load(const std::string &file){
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
		texture = util::load_texture(img_file, &width, &height);
		load(n);
		//Warn the user if there are any TextureAtlas elements following this one
		//since they'll be ignored
		if (n->NextSiblingElement("TextureAtlas")){
			std::cerr << "TextureAtlas warning: Ignoring other TextureAtlas definitions"
				<< " in " << file << std::endl;
		}
	}
	else {
		std::cerr << "TextureAtlas error: " << file << " had no xml data\n";
		assert(false);
	}
}
void TextureAtlas::load(tinyxml2::XMLNode *node){
	using namespace tinyxml2;
	//Iterate over all the subtextures in the xml document and load their positions
	for (XMLNode *i = node->FirstChild(); i != nullptr; i = i->NextSibling()){
		//Support both the attribute format used by Kenny.NL and TexturePacker's
		//generic XML output
		XMLElement *e = i->ToElement();
		SDL_Rect r;
		std::string name;
		//The texture packer generic XML format
		if (e->Attribute("n") && e->Attribute("x") && e->Attribute("y")
			&& e->Attribute("w") && e->Attribute("h"))
		{
			name = e->Attribute("n");
			r = SDL_Rect{e->IntAttribute("x"), e->IntAttribute("y"),
				e->IntAttribute("w"), e->IntAttribute("h")};
		}
		//The format used by Kenny.NL
		else if (e->Attribute("name") && e->Attribute("x") && e->Attribute("y")
			&& e->Attribute("width") && e->Attribute("height"))
		{
			name = e->Attribute("name");
			r = SDL_Rect{e->IntAttribute("x"), e->IntAttribute("y"),
				e->IntAttribute("width"), e->IntAttribute("height")};
		}
		else {
			std::cerr << "TextureAtlas error: loading unsupported format" << std::endl;
			assert(false);
		}
		glm::vec2 dim{width, height};
		std::array<glm::vec2, 4> arr;
		arr[0] = glm::vec2{r.x, dim.y - r.y - r.h} / dim;
		arr[1] = glm::vec2{r.x + r.w, dim.y - r.y - r.h} / dim;
		arr[2] = glm::vec2{r.x, dim.y - r.y} / dim;
		arr[3] = glm::vec2{r.x + r.w, dim.y - r.y} / dim;
		images[name] = arr;
	}
}
std::string get_xml_error(const tinyxml2::XMLError &error){
	using namespace tinyxml2;
	switch (error){
		case XML_NO_ATTRIBUTE:
			return "XML_NO_ATTRIBUTE";
		case XML_WRONG_ATTRIBUTE_TYPE:
			return "XML_WRONG_ATTRIBUTE_TYPE";
		case XML_ERROR_FILE_NOT_FOUND:
			return "XML_ERROR_FILE_NOT_FOUND";
		case XML_ERROR_FILE_COULD_NOT_BE_OPENED:
			return "XML_ERROR_FILE_COULD_NOT_BE_OPENED";
		case XML_ERROR_FILE_READ_ERROR:
			return "XML_ERROR_FILE_READ_ERROR";
		case XML_ERROR_ELEMENT_MISMATCH:
			return "XML_ERROR_ELEMENT_MISMATCH";
		case XML_ERROR_PARSING_ELEMENT:
			return "XML_ERROR_PARSING_ELEMENT";
		case XML_ERROR_PARSING_ATTRIBUTE:
			return "XML_ERROR_PARSING_ATTRIBUTE";
		case XML_ERROR_IDENTIFYING_TAG:
			return "XML_ERROR_IDENTIFYING_TAG";
		case XML_ERROR_PARSING_TEXT:
			return "XML_ERROR_PARSING_TEXT";
		case XML_ERROR_PARSING_CDATA:
			return "XML_ERROR_PARSING_CDATA";
		case XML_ERROR_PARSING_COMMENT:
			return "XML_ERROR_PARSING_COMMENT";
		case XML_ERROR_PARSING_DECLARATION:
			return "XML_ERROR_PARSING_DECLARATION";
		case XML_ERROR_PARSING_UNKNOWN:
			return "XML_ERROR_PARSING_UNKNOWN";
		case XML_ERROR_EMPTY_DOCUMENT:
			return "XML_ERROR_EMPTY_DOCUMENT";
		case XML_ERROR_MISMATCHED_ELEMENT:
			return "XML_ERROR_MISMATCHED_ELEMENT";
		case XML_ERROR_PARSING:
			return "XML_ERROR_PARSING";
		case XML_CAN_NOT_CONVERT_TEXT:
			return "XML_CAN_NOT_CONVERT_TEXT";
		case XML_NO_TEXT_NODE:
			return "XML_NO_TEXT_NODE";
		default:
			return "XML_SUCCESS";
	}
}

