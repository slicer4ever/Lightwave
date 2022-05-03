#ifndef FONT_H
#define FONT_H
#include "LWCore/LWTypes.h"
#include "LWVideo/LWImage.h"
#include <unordered_map>
#include <functional>

struct LWGlyph;

//Font clone without needing a video driver to load images.
class Font {
public:
	static const uint32_t MaxTextures = 16; /*!< \brief max number of "pages" the glyphs can inhabit. */

	/*!< \brief attempts to load an artery atlas file format, this format is the simpliest output to use for a multi-signed distance font generator (https://github.com/Chlumsky/msdf-atlas-gen) if a msdf font is used be sure to use the correct pixel shaders for rendering.
		 \note LWFont does not support multiple variants of fonts, as such the last variant that supports unicode is selected.
	*/
	static Font *LoadFontAR(LWFileStream *Stream, LWAllocator &Allocator);

	/*!< \brief attempts to load a bitmap fnt file, an angel font type(text format). the font size and characters are prebaked, so not many options are available.  only single page fonts are supported. */
	static Font *LoadFontFNT(LWFileStream *Stream, LWAllocator &Allocator);

	/*!< \brief attempts to load a TTF from the filestream, and allocates it with the allocator.
		 \param Stream the file of the font to be loaded.
		 \param Driver the video driver to create the texture with.
		 \param emSize the font size to create in em units.
		 \param FirstChar the first utf-32 character to load into this font table.
		 \param NbrChars the total number of characters from first to load.
		 \param Allocator the allocator that should be used to allocate memory for the font.
		 \return the font if it could be created, otherwise null if not loadable.
	*/
	static Font *LoadFontTTF(LWFileStream *Stream, uint32_t emSize, uint32_t FirstChar, uint32_t NbrChars, LWAllocator &Allocator);

	/*!< \brief attempts to load a TTF from the filestream, with multiple glyph ranges, and allocates it with the allocator.
		 \param Stream the file of the font to be loaded.
		 \param Driver the video driver to create the texture with.
		 \param emSize the font size to create in em units.
		 \param RangeCount the total number of ranges to expect
		 \param FirstChar an array of the first utf-32 character of the range to load into this font table.
		 \param NbrChars an array of the total number of characters of the range from the first to load.
		 \param Allocator the allocator that should be used to allocate memory for the font.
		 \return the font if it could be created, otherwise null if not loadable.
	*/
	static Font *LoadFontTTF(LWFileStream *Stream, uint32_t emSize, uint32_t RangeCount, const uint32_t *FirstChar, const uint32_t *NbrChars, LWAllocator &Allocator);

	static bool SaveFontTTF(LWFileStream *Stream, Font *Fnt, LWAllocator &Allocator);

	/*!< \brief sets the Image for the font. */
	Font &SetImage(uint32_t ImageIndex, LWImage &Img);

	/*!< \brief inserts the amount of horizontal kerning between the left and right characters. */
	Font &InsertKern(uint32_t Left, uint32_t Right, float Kerning);

	/*!< \brief inserts a glyph name into the glyph name map. */
	Font &InsertGlyphName(const LWUTF8Iterator &GlyphName, uint32_t GlyphID);

	/*!< \brief returns a glyph of the specified Unicode character.
		 \param Character the character the glyph wants.
		 \param Insert to insert the glyph if it doesn't exist.
	*/
	LWGlyph *GetGlyph(uint32_t Character, bool Insert = false);

	/*!< \brief returns a glyph character of the specified name.
		 \param GlyphName the name of the glyph to find.
		 \return 0 on failure, otherwise the character code for that glyph.
	*/
	uint32_t GetGlyphName(const LWUTF8Iterator &GlyphName) const;

	/*!< \brief returns a glyph of the precalculated hash name.
		 \param GlyphHash the hash of the glyph to find.
		 \return 0 on failure, otherwise the character code for the glyph.
	*/
	uint32_t GetGlyphName(uint32_t GlyphHash) const;

	/*!< \brief returns the kerning between the left and right characters. */
	float GetKernBetween(uint32_t Left, uint32_t Right) const;

	/*!< \brief returns the y pixel size of a single line. */
	float GetLineSize(void) const;

	/*!< \brief returns the texture atlas of the characters. */
	LWImage &GetImage(uint32_t Page);

	/*!< \brief constructs a font object for rendering. note that LWFont owns the texture object, as such it will freely destroy the texture object if the texture is changed, or when the font is destroyed.*/
	Font(float LineSize);

	/*!< \brief destructs the font object, and destroys the texture. */
	~Font();
private:
	LWImage m_ImageList[MaxTextures];
	std::unordered_map<uint32_t, uint32_t> m_GlyphNameMap;
	std::unordered_map<uint32_t, LWGlyph> m_GlyphTable;
	std::unordered_map<uint32_t, float> m_KernTable;
	float m_LineSize;
};

#endif