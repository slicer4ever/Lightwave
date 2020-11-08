#ifndef LWAUDIOSTREAM_H
#define LWAUDIOSTREAM_H
#include <LWCore/LWTypes.h>

/*!< \brief audio stream object which functions for decoding various formats for playback. */
class LWAudioStream {
public:
	enum {
		FormatRaw = 0x0, /*!< \brief type indicating that the underlying data is already decompressed as a raw PCM. */
		FormatWav = 0x1, /*!< \brief Type indicating that the underlying data is a wav format.  only wav formats which are decompressed are supported. */
		FormatVorbis = 0x2, /*!< \brief Type indicating that the underlying data is a Vorbis ogg format. */

		FormatFlag = 0x3, /*!< \brief Flag bitwise and against m_Flag to retrieve just the format information. */

		Decompressed = 0x80, /*!< \brief flag to indicate that the data is to be decompressed. */

		LinearPCM=0, /*!< \brief each sample is a linear pcm. */
		IEEEFloatPCM, /*!< \brief each sample is a floating point linear pcm. */
	};

	/*!< \brief determines format from the path extension, returns -1 if format is not determined. */
	static uint32_t GetFormatFromExtension(const LWUTF8Iterator &FilePath);

	/*!< \brief constructs an audio stream from the passed in file stream, determines format based on file extension. */
	static LWAudioStream *Create(const LWUTF8Iterator &Filepath, uint32_t Flag, LWAllocator &Allocator);

	/*!< \brief constructs an audio stream base on the specified path of the file stream. */
	static LWAudioStream *Create(LWFileStream &Stream, const LWUTF8Iterator &FilePath, uint32_t Flag, LWAllocator &Allocator);

	/*!< \brief constructs an audio stream based on the passed in buffer, the buffer is copied in it's entirety, the format type is deduced from the path's extenson. */
	static LWAudioStream *Create(const char *Buffer, uint32_t BufferLen, const LWUTF8Iterator &FilePath, uint32_t Flag, LWAllocator &Allocator);

	/*!< \brief construct's an audio stream based on the passed in buffer, note the audio stream takes possession of the passed in buffer. */
	static LWAudioStream *Create(char *Buffer, uint32_t BufferLen, const LWUTF8Iterator &FilePath, uint32_t Flag, LWAllocator &Allocator);

	/*!< \brief constructs an audio stream based on the specified format. */
	static LWAudioStream *Create(LWFileStream &Stream, uint32_t Flag, uint32_t FormatType, LWAllocator &Allocator);

	/*!< \brief constructs an audio stream based on the passed in buffer, the buffer is copied in it's entirety. */
	static LWAudioStream *Create(const char *Buffer, uint32_t BufferLen, uint32_t Flag, uint32_t FormatType, LWAllocator &Allocator);

	/*!< \brief constructs an audio stream based on the passed in buffer, note that audio stream takes possession of the passed in buffer. */
	static LWAudioStream *Create(char *Buffer, uint32_t BufferLen, uint32_t Flag, uint32_t FormatType, LWAllocator &Allocator);

	/*!< \brief decodes the requested samples into the specified buffer, then returns a pointer to that buffer, however if the underlying data is already decompressed, the returned pointer is to the samples in that stream. 
		 \param Buffer the buffer to receive the samples, should be at least SampleLen*SampleSize*Channels in size.
		 \param SamplePos the position in the audio stream samples that are requested.
		 \param SampleLen the number of samples requested.
		 \param ForceCopy even if the underlying stream is decompressed, passing true here will still copy into the buffer.
		 \return a pointer to Buffer, or to the internal buffer if the internal buffer is already decompressed.
	*/
	char *DecodeSamples(char *Buffer, uint32_t SamplePos, uint32_t SampleLen, bool ForceCopy = false);

	/*!< \brief returns the intended slice size for the stream in samples. */
	uint32_t GetSampleSliceSize(uint32_t BufferSize) const;

	/*!< \brief returns the amount of time(relative to LWTimer::Resolution()) a single sample takes to play. */
	uint64_t GetTimePerSample(void) const;

	/*!< \brief returns the total time(relative to LWTimer::Resolution()) for all samples to play. */
	uint64_t GetTotalTime(void) const;

	/*!< \brief returns the total samples the audio stream has. */
	uint32_t GetSampleLength(void) const;

	/*!< \brief returns the rate at which the audio stream should be played at optimally. */
	uint32_t GetSampleRate(void) const;

	/*!< \brief returns the size in bytes of each sample, all samples are expected to be interleaved with channels, so one frame is sampleSize*Channels in size.*/
	uint32_t GetSampleSize(void) const;

	/*!< \brief returns the type associated for the sample. */
	uint32_t GetSampleType(void) const;

	/*!< \brief returns the format of the underlying stream. */
	uint32_t GetFormatType(void) const;

	/*!< \brief returns the number of channels for the audio stream. */
	uint32_t GetChannels(void) const;

	/*!< \brief returns the flags associated with the audio stream. */
	uint32_t GetFlag(void) const;

	/*!< \brief returns the length of the stream in seconds. */
	float GetLength(void) const;

	/*!< \brief move operator. */
	LWAudioStream &operator = (LWAudioStream &&Stream);

	/*!< \brief move constructor. */
	LWAudioStream(LWAudioStream &&Stream);

	/*!< \brief constructs in place the audio stream object. 
		 \note Buffer and Context are to be owned by this object when passed in.
	*/
	LWAudioStream(void *Context, char *Buffer, uint32_t SampleLength, uint32_t SampleSize, uint32_t SampleRate, uint32_t SampleType, uint32_t Channels, uint32_t Flag);

	/*!< \brief constructs a blank audio stream object. */
	LWAudioStream();

	/*!< \brief destroys the audio stream. */
	~LWAudioStream();
private:
	void *m_Context;
	char *m_Buffer;
	uint32_t m_SampleLength;
	uint32_t m_SampleRate;
	uint32_t m_SampleType;
	uint32_t m_SampleSize;
	uint32_t m_Channels;
	uint32_t m_Flag;
};

#endif
