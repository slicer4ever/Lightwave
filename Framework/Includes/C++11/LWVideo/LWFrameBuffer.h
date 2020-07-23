#ifndef LWFRAMEBUFFER_H
#define LWFRAMEBUFFER_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWVector.h"
#include "LWVideo/LWTypes.h"

/*!< \brief LWFrameBufferAttachment is the bound attachments for the specified slot for the framebuffer, and includes face/layer/mipmap attachment information for that slot. */
struct LWFrameBufferAttachment {
	LWTexture *m_Source = nullptr; /*!< \brief the texture source for this attachment(should be a Texture2D/Texture2DArray/TextureCubemap). */
	uint32_t m_Layer = 0; /*!< \brief the texture layer for 2DArray/1DArrays. */
	uint32_t m_Mipmap = 0; /*!< \brief the texture mipmap layer(0 for base image). */
	uint32_t m_Face = 0; /*!< \brief the cube map face for the attachment. */

	/*!< \brief default construct for slot. */
	LWFrameBufferAttachment() = default;
};

/*!< \brief LWFrameBuffer is the framebuffer used for all rendering operations. */
class LWFrameBuffer{
public:
	enum{
		Color0=0, /*!< \brief first color attachment point. */
		Color1, /*!< \brief second color attachment point. */
		Color2, /*!< \brief third color attachment point. */
		Color3, /*!< \brief fourth color attachment point. */
		Color4, /*!< \brief fifth color attachment point. */
		Color5, /*!< \brief sixth color attachment point. */
		Depth, /*!< \brief depth(and stencil) attachment point. */
		Count, /*!< \brief total number of attachments available. */
		ColorCount = 6, /*!< \brief total number of color attachments available. */
		Dirty = 0x80000000 /*!< \brief flag to indicate framebuffer has changed states. */
	};
	/*!< \brief sets a texture to the specified attachment point.  note the texture must be the same size as the framebuffer object. 
		 \param Layer for arrayed textures the layer to be bound.
		 \param Mipmap the mipmap slice to use when bound.
	*/
	LWFrameBuffer &SetAttachment(uint32_t AttachmentID, LWTexture *Texture, uint32_t Layer = 0, uint32_t Mipmap = 0);

	/*!< \brief set's the face of a cubemap texture to the specified attachment point.  note that each cube face must be the same size as the framebuffer object.
		 \param Layer for array cubemap texture, the layer to be bound.
		 \param Mipmap the mipmap slice to use when bound.
	*/
	LWFrameBuffer &SetCubeAttachment(uint32_t AttachmentID, LWTexture *Texture, uint32_t Face, uint32_t Layer = 0, uint32_t Mipmap = 0);

	/*!< \brief removes all attachment's from framebuffer. */
	LWFrameBuffer &ClearAttachments(void);

	/*!< \brief returns if the dirty flag is set. */
	bool isDirty(void) const;

	/*!< \brief clears the dirty flag. */
	LWFrameBuffer &ClearDirty(void);

	/*!< \brief returns the size of the frame buffer, all attachments must be this size to function correctly. */
	LWVector2i GetSize(void) const;

	/*!< \brief returns the size of the frame buffer casted to floats. */
	LWVector2f GetSizef(void) const;

	/*!< \brief returns the attachment at the attachmentID. */
	LWFrameBufferAttachment &GetAttachment(uint32_t AttachmentID);

	/*!< \brief returns the current flag for the frame buffer. */
	uint32_t GetFlag(void) const;

	/*!< \brief constructs a framebuffer with the specified size, and the video context associated with the frame buffer. */
	LWFrameBuffer(const LWVector2i &Size);
private:
	uint32_t m_Flag;
	LWFrameBufferAttachment m_Attachments[Count];
	LWVector2i m_Size;
};


/*!< \cond */
template<class Type>
class LWFrameBufferCon : public LWFrameBuffer {
public:

	LWFrameBufferCon<Type> &SetContext(Type &Context) {
		m_Context = Context;
		return *this;
	}

	Type GetContext(void) const {
		return m_Context;
	}

	Type &GetContext(void) {
		return m_Context;
	}

	LWFrameBufferCon(Type Context, const LWVector2i &Size) : LWFrameBuffer(Size), m_Context(Context) {}
private:
	Type m_Context;
};


/*!< \endcond */

#endif