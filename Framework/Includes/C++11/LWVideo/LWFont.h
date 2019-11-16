#ifndef LWFONT_H
#define LWFONT_H
#include "LWVideo/LWTypes.h"
#include "LWCore/LWTypes.h"
#include "LWVideo/LWTexture.h"
#include "LWVideo/LWMesh.h"
#include <unordered_map>
#include <functional>

struct LWGlyph {
	//float m_Kerning[MaxGlyphs]; /*!< \brief horizontal kerning between this glyph, and the glyph which would appear before this one. */
	LWVector4f m_TexCoord; /*!< \brief texture coordinates for the glyph. */
	LWVector2f m_Size; /*!< \brief pixel size of the glyph. */
	LWVector2f m_Advance; /*!< \brief horizontal, and vertical advance(vertical is not used in default rendering methods) */
	LWVector2f m_Bearing; /*!< \brief bearing offsets. */
	uint32_t m_Character; /*!< \brief utf-32 character code. */
	uint32_t m_TextureIndex; /*!< \brief texture index the glyph is on. */
};

/*!< \brief callback function which is used for drawing out the text. return true if successful. 
	 \param Tex the texture the glyph is on.
	 \param Position the position of the rectangle for the glyph to be drawn.
	 \param Size the size of the rectangle for the glpyh to be drawn.
	 \param TexCoord the texture position, x,y=top left, z,w = bottom right.
	 \param Color the color of the glyph.
	 \return true if written, false if failure(if false is returned then the font writer will stop.)
*/
typedef std::function<bool(LWTexture *Tex, const LWVector2f &Position, const LWVector2f &Size, const LWVector4f &TexCoord, const LWVector4f &Color)> LWFontWriteCallback;

/*!\ \brief a very simple font writer that demonstrates how to draw text into a mesh. */
struct LWFontSimpleWriter {
	static const uint32_t MaxTextures = 32;
	LWMesh<LWVertexUI> *m_Mesh; //Mesh to draw into.
	LWTexture *m_Textures[MaxTextures]; //Texture for the font.
	uint32_t m_TextureVertices[MaxTextures]; //Vertices for the text stream.
	uint32_t m_TextureCount = 0; //Total number of textures drawn.

	/*!< \brief changes the texture if needed. */
	bool WriteTexture(LWTexture *Tex);

	/*!< \brief function passed to the font object for writing. */
	bool WriteGlyph(LWTexture *Tex, const LWVector2f &Position, const LWVector2f &Size, const LWVector4f &TexCoord, const LWVector4f &Color);

	/*!< \brief constructor for the writer. */
	LWFontSimpleWriter(LWMesh<LWVertexUI> *Mesh);

	/*!< \brief default construct for the writer. */
	LWFontSimpleWriter() = default;
};


class LWFont {
public:
	static const uint32_t MaxTextures = 16; /*!< \brief max number of "pages" the glyphs can inhabit. */
	/*!< \brief attempts to load a bitmap fnt file, an angel font type(text format). the font size and characters are prebaked, so not many options are available.  only single page fonts are supported. */
	static LWFont *LoadFontFNT(LWFileStream *Stream, LWVideoDriver *Driver, LWAllocator &Allocator);

	/*!< \brief attempts to load a TTF from the filestream, and allocates it with the allocator.
		 \param Stream the file of the font to be loaded.
		 \param Driver the video driver to create the texture with.
		 \param emSize the font size to create in em units.
		 \param FirstChar the first utf-32 character to load into this font table.
		 \param NbrChars the total number of characters from first to load.
		 \param Allocator the allocator that should be used to allocate memory for the font.
		 \return the font if it could be created, otherwise null if not loadable.
	*/
	static LWFont *LoadFontTTF(LWFileStream *Stream, LWVideoDriver *Driver, uint32_t emSize, uint32_t FirstChar, uint32_t NbrChars, LWAllocator &Allocator);

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
	static LWFont *LoadFontTTF(LWFileStream *Stream, LWVideoDriver *Driver, uint32_t emSize, uint32_t RangeCount, const uint32_t *FirstChar, const uint32_t *NbrChars, LWAllocator &Allocator);

	/*!< \brief returns the default shader for rendering font, embedded into the code. */
	static const char *GetFontShaderSource(void);

	/*!< \brief returns the shader for use with rendering signed distance field font's, embedded into the code. */
	static const char *GetSDFFontShaderSource(void);

	/*!< \brief sets the texture for the font. */
	LWFont &SetTexture(uint32_t TextureIndex, LWTexture *Tex);

	/*!< \brief measures a text object, and returns the bounding rectangle which encompasses the entire text object. */
	LWVector4f MeasureText(const LWText &Text, float Scale);

	/*!< \brief measures a formatted text object, and returns the bounding rectangle which encompasses the entire text object. */
	LWVector4f MeasureTextf(const LWText &Text, float Scale, ...);

	/*!< \brief measures a text object upto n characters, and returns the bounding rectangle which encompasses the entire text object. */
	LWVector4f MeasureText(const LWText &Text, uint32_t CharCount, float Scale);

	/*!< \brief measures a formatted text object upto n characters, and returns the bounding rectangle which encompasses the entire text object. */
	LWVector4f MeasureTextf(const LWText &Text, uint32_t CharCount, float Scale, ...);

	/*!< \brief measures text upto a certain width, and returns the index of the character at that position. */
	uint32_t CharacterAt(const LWText &Text, float Width, float Scale);

	/*!< \brief measures text upto a certain width, and returns the index of the character at that position. */
	uint32_t CharacterAtf(const LWText &Text, float Width, float Scale, ...);

	/*!< \brief measures text upto a certain width, and returns the index of the character at that position. */
	uint32_t CharacterAt(const LWText &Text, float Width, uint32_t CharCount, float Scale);

	/*!< \brief measures text upto a certain width, and returns the index of the character at that position. */
	uint32_t CharacterAtf(const LWText &Text, float Width, uint32_t CharCount, float Scale, ...);

	/*!< \brief convenience function that automatically binds a class's method as the writer function. */
	template<class Obj, class CallBack>
	LWVector4f DrawTextm(const LWText &Text, const LWVector2f &Position, float Scale, const LWVector4f &Color, Obj *O, CallBack C) {
		return DrawText(Text, Position, Scale, Color, std::bind(C, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
	}

	/*!< \brief convenience function that automatically binds a class's method as the writer function. */
	template<class Obj, class CallBack>
	LWVector4f DrawTextmf(const LWText &Fmt, const LWVector2f &Position, float Scale, const LWVector4f &Color, Obj *O, CallBack C, ...) {
		char Buffer[1024];
		va_list lst;
		va_start(lst, C);
		vsnprintf(Buffer, sizeof(Buffer), (const char*)Fmt.GetCharacters(), lst);
		va_end(lst);
		return DrawText(Buffer, Position, Scale, Color, std::bind(C, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
	}

	/*!< \brief convenience function that automatically binds a class's method as the writer function. */
	template<class Obj, class CallBack>
	LWVector4f DrawTextm(const LWText &Text, uint32_t CharCount, const LWVector2f &Position, float Scale, const LWVector4f &Color, Obj *O, CallBack C){
		return DrawText(Text, CharCount, Position, Scale, Color, std::bind(C, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
	}

	/*!< \brief convenience function that automatically binds a class's method as the writer function. */
	template<class Obj, class CallBack>
	LWVector4f DrawTextmf(const LWText &Fmt, uint32_t CharCount, const LWVector2f &Position, float Scale, const LWVector4f &Color, Obj *O, CallBack C, ...) {
		char Buffer[1024];
		va_list lst;
		va_start(lst, C);
		vsnprintf(Buffer, sizeof(Buffer), (const char*)Fmt.GetCharacters(), lst);
		va_end(lst);
		return DrawText(Buffer, CharCount, Position, Scale, Color, std::bind(C, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
	}

	/*!< \brief convenience function that automatically binds a class's method as the writer function. */
	template<class Obj, class CallBack>
	LWVector4f DrawClippedTextm(const LWText &Text, const LWVector2f &Position, float Scale, const LWVector4f &Color, const LWVector4f &AABB, Obj *O, CallBack C) {
		return DrawClippedText(Text, Position, Scale, Color, AABB, std::bind(C, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
	}

	/*!< \brief convenience function that automatically binds a class's method as the writer function. */
	template<class Obj, class CallBack>
	LWVector4f DrawClippedTextmf(const LWText &Fmt, const LWVector2f &Position, float Scale, const LWVector4f &Color, const LWVector4f &AABB, Obj *O, CallBack C, ...) {
		char Buffer[1024];
		va_list lst;
		va_start(lst, C);
		vsnprintf(Buffer, sizeof(Buffer), (const char*)Fmt.GetCharacters(), lst);
		va_end(lst);
		return DrawClippedText(Buffer, Position, Scale, Color, AABB, std::bind(C, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
	}

	/*!< \brief convenience function that automatically binds a class's method as the writer function. */
	template<class Obj, class CallBack>
	LWVector4f DrawClippedTextm(const LWText &Text, uint32_t CharCount, const LWVector2f &Position, float Scale, const LWVector4f &Color, const LWVector4f &AABB, Obj *O, CallBack C) {
		return DrawClippedText(Text, CharCount, Position, Scale, Color, AABB, std::bind(C, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
	}

	/*!< \brief convenience function that automatically binds a class's method as the writer function. */
	template<class Obj, class CallBack>
	LWVector4f DrawClippedTextmf(const LWText &Fmt, uint32_t CharCount, const LWVector2f &Position, float Scale, const LWVector4f &Color, const LWVector4f &AABB, Obj *O, CallBack C, ...) {
		char Buffer[1024];
		va_list lst;
		va_start(lst, C);
		vsnprintf(Buffer, sizeof(Buffer), (const char*)Fmt.GetCharacters(), lst);
		va_end(lst);
		return DrawClippedText(Buffer, CharCount, Position, Scale, Color, AABB, std::bind(C, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
	}

	/*!< \brief renders text to the writer function. 
		 \param Text the text to render out.
		 \param Position the location on the screen to draw to(this is translated by the ortho matrix.)
		 \param Scale the amount to scale the font up/down by(1.0 is default font scale based on the emSize when the font was created.)
		 \param Mesh the mesh to render into.
		 \param Color the color to mark each text character as.
		 \return the rectangular bounding box which encompasses this text object.
	*/
	LWVector4f DrawText(const LWText &Text, const LWVector2f &Position, float Scale, const LWVector4f &Color, LWFontWriteCallback Writer);

	/*!< \brief constructs a mesh object for rendering the formatted text with the font.
	 	 \param Text the text to render out.
		 \param Position the location on the screen to draw to(this is translated by the ortho matrix.)
		 \param Scale the amount to scale the font up/down by(1.0 is default font scale based on the emSize when the font was created.)
		 \param Mesh the mesh to render into.
		 \param Color the color to mark each text character as.
		 \return the rectangular bounding box which encompasses this text object.
	*/
	LWVector4f DrawTextf(const LWText &Text, const LWVector2f &Position, float Scale, const LWVector4f &Color, LWFontWriteCallback Writer, ...);

	/*!< \brief constructs a mesh object for rendering the text upto n characters with the font.
		 \param Text the text to render out.
		 \param CharCount the number of characters to render upto.
		 \param Position the location on the screen to draw to(this is translated by the ortho matrix.)
		 \param Scale the amount to scale the font up/down by(1.0 is default font scale based on the emSize when the font was created.)
		 \param Mesh the mesh to render into.
		 \param Color the color to mark each text character as.
		 \return the rectangular bounding box which encompasses this text object.
	*/
	LWVector4f DrawText(const LWText &Text, uint32_t CharCount, const LWVector2f &Position, float Scale, const LWVector4f &Color, LWFontWriteCallback Writer);

	/*!< \brief constructs a mesh object for rendering the formatted text upto n characters with the font.
		 \param Text the text to render out.
		 \param CharCount the number of characters to render upto.
		 \param Position the location on the screen to draw to(this is translated by the ortho matrix.)
		 \param Scale the amount to scale the font up/down by(1.0 is default font scale based on the emSize when the font was created.)
		 \param Mesh the mesh to render into.
		 \param Color the color to mark each text character as.
		 \return the rectangular bounding box which encompasses this text object.
	*/
	LWVector4f DrawTextf(const LWText &Text, uint32_t CharCount, const LWVector2f &Position, float Scale, const LWVector4f &Color, LWFontWriteCallback Writer, ...);


	/*!< \brief constructs a mesh object for rendering the text with the font.
		 \param Text the text to render out.
		 \param Position the location on the screen to draw to(this is translated by the ortho matrix.)
		 \param Scale the amount to scale the font up/down by(1.0 is default font scale based on the emSize when the font was created.)
		 \param Mesh the mesh to render into.
		 \param Color the color to mark each text character as.
		 \param AABB the axis aligned bounding box to clip the text around. (left, bottom, width, height)
		 \return the rectangular bounding box which encompasses this text object.
	*/
	LWVector4f DrawClippedText(const LWText &Text, const LWVector2f &Position, float Scale, const LWVector4f &Color, const LWVector4f &AABB, LWFontWriteCallback Writer);

	/*!< \brief constructs a mesh object for rendering the formatted text with the font.
		 \param Text the text to render out.
		 \param Position the location on the screen to draw to(this is translated by the ortho matrix.)
	 	 \param Scale the amount to scale the font up/down by(1.0 is default font scale based on the emSize when the font was created.)
		 \param Mesh the mesh to render into.
		 \param Color the color to mark each text character as.
		 \param AABB the axis aligned bounding box to clip the text around. (left, bottom, width, height)
		 \return the rectangular bounding box which encompasses this text object.
	*/
	LWVector4f DrawClippedTextf(const LWText &Text, const LWVector2f &Position, float Scale, const LWVector4f &Color, const LWVector4f &AABB, LWFontWriteCallback Writer, ...);

	/*!< \brief constructs a mesh object for rendering the text upto n characters with the font.
		 \param Text the text to render out.
		 \param CharCount the number of characters to render upto.
	 	 \param Position the location on the screen to draw to(this is translated by the ortho matrix.)
		 \param Scale the amount to scale the font up/down by(1.0 is default font scale based on the emSize when the font was created.)
		 \param Mesh the mesh to render into.
		 \param Color the color to mark each text character as.
		 \param AABB the axis aligned bounding box to clip the text around. (left, bottom, width, height)
		 \return the rectangular bounding box which encompasses this text object.
	*/
	LWVector4f DrawClippedText(const LWText &Text, uint32_t CharCount, const LWVector2f &Position, float Scale, const LWVector4f &Color, const LWVector4f &AABB, LWFontWriteCallback Writer);

	/*!< \brief constructs a mesh object for rendering the formatted text upto n characters with the font.
		 \param Text the text to render out.
		 \param CharCount the number of characters to render upto.
		 \param Position the location on the screen to draw to(this is translated by the ortho matrix.)
		 \param Scale the amount to scale the font up/down by(1.0 is default font scale based on the emSize when the font was created.)
		 \param Mesh the mesh to render into.
		 \param Color the color to mark each text character as.
		 \param AABB the axis aligned bounding box to clip the text around. (left, bottom, width, height)
		 \return the rectangular bounding box which encompasses this text object.
	*/
	LWVector4f DrawClippedTextf(const LWText &Text, uint32_t CharCount, const LWVector2f &Position, float Scale, const LWVector4f &Color, const LWVector4f &AABB, LWFontWriteCallback Writer, ...);

	/*!< \brief inserts the amount of horizontal kerning between the left and right characters. */
	LWFont &InsertKern(uint32_t Left, uint32_t Right, float Kerning);

	/*!< \brief inserts a glyph name into the glyph name map. */
	LWFont &InsertGlyphName(const LWText &GlyphName, uint32_t GlyphID);

	/*!< \brief sets the glyph character code used when encountering a character that isn't in the glyph map. */
	LWFont &SetErrorGlyph(uint32_t Character);

	/*!< \brief returns a glyph of the specified utf-32 character. 
		 \param Character the character the glyph wants.
		 \param Insert to insert the glyph if it doesn't exist.
	*/
	LWGlyph *GetGlyph(uint32_t Character, bool Insert = false);

	/*!< \brief returns a glyph character of the specified name.
		 \param GlyphName the name of the glyph to find.
		 \return 0 on failure, otherwise the character code for that glyph.
	*/
	uint32_t GetGlyphName(const LWText &GlyphName) const;

	/*!< \brief returns a glyph of the precalculated hash name.
		 \param GlyphHash the hash of the glyph to find.
		 \return 0 on failure, otherwise the character code for the glyph.
	*/
	uint32_t GetGlyphName(uint32_t GlyphHash) const;

	/*!< \brief returns the kerning between the left and right characters. */
	float GetKernBetween(uint32_t Left, uint32_t Right) const;

	/*!< \brief returns the y pixel size of a single line. */
	float GetLineSize(void) const;

	/*!< \brief returns the error glyph used when a character is encountered that isn't in the glyph map.*/
	LWGlyph *GetErrorGlyph(void);

	/*!< \brief returns the texture atlas of the characters. */
	LWTexture *GetTexture(uint32_t Page);

	/*!< \brief constructs a font object for rendering. note that LWFont owns the texture object, as such it will freely destroy the texture object if the texture is changed, or when the font is destroyed.*/
	LWFont(LWVideoDriver *Driver, float LineSize);

	/*!< \brief destructs the font object, and destroys the texture. */
	~LWFont();
private:
	LWVideoDriver *m_Driver;
	std::unordered_map<uint32_t, uint32_t> m_GlyphNameMap;
	std::unordered_map<uint32_t, LWGlyph> m_GlyphTable;
	std::unordered_map<uint32_t, float> m_KernTable;
	LWGlyph *m_ErrorGlyph;
	LWTexture *m_TextureList[MaxTextures];
	float m_LineSize;
};

#endif