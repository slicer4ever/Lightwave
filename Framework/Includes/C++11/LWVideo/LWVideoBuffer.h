#ifndef LWVIDEOBUFFER_H
#define LWVIDEOBUFFER_H
#include "LWVideo/LWTypes.h"
#include "LWCore/LWTypes.h"

/*!< \brief Expected structure of any DrawIndirect rendering buffer's that use indice's. */
struct LWIndirectIndice {
	uint32_t m_IndexCount; /*!< \brief number of indice's per instance. */
	uint32_t m_InstanceCount; /*!< \brief number of instance's. */
	uint32_t m_IndexOffset; /*!< \brief offset into indice buffer to start at. */
	int32_t m_IndexVertexOffset; /*!< \brief vertex offset added to each indice. */
	uint32_t m_InstanceOffset; /*!< \brief instance offset to start at for the instance id. */
};

/*!< \brief Expected structure of any DrawIndirect rendering buffer's that do not use indice's. */
struct LWIndirectVertex {
	uint32_t m_VertexCount; /*!< \brief number of vertice's per instance. */
	uint32_t m_InstanceCount; /*!< \brief number of instance's. */
	uint32_t m_VertexOffset; /*!< \brief offset into vertex buffer to start at. */
	uint32_t m_InstanceOffset; /*!< \brief instance offset to start at for the instance id. */
};

/*!< \brief video buffer container class, allows for modifying and uploading a local copy for dynamic data, or to serve as an object for use with static data.  Because Lightwave prefers a deferred context and wants to be compatible with multiple api's, it is not possible to do partial updates on a video buffer. */
class LWVideoBuffer : public LWVideoResource {
public:
	enum{
		Vertex=0x0, /*!< \brief the buffer type is to be used as vertex input data. */
		Uniform=0x1, /*!< \brief the buffer type is to be used as uniform buffer data. */
		Index16=0x2, /*!< \brief the buffer type is to be used as an 16-bit index for vertex data. format is unsigned 16 bit(maximum of 2^16-1 indices). */
		Index32=0x3, /*!< \brief the buffer type is to be used as an 32-bit index for vertex data. format is unsigned 32 bit(maximum of 2^32-1 indices). */
		ImageBuffer=0x4, /*!< \brief the buffer type is to be used as a shader buffer data. */
		Indirect = 0x5, /*!< \brief buffer to be used when calling indirect render functions. */
		TypeFlag=0x7, /*!< \brief flag used and the flag variable to identify the type of buffer contained. */
		
		PersistentMapped = 0x0, /*!< \brief usage type to indicate the buffer should be persistently mapped for gpu data writing if possible(for highest backwards-compatablility, be sure to set usage type as write discarable.  not usable with a static buffer).  (This flag is work-in progress so may not be implemented on every api yet. if this flag is set, it superscedes the local buffer's flag buffer, and the local buffer is the pointer to the mapped data, some implementations might be forced to fake the local buffer as the persistent mapped buffer.) */
		Static = 0x8, /*!< \brief usage type to indicate the buffer is not to be modified after being created. */
		WriteDiscardable = 0x10, /*!< \brief usage type to indicate upon each write, it is safe to discard the previous data that existed(using this flag optimizes for per-frame update frequencys). */
		WriteNoOverlap = 0x18, /*!< \brief usage type to indicate upon each write, the app will guarantee it is not writing to data that is already being worked on. */
		Readable = 0x20, /*!< \brief usage type to indicate that the buffer intends to be read back to the cpu. */
		GPUResource = 0x28, /*!< \brief usage type to indicate that the buffer will be read and writable on the gpu, if this flag is set then the resource can not be written to after the initialization of data. */

		UsageFlag = 0x38, /*!< \brief bitwise operator to get the usage flag. */
		UsageBitOffset = 0x3, /*!< \brief bits to rshift the usage flag to get the value to 0 range. */

		LocalCopy=0x10000000, /*!< \brief flag which indicates a local buffer is attached the video buffer, which can be used to temporarily modify the underlying buffer. */
		Dirty = 0x20000000, /*!< \brief flag to indicate the local buffer has been modified. */
	};

	/*!< \brief sets the edited length of the local copy and set's the dirty flag for updating the video buffer.
		 \param EditLength the length of the local buffer that has been changed.
		 \param DontUpdate does not set the buffer as dirty if this bool is set to true, defaulted to false.
	*/
	LWVideoBuffer &SetEditLength(uint32_t EditLength, bool DontUpdate=false);

	/*!< \brief copys an array of Type of data upto Length into the local buffer(if it's been asked for), then set's the edit length to that length and flags the buffer for writing. */
	template<class Type>
	LWVideoBuffer &CopyType(Type *Data, uint32_t Length, bool DontUpdate = false) {
		if (!m_LocalBuffer) return *this;
		std::copy(Data, Data + Length, (Type*)m_LocalBuffer);
		return SetEditLength(Length * sizeof(Type), DontUpdate);
	}

	/*!< \brief returns true if the dirty flag is set. */
	bool isDirty(void) const;

	/*!< \brief clears the dirty flag. */
	LWVideoBuffer &ClearDirty(void);

	/*!< \brief makes the local buffer if it does not exist, and returns the buffer. (note: an allocator must have been specified during construction otherwise, no local buffer will have been made)*/
	uint8_t *MakeLocalBuffer(void);

	/*!< \brief frees the local buffer if it's unnecessary anymore. */
	LWVideoBuffer &DestroyLocalBuffer(void);

	/*!< \brief the mover operator for the video buffer. */
	LWVideoBuffer &operator = (LWVideoBuffer &&Buffer);

	/*!< \brief returns the allocator used for allocating the local buffer. */
	LWAllocator *GetAllocator(void) const;

	/*!< \brief returns the local buffer for the video buffer. */
	uint8_t *GetLocalBuffer(void) const;

	/*!< \brief returns the flag for the video buffer. */
	uint32_t GetFlag(void) const;

	/*!< \brief returns the length of the video buffer type count. */
	uint32_t GetLength(void) const;

	/*!< \brief returns the length of video buffer's underlying type size. */
	uint32_t GetTypeSize(void) const;

	/*!< \brief returns the raw byte length of the video buffer. */
	uint32_t GetRawLength(void) const;

	/*!< \brief returns the length of the edited local buffer. */
	uint32_t GetEditLength(void) const;

	/*!< \brief returns the type of video buffer. */
	uint32_t GetType(void) const;

	/*!< \brief copy constructor */
	LWVideoBuffer(LWVideoBuffer &&Buffer);
	
	/*!< \brief default constructor. */
	LWVideoBuffer();

	/*!< \brief constructs the entire video buffer. 
		 \param Buffer buffer to copy into the local buffer, if flag is set with the LocalCopy flag.
		 \param VideoContext the video context associated with the video buffer.
		 \param Allocator the allocator used to generate the local buffer. if an allocator is not passed, no local buffer can be created.
		 \param TypeSize the size of each of the underlying types.
		 \param Length the number of bytes the video buffer contains.
		 \param Flag the flag associated with the usage type, and if a local buffer should be created.
	*/
	LWVideoBuffer(const uint8_t *Buffer, LWAllocator *Allocator, uint32_t TypeSize, uint32_t Length, uint32_t Flag);

	/*!< \brief destructs the video buffer. */
	~LWVideoBuffer();
private:
	LWAllocator *m_Allocator = nullptr;
	uint8_t *m_LocalBuffer = nullptr;
	uint32_t m_Flag = 0;
	uint32_t m_Length = 0;
	uint32_t m_EditLength = 0;
	uint32_t m_TypeSize = 0;
};

template<class Type>
class LWVideoBufferCon : public LWVideoBuffer {
public:

	Type GetContext(void) const{
		return m_Context;
	}

	Type &GetContext(void) {
		return m_Context;
	}

	LWVideoBufferCon(const uint8_t *Buffer, LWAllocator *Allocator, uint32_t TypeSize, uint32_t Length, uint32_t Flag, Type Context) : LWVideoBuffer(Buffer, Allocator, TypeSize, Length, Flag), m_Context(Context) {}
private:
	Type m_Context;
};

#endif