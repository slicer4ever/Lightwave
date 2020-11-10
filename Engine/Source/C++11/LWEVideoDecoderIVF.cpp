#include "LWEVideoDecoderIVF.h"
#include <vpx/vp8.h>
#include <vpx/vp8dx.h>
#include <vpx/vpx_codec.h>
#include <vpx/vpx_decoder.h>
#include <LWCore/LWAllocator.h>
#include <LWCore/LWByteBuffer.h>
#include <LWCore/LWTimer.h>
#include "LWEVideoPlayer.h"

LWEVideoIVFHeader::LWEVideoIVFHeader(const int8_t *Buffer) {
	uint32_t o = 0;
	o += LWByteBuffer::Read<char>(m_Magic, 4, Buffer+o);
	o += LWByteBuffer::Read<int16_t>(&m_Version, Buffer + o);
	o += LWByteBuffer::Read<int16_t>(&m_HeaderSize, Buffer + o);
	o += LWByteBuffer::Read<int32_t>(&m_fourcc, Buffer + o);
	o += LWByteBuffer::Read<int16_t>(&m_Width, Buffer + o);
	o += LWByteBuffer::Read<int16_t>(&m_Height, Buffer + o);
	o += LWByteBuffer::Read<int32_t>(&m_TimeNumer, Buffer + o);
	o += LWByteBuffer::Read<int32_t>(&m_TimeDenom, Buffer + o);
	o += LWByteBuffer::Read<int32_t>(&m_FrameCount, Buffer + o);
};

LWEVideoIVFFrameHeader::LWEVideoIVFFrameHeader(const int8_t *Buffer) {
	uint32_t o = 0;
	o += LWByteBuffer::Read<uint32_t>(&m_FrameSize, Buffer + o);
	o += LWByteBuffer::Read<uint64_t>(&m_Timestamp, Buffer + o);
}

bool LWEVideoDecoderIVF::CreateDecoder(LWEVideoPlayer &Player, LWFileStream &Stream, LWVideoDriver *Driver, uint32_t PlayerFlags, float PlaybackSpeed, uint32_t LoopCnt, void *Userdata, std::function<void(LWEVideoPlayer &, void * )> FinishedCallback, LWAllocator &Allocator){
	int8_t HeaderBuf[LWE_IVF_FILE_HDR_SZ];
	if ((Stream.Read((char*)HeaderBuf, LWE_IVF_FILE_HDR_SZ)) != LWE_IVF_FILE_HDR_SZ) return false;
	LWEVideoIVFHeader Header(HeaderBuf);
	if (memcmp(Header.m_Magic, LWE_IVF_HEADER_MAGIC, 4)) return false;
	if (Header.m_Version != 0) return false;

	const vpx_codec_iface_t *Decoder = nullptr;
	if (Header.m_fourcc == LWE_VP8_FOURCC) Decoder = vpx_codec_vp8_dx();
	else if (Header.m_fourcc == LWE_VP9_FOURCC) Decoder = vpx_codec_vp9_dx();
	if (!Decoder) return false;
	vpx_codec_ctx_t Codec;
	if (vpx_codec_dec_init(&Codec, Decoder, nullptr, 0)) return false;

	LWEVideoDecoderIVF *IVFDecoder = Allocator.Create<LWEVideoDecoderIVF>(Stream, Codec, Header.m_FrameCount, Allocator);
	Player = LWEVideoPlayer(Driver, IVFDecoder, LWVector2i(Header.m_Width, Header.m_Height), LWTimer::GetResolution()*Header.m_TimeDenom / Header.m_TimeNumer, Header.m_FrameCount, LoopCnt, PlayerFlags, Userdata, FinishedCallback, PlaybackSpeed, Allocator);
	return true;
}

uint32_t LWEVideoDecoderIVF::AdvanceFrame(uint8_t *PixelBuffer, const LWVector2i &FrameSize) {
	int8_t FrameHeaderBuf[LWE_IVF_FRAME_HDR_SZ];
	uint32_t Res = m_Stream.Read((char*)FrameHeaderBuf, LWE_IVF_FRAME_HDR_SZ);
	if (!Res) return Error_OutofFrames;
	if(Res!=LWE_IVF_FRAME_HDR_SZ){
		fmt::print("Error Header not expected size.\n");
		return Error_Decoding;
	}
	m_FramePositions[m_Frame] = m_Stream.GetPosition() - LWE_IVF_FRAME_HDR_SZ;
	LWEVideoIVFFrameHeader FrameHeader(FrameHeaderBuf);
	if (FrameHeader.m_FrameSize > m_FrameBufferSize) {
		uint8_t *oBuffer = m_FrameBuffer;
		m_FrameBuffer = m_Allocator.Allocate<uint8_t>(FrameHeader.m_FrameSize);
		m_FrameBufferSize = FrameHeader.m_FrameSize;
		LWAllocator::Destroy(oBuffer);
	}
	if (m_Stream.Read(m_FrameBuffer, FrameHeader.m_FrameSize) != FrameHeader.m_FrameSize) {
		fmt::print("Error occurred while reading frame.\n");
		return Error_Decoding;
	}
	if (vpx_codec_decode(m_Codec, m_FrameBuffer, FrameHeader.m_FrameSize, nullptr, 0)) {
		fmt::print("Error decoding frame.\n");
		return Error_Decoding;
	}
	vpx_codec_iter_t ImgIter = nullptr;
	vpx_image_t *Img = vpx_codec_get_frame(m_Codec, &ImgIter);
	if (!Img) {
		fmt::print("Error getting frame.\n");
		return Error_Decoding;
	}
	LWVector2i hSize = FrameSize / 2;
	for (int32_t y = 0; y < hSize.y; y++) {

		uint8_t *YA = Img->planes[0] + Img->stride[0] * (y * 2);
		uint8_t *YB = YA + Img->stride[0];
		uint8_t *PYA = PixelBuffer + (FrameSize.x*(y * 2));
		uint8_t *PYB = PYA + FrameSize.x;
		uint8_t *U = Img->planes[1] + Img->stride[1] * y;
		uint8_t *V = Img->planes[2] + Img->stride[2] * y;
		uint8_t *PU = PixelBuffer + (FrameSize.x*(FrameSize.y+y));
		uint8_t *PV = PixelBuffer + (FrameSize.x*(FrameSize.y+y)+hSize.x);
		std::copy(YA, YA + FrameSize.x, PYA);
		std::copy(YB, YB + FrameSize.x, PYB);
		std::copy(U, U + hSize.x, PU);
		std::copy(V, V + hSize.x, PV);
	}
	m_Frame++;
	return Error_None;
}

bool LWEVideoDecoderIVF::GoToFrame(uint32_t FrameIdx) {
	int8_t HeaderBuf[LWE_IVF_FRAME_HDR_SZ];
	FrameIdx = std::min<uint32_t>(FrameIdx, m_FrameCount - 1);
	if (!m_FramePositions[FrameIdx]) {
		uint32_t FramePos = LWE_IVF_FILE_HDR_SZ;
		for (uint32_t i = 0; i <= FrameIdx; i++) {
			if (m_FramePositions[i]) FramePos = m_FramePositions[i];
			else {
				if (!i) {
					m_FramePositions[i] = FramePos;
					continue;
				}
				m_Stream.Seek(FramePos, LWFileStream::SeekStart);
				uint32_t Len = m_Stream.Read((char*)HeaderBuf, LWE_IVF_FRAME_HDR_SZ);
				if (Len != LWE_IVF_FRAME_HDR_SZ) {
					fmt::print("Error going to frame.\n");
					return false;
				}
				LWEVideoIVFFrameHeader Header(HeaderBuf);
				m_FramePositions[i] = FramePos + LWE_IVF_FRAME_HDR_SZ + Header.m_FrameSize;
				FramePos = m_FramePositions[i];
			}
		}
	}
	m_Stream.Seek(m_FramePositions[FrameIdx], LWFileStream::SeekStart);
	m_Frame = FrameIdx;
	return true;
}

LWEVideoDecoderIVF::LWEVideoDecoderIVF(LWFileStream &Stream, vpx_codec_ctx &Codec, uint32_t FrameCount, LWAllocator &Allocator) : m_Allocator(Allocator), m_Stream(std::move(Stream)), m_FrameCount(FrameCount) {
	m_Codec = Allocator.Create<vpx_codec_ctx>();
	m_FramePositions = Allocator.Allocate<uint32_t>(m_FrameCount);
	*m_Codec = Codec;
	std::fill(m_FramePositions, m_FramePositions + m_FrameCount, 0);
}

LWEVideoDecoderIVF::~LWEVideoDecoderIVF() {
	LWAllocator::Destroy(m_Codec);
	LWAllocator::Destroy(m_FramePositions);
	LWAllocator::Destroy(m_FrameBuffer);
}