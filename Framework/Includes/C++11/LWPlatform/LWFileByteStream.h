#ifndef LWFILEBYTESTREAM_H
#define LWFILEBYTESTREAM_H
#include <LWCore/LWByteStream.h>
#include <LWPlatform/LWFileStream.h>

class LWFileByteStream : public LWByteStream {
public:
	/*!< \brief the file read callback for the byte stream. */
	static uint32_t ReadCallback(int8_t *Buffer, uint32_t Len, void *UserData);

	/*!< \brief move operator. */
	LWFileByteStream &operator = (LWFileByteStream &&O);

	/*!< \brief constructor for file byte stream.
		 \param Stream the opened stream to read from(note that LWFileByteStream does not own this file stream, and it needs to be cleaned up appropiatly when finished.).
		 \param CachedBufferLength the size of the cache to read from.
		 \param Flag flag indicating if networked or not.
		 \param Allocator the allocator for opening the stream buffer.
	*/
	LWFileByteStream(LWFileStream *Stream, uint32_t CachedBufferLength, uint32_t Flag, LWAllocator &Allocator);
	
	/*!< \brief move constructor. */
	LWFileByteStream(LWFileByteStream &&O);

private:
	LWFileStream *m_Stream;

};



#endif