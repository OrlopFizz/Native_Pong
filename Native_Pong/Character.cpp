#include <glm.hpp>
struct Character {
	unsigned int textureID {}; //ID of the glyph(character) texture
	glm::ivec2 size {}; //Size of the Glyph
	glm::ivec2 bearing {}; //Offset from baseline to left/top of Glyph
	unsigned int advance {}; //Offset to advance to next Glyph

	Character(unsigned int pid, glm::ivec2 psize, glm::ivec2 pbearing, unsigned int padvance) : textureID{ pid }, size{ psize }, bearing{ pbearing }, advance{padvance} {}
	Character() {};
};