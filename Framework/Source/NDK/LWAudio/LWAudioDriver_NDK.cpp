#include "LWAudio/LWAudioDriver.h"
#include "LWAudio/LWAudioStream.h"
#include "LWCore/LWAllocator.h"
#include "LWPlatform/LWFileStream.h"
#include "LWPlatform/LWWindow.h"

#include <algorithm>
#include <iostream>
#include <vorbis/codec.h>
#include "LWAudio/LWAudioDriver.h"
#include "LWCore/LWAllocator.h"
#include "LWCore/LWTimer.h"
#include "LWPlatform/LWFileStream.h"
#include <algorithm>
#include <iostream>
#include <vorbis/codec.h>

bool LWAudioDriver::Update(uint64_t lCurrentTime, LWWindow *Window) {
	if (!m_LastTime) m_LastTime = lCurrentTime;
	if (Window && m_Flag&NoFocusAudio) {
		if ((Window->GetFlag()&LWWindow::Focused) == 0 && (m_Flag&PausedSystem) == 0) {
			m_Flag |= PausedSystem;
			for (uint32_t i = 0; i < m_ActiveSounds; i++) m_ActiveSoundPool[i]->Pause();

		} else if ((Window->GetFlag()&LWWindow::Focused) != 0 && (m_Flag&PausedSystem) != 0) {
			if ((m_Flag&Paused) == 0) {
				for (uint32_t i = 0; i < m_ActiveSounds; i++) m_ActiveSoundPool[i]->Resume();
			}
			m_Flag &= ~PausedSystem;
		}
	}
	auto ReleaseSoundF = [this](LWSound *S, uint32_t Idx) {
		if (m_CallBacks[CallbackReleased]) m_CallBacks[CallbackReleased](S, this);
		LWSoundContext *Context = S->GetContext();
		std::swap(m_ActiveSoundPool[Idx], m_ActiveSoundPool[m_ActiveSounds - 1]);
		(*Context->m_PlayerItf)->Destroy(Context->m_PlayerItf);
		m_ActiveSounds--;
		fmt::print("Sound Released: {}\n", Idx);
		return;
	};

	auto UpdateSoundF = [this](LWSound *S, uint64_t TimeElapsed) -> bool {
		LWSoundContext *Context = S->GetContext();

		LWAudioStream *Stream = S->GetAudioStream();
		uint64_t lCurrentTime = S->GetTimePlayed();
		uint64_t TimePerSample = (uint32_t)(LWTimer::GetResolution() / (uint64_t)Stream->GetSampleRate());

		uint64_t TotalTime = Stream->GetSampleLength()*TimePerSample;
		uint32_t FrameSize = Stream->GetSampleSize()*Stream->GetChannels();
		uint32_t SamplesPerSlice = LWSOUND_RESERVEBUFFERSIZE / FrameSize;
		if (Stream->GetFormatType() == LWAudioStream::FormatRaw) SamplesPerSlice = Stream->GetSampleLength();

		uint64_t TimePerSlice = SamplesPerSlice*TimePerSample;

		if (S->GetFlag()&LWSound::Playing) lCurrentTime += TimeElapsed;


		uint32_t SliceCount = (Stream->GetSampleLength() + (SamplesPerSlice - 1)) / SamplesPerSlice;

		uint32_t SeekSamples = 0;
		uint32_t FinishCount = S->GetFinishedCount();
		uint32_t Flag = S->GetFlag();
		uint64_t LoadedSamples = S->GetSamplesLoaded();

		if (Flag&LWSound::RequestSeek) {
			(*Context->m_BufferQueueItf)->Clear(Context->m_BufferQueueItf);
			lCurrentTime = TimePerSample*Stream->GetSampleLength()*FinishCount + TimePerSample*S->GetSeekPosition();
			uint32_t TargetSlice = (uint32_t)((lCurrentTime%TotalTime) / TimePerSlice);
			SeekSamples = S->GetSeekPosition() - (TargetSlice*SamplesPerSlice);
			LoadedSamples = FinishCount*Stream->GetSampleLength() + SamplesPerSlice*TargetSlice;
			Flag |= (Flag&LWSound::Playing) ? LWSound::RequestPlay : 0;
		}

		if (lCurrentTime >= TotalTime*(FinishCount + 1)) {
			S->SetFinishCount(FinishCount + 1);
			S->SetFlag(Flag);
			if (m_CallBacks[CallbackFinished]) m_CallBacks[CallbackFinished](S, this);
			Flag = S->GetFlag();
		}
		uint32_t PlayedSamples = (uint32_t)(lCurrentTime / TimePerSample);
		uint32_t PlayedSlices = (PlayedSamples%Stream->GetSampleLength()) / SamplesPerSlice + SliceCount*(PlayedSamples / Stream->GetSampleLength());
		uint64_t LoadedSlices = (LoadedSamples%Stream->GetSampleLength()) / SamplesPerSlice + SliceCount*(LoadedSamples / Stream->GetSampleLength());
		uint64_t PreloadedSlices = LoadedSlices >= PlayedSlices ? (LoadedSlices - PlayedSlices) : 0;

		//LWSOUND_RESEVECNT-1 cause 1 or 2 samples may still need to be played, and are not cleared from the buffer yet, so we leave one block always open.
		while (PreloadedSlices < (LWSOUND_RESERVECNT - 1) && LoadedSamples != S->GetTotalSamples()) {
			uint32_t SliceIdx = (LoadedSlices%SliceCount);
			uint32_t SliceSamples = (SliceIdx == (SliceCount - 1)) ? (Stream->GetSampleLength() - (SamplesPerSlice*SliceIdx)) : SamplesPerSlice;

			uint32_t ReserveIdx = Context->m_ReserveIdx%LWSOUND_RESERVECNT;
			//std::cout << "Loading samples: " << SliceSamples << " Pos: " << SliceIdx*SamplesPerSlice << " Pre: " << PreloadedSlices << " Total: " << Stream->GetSampleLength() << std::endl;

			char *Reserve = Context->m_ReserveBuffer + ReserveIdx*LWSOUND_RESERVEBUFFERSIZE;
			char *Samples = Stream->DecodeSamples(Reserve, SliceIdx*SamplesPerSlice, SliceSamples);
			if (Reserve == Samples) Context->m_ReserveIdx++;
			(*Context->m_BufferQueueItf)->Enqueue(Context->m_BufferQueueItf, Samples + SeekSamples*FrameSize, (SliceSamples*FrameSize) - SeekSamples*FrameSize);

			LoadedSamples += SliceSamples;
			PreloadedSlices++;
			LoadedSlices++;
			SeekSamples = 0;
		}
		S->SetSamplesLoaded(LoadedSamples);
		S->SetTimePlayed(lCurrentTime);

		if ((Flag&LWSound::Dirty) == 0) {
			S->SetFlag(Flag&~(LWSound::Dirty | LWSound::RequestPlay | LWSound::RequestStop | LWSound::RequestSeek));
			return false;
		}
		if (Flag&LWSound::RequestPlay) {
			(*Context->m_PlayItf)->SetPlayState(Context->m_PlayItf, SL_PLAYSTATE_PLAYING);
			Flag |= LWSound::Playing;
		} else if (Flag&LWSound::RequestStop) {
			(*Context->m_PlayItf)->SetPlayState(Context->m_PlayItf, SL_PLAYSTATE_PAUSED);
			Flag &= ~LWSound::Playing;
		}
		SLmillibel Max = 0;
		SLmillibel Min = -3000;
		if (S->GetVolume() <= std::numeric_limits<float>::epsilon() || m_MasterVolume <= std::numeric_limits<float>::epsilon()) Min = SL_MILLIBEL_MIN;
		//(*Context->m_VolumeItf)->GetMaxVolumeLevel(Context->m_VolumeItf, &Max);
		SLmillibel Vol = Min +(SLmillibel)(S->GetVolume()*m_MasterVolume*(Max- Min));
		(*Context->m_VolumeItf)->SetVolumeLevel(Context->m_VolumeItf, Vol);
		(*Context->m_VolumeItf)->SetStereoPosition(Context->m_VolumeItf, (SLmillibel)(S->GetPan()*1000.0f));
		S->SetFlag(Flag&~(LWSound::Dirty | LWSound::RequestPlay | LWSound::RequestStop | LWSound::RequestSeek));
		return true;
	};

	uint64_t TimeElapsed = lCurrentTime >= m_LastTime ? (lCurrentTime - m_LastTime) : 0;
	bool Changed = false;
	if (m_Flag&Dirty) {
		Changed = true;
	}
	for (uint32_t i = 0; i < m_ActiveSounds; i++) {
		LWSound *S = m_ActiveSoundPool[i];
		uint32_t Flag = S->GetFlag();
		if (Flag&LWSound::Release) {
			ReleaseSoundF(S, i);
			i--;
		} else if (UpdateSoundF(S, TimeElapsed)) Changed = true;
	}
	m_Flag &= ~Dirty;
	m_LastTime = lCurrentTime;
	return true;
}

/*
bool LWAudioDriver::Update(uint64_t CurrentTime) {
	if (!m_LastTime) m_LastTime = CurrentTime;


	auto UpdateSoundF = [this, &LoadSlice](LWSound *S, uint64_t TimeElapsed) -> bool {
		LWSoundContext *Context = S->GetContext();

		uint32_t LoopCnt = S->GetLoopCount();
		uint32_t TotalSamples = S->GetSampleLength();
		uint64_t CurrentTime = S->GetTimePlayed();
		uint64_t TimePerSample = LWTimer::GetResolution() / S->GetSampleRate();
		uint64_t TotalTime = TotalSamples*TimePerSample;
		uint32_t SamplesPerSlice = (LWSound::ReserveSize - sizeof(LWAudioBufferHeader)) / S->GetSampleSize();
		uint64_t TimePerSlice = SamplesPerSlice*TimePerSample;

		if (S->GetFlag()&LWSound::Playing) CurrentTime += TimeElapsed;

		char *ActiveBuffer = S->GetActiveBuffer();
		LWFileStream *ActiveStream = S->GetActiveStream();
		if (!ActiveStream) SamplesPerSlice = TotalSamples;
		uint32_t SliceCount = TotalSamples / SamplesPerSlice;


		uint32_t SeekSamples = 0;
		uint32_t FinishCount = S->GetFinishedCount();
		uint32_t Flag = S->GetFlag();
		uint32_t LoadedSamples = S->GetSamplesLoaded();
		if (Flag&LWSound::RequestSeek) {
			(*Context->m_BufferQueueItf)->Clear(Context->m_BufferQueueItf);
			CurrentTime = TimePerSample*TotalSamples*FinishCount + TimePerSample*S->GetSeekPosition();
			uint32_t TargetSlice = (uint32_t)((CurrentTime%TotalTime) / TimePerSlice);
			SeekSamples = S->GetSeekPosition() - (TargetSlice*SamplesPerSlice);
			LoadedSamples = FinishCount*TotalSamples + SamplesPerSlice*TargetSlice;
			Flag |= (Flag&LWSound::Playing) ? LWSound::RequestPlay : 0;
		}
		if (CurrentTime >= TotalTime*(FinishCount + 1)) {
			S->SetFinishCount(FinishCount + 1);
			S->SetFlag(Flag);
			if (m_CallBacks[CallbackFinished]) m_CallBacks[CallbackFinished](S, this);
			Flag = S->GetFlag();
		}
		uint32_t PlayedSlices = (uint32_t)(CurrentTime / TimePerSlice);
		uint32_t LoadedSlices = (uint32_t)(LoadedSamples / SamplesPerSlice);
		uint32_t TotalSlices = SliceCount*(LoopCnt + 1);
		while (PlayedSlices + (LWSound::ReserveSlices - 1) > LoadedSlices && LoadedSlices != TotalSlices) {
			uint32_t SliceIdx = (LoadedSlices%SliceCount);
			char *Samples = nullptr;
			uint32_t SliceSamples = (SliceIdx == SliceCount) ? (TotalSamples - (SamplesPerSlice*SliceIdx)) : SamplesPerSlice;
			if (ActiveStream) {
				Samples = LoadSlice(ActiveStream, ActiveBuffer, S, SliceSamples, SliceIdx);
				S->IncrementReserveIndex();
			} else {
				Samples = ActiveBuffer + sizeof(LWAudioBufferHeader);
			}
			(*Context->m_BufferQueueItf)->Enqueue(Context->m_BufferQueueItf, Samples + SeekSamples, (SliceSamples*S->GetSampleSize()) - SeekSamples);
			//XAUDIO2_BUFFER XBuf = { ((LoadedSamples + SliceSamples) == TotalSlices) ? (uint32_t)XAUDIO2_END_OF_STREAM : 0, SliceSamples*S->GetSampleSize(), (BYTE*)Samples, SeekSamples, SliceSamples - SeekSamples, 0, 0, 0, S };
			//Context->m_Source->SubmitSourceBuffer(&XBuf, nullptr);
			LoadedSlices++;
			LoadedSamples += SliceSamples;
			SeekSamples = 0;
		}
		S->SetSamplesLoaded(LoadedSamples);
		S->SetTimePlayed(CurrentTime);

		if ((Flag&LWSound::Dirty) == 0 && (m_Flag&Dirty)==0) {
			S->SetFlag(Flag&~(LWSound::Dirty | LWSound::RequestPlay | LWSound::RequestStop | LWSound::RequestSeek));
			return false;
		}
		if (Flag&LWSound::RequestPlay) {
			(*Context->m_PlayItf)->SetPlayState(Context->m_PlayItf, SL_PLAYSTATE_PLAYING );
			Flag |= LWSound::Playing;
		} else if (LWSound::RequestStop) {
			(*Context->m_PlayItf)->SetPlayState(Context->m_PlayItf, SL_PLAYSTATE_PAUSED);
			Flag &= ~LWSound::Playing;
		}
		SLmillibel Max = 0;
		(*Context->m_VolumeItf)->GetMaxVolumeLevel(Context->m_VolumeItf, &Max);

		(*Context->m_VolumeItf)->SetVolumeLevel(Context->m_VolumeItf, (SLmillibel)(S->GetVolume()*m_MasterVolume*Max));
		(*Context->m_VolumeItf)->SetStereoPosition(Context->m_VolumeItf, (SLmillibel)(S->GetPan()*1000.0f));
		S->SetFlag(Flag&~(LWSound::Dirty | LWSound::RequestPlay | LWSound::RequestStop | LWSound::RequestSeek));
		return true;
	};

	uint64_t TimeElapsed = CurrentTime >= m_LastTime ? (CurrentTime - m_LastTime) : 0;
	
	for (uint32_t i = 0; i < m_ActiveSounds; i++) {
		LWSound *S = m_ActiveSoundPool[i];
		uint32_t Flag = S->GetFlag();
		if (Flag&LWSound::Release) {
			ReleaseSoundF(S);
			i--;
		} else UpdateSoundF(S, TimeElapsed);
	}
	m_Flag &= ~Dirty;
	m_LastTime = CurrentTime;
	return true;
}*/

LWSound *LWAudioDriver::CreateSound(LWAudioStream *Stream, uint32_t Flags, void *UserData, uint32_t LoopCount /* = 0 */, float Volume /* = 1.0f */, float Pitch /* = 1.0f */, float Pan /* = 0.0f */) {
	if (m_ActiveSounds >= MaxSounds) return nullptr;
	if (m_Flag&(Paused | PausedSystem)) return nullptr;
	LWSound *Target = m_ActiveSoundPool[m_ActiveSounds];
	LWSoundContext *SndCntx = Target->GetContext();
	uint32_t FrameSize = Stream->GetSampleSize()*Stream->GetChannels();

	const int32_t InterfaceCnt = 2;
	const SLInterfaceID SLInterfaces[InterfaceCnt] = { SL_IID_BUFFERQUEUE, SL_IID_VOLUME };
	const SLboolean SLReqs[InterfaceCnt] = { true, true };

	fmt::print("Format: {} Rate: {} Size: {} Samples: {} Channels: {}\n", Stream->GetSampleType(), Stream->GetSampleRate(), Stream->GetSampleSize(), Stream->GetSampleLength(), Stream->GetChannels());

	SLDataLocator_AndroidSimpleBufferQueue BufQue = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, LWSOUND_RESERVECNT - 1 };
	SLDataFormat_PCM Format = { SL_DATAFORMAT_PCM, Stream->GetChannels(), Stream->GetSampleRate() * 1000/* milliHerz */, Stream->GetSampleSize() * 8, Stream->GetSampleSize() * 8, 0/*! Stereo channels */, SL_BYTEORDER_LITTLEENDIAN };
	SLDataLocator_OutputMix DataLoc = { SL_DATALOCATOR_OUTPUTMIX, m_Context.m_OutputMixItf };
	SLDataSink DataSink = { &DataLoc, nullptr };
	SLDataSource DataSrc = { &BufQue, &Format };

	if ((*m_Context.m_EngineItf)->CreateAudioPlayer(m_Context.m_EngineItf, &SndCntx->m_PlayerItf, &DataSrc, &DataSink, InterfaceCnt, SLInterfaces, SLReqs) != SL_RESULT_SUCCESS) {
		fmt::print("Create failed.\n");
		return nullptr;
	}
	SLuint32 Res = (*SndCntx->m_PlayerItf)->Realize(SndCntx->m_PlayerItf, false);
	if (Res != SL_RESULT_SUCCESS) {
		fmt::print("Error 'Realize': {:#x}", Res);
	} else if ((Res = (*SndCntx->m_PlayerItf)->GetInterface(SndCntx->m_PlayerItf, SL_IID_PLAY, &SndCntx->m_PlayItf)) != SL_RESULT_SUCCESS) {
		fmt::print("Error 'SL_IID_PLAY': {:#x}", Res);
	} else if ((Res = (*SndCntx->m_PlayerItf)->GetInterface(SndCntx->m_PlayerItf, SL_IID_BUFFERQUEUE, &SndCntx->m_BufferQueueItf)) != SL_RESULT_SUCCESS){
		fmt::print("Error 'SL_IID_BUFFERQUEUE': {:#x}", Res);
	} else if ((Res = (*SndCntx->m_PlayerItf)->GetInterface(SndCntx->m_PlayerItf, SL_IID_VOLUME, &SndCntx->m_VolumeItf)) != SL_RESULT_SUCCESS) {
		fmt::print("Error 'SL_IID_VOLUME': {:#x}", Res);
	}
	if(Res!=SL_RESULT_SUCCESS){
		(*SndCntx->m_PlayerItf)->Destroy(SndCntx->m_PlayerItf);
		return nullptr;
	}

	(*SndCntx->m_VolumeItf)->EnableStereoPosition(SndCntx->m_VolumeItf, true);
	(*SndCntx->m_VolumeItf)->SetStereoPosition(SndCntx->m_VolumeItf, 0);

	Target->SetUserData(UserData).SetAudioStream(Stream).SetVolume(Volume).SetPan(Pan).SetPitch(Pitch);
	Target->SetTimePlayed(0).SetSamplesLoaded(0).SetTotalSamples((uint64_t)(LoopCount + 1)*(uint64_t)Stream->GetSampleLength());
	Target->SetFinishCount(0).SetFlag(Flags | LWSound::Dirty);
	if (m_CallBacks[CallbackCreate]) m_CallBacks[CallbackCreate](Target, this);
	m_ActiveSounds++;
	fmt::print("Sound made: {}\n", m_ActiveSounds);
	return Target;
}

/*
LWSound *LWAudioDriver::CreateSound(char *Buffer, uint32_t BufferLen, uint32_t Flags, void *UserData, uint32_t LoopCount, float Volume, float Pitch, float Pan) {
	if (m_ActiveSounds >= MaxSounds) return nullptr;
	LWSound *Target = m_ActiveSoundPool[m_ActiveSounds];
	LWAudioBufferHeader *Header = (LWAudioBufferHeader*)Buffer;
	LWSoundContext *Context = Target->GetContext();

	const int32_t InterfaceCnt = 2;
	const SLInterfaceID SLInterfaces[InterfaceCnt] = { SL_IID_BUFFERQUEUE, SL_IID_VOLUME };
	const SLboolean SLReqs[InterfaceCnt] = { true, true };

	uint32_t SampleRate = Header->m_Rate;
	uint32_t SampleSize = Header->m_SampleSize * Header->m_Channels;
	uint32_t SampleLength = (BufferLen - sizeof(LWAudioBufferHeader)) / SampleSize;
	std::cout << "Rate: " << SampleRate << " Size: " << SampleSize << " Length: " << SampleLength << std::endl;

	SLDataLocator_AndroidSimpleBufferQueue BufQue = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, LWSound::ReserveSlices-1 };
	SLDataFormat_PCM Format = { SL_DATAFORMAT_PCM, Header->m_Channels, Header->m_Rate * 1000/* milliHerz , Header->m_SampleSize * 8, Header->m_SampleSize * 8, 0/*! Stereo channels , SL_BYTEORDER_LITTLEENDIAN };
	SLDataLocator_OutputMix DataLoc = { SL_DATALOCATOR_OUTPUTMIX, m_Context.m_OutputMixItf };
	SLDataSink DataSink = { &DataLoc, nullptr };
	SLDataSource DataSrc = { &BufQue, &Format };
	
	if ((*m_Context.m_EngineItf)->CreateAudioPlayer(m_Context.m_EngineItf, &Context->m_PlayerItf, &DataSrc, &DataSink, InterfaceCnt, SLInterfaces, SLReqs) != SL_RESULT_SUCCESS) {
		std::cout << "Create failed." << std::endl;
		return nullptr;
	}
	bool Success = true;
	if ((*Context->m_PlayerItf)->Realize(Context->m_PlayerItf, false) != SL_RESULT_SUCCESS) Success = false;
	else if ((*Context->m_PlayerItf)->GetInterface(Context->m_PlayerItf, SL_IID_PLAY, &Context->m_PlayItf) != SL_RESULT_SUCCESS) Success = false;
	else if ((*Context->m_PlayerItf)->GetInterface(Context->m_PlayerItf, SL_IID_BUFFERQUEUE, &Context->m_BufferQueueItf) != SL_RESULT_SUCCESS) Success = false;
	else if ((*Context->m_PlayerItf)->GetInterface(Context->m_PlayerItf, SL_IID_VOLUME, &Context->m_VolumeItf) != SL_RESULT_SUCCESS) Success = false;

	if(!Success){
		(*Context->m_PlayerItf)->Destroy(Context->m_PlayerItf);
		return nullptr;
	}
	
	(*Context->m_VolumeItf)->EnableStereoPosition(Context->m_VolumeItf, true);
	(*Context->m_VolumeItf)->SetStereoPosition(Context->m_VolumeItf, 0);

	Target->SeekTo(0).SetUserData(UserData).SetSampleSize(SampleSize).SetSampleRate(SampleRate);
	Target->SetSampleLength(SampleLength).SetVolume(Volume).SetPan(Pan).SetPitch(Pitch).SetLoopCount(LoopCount);
	Target->SetFlag(Flags | LWSound::Dirty);
	Target->SetSamplesLoaded(0).SetTimePlayed(0).SetFinishCount(0);
	Target->SetActiveBuffer(Buffer).SetActiveStream(nullptr);
	if (m_CallBacks[CallbackCreate]) m_CallBacks[CallbackCreate](Target, this);
	m_ActiveSounds++;
	return Target;
}

LWSound *LWAudioDriver::CreateSound(LWFileStream *Stream, uint32_t StreamFormat, uint32_t Flags, void *UserData, uint32_t LoopCount, float Volume, float Pitch, float Pan) {
	if (m_ActiveSounds >= MaxSounds) return nullptr;
	LWSound *Target = m_ActiveSoundPool[m_ActiveSounds];
	char *Reserve = Target->GetReserveBuffer() + Target->GetReserveIndex()*LWSound::ReserveSize;

	const int32_t InterfaceCnt = 2;
	const SLInterfaceID SLInterfaces[InterfaceCnt] = { SL_IID_BUFFERQUEUE, SL_IID_VOLUME };
	const SLboolean SLReqs[InterfaceCnt] = { true, true };
	uint32_t Len = 0;
	//WAVEFORMATEX WaveFmt;
	if (StreamFormat == FormatWAV) {
		Len = LoadSoundWAV(Stream, Reserve, LWSound::ReserveSize, 0);
	} else if (StreamFormat == FormatVORBIS) {
		Len = LoadSoundOGG(Stream, Reserve, LWSound::ReserveSize, 0);
	} else {
		return nullptr;
	}
	LWAudioBufferHeader *Header = (LWAudioBufferHeader*)Reserve;
	LWSoundContext *Context = Target->GetContext();



	uint32_t SampleRate = Header->m_Rate;
	uint32_t SampleSize = Header->m_SampleSize * Header->m_Channels;
	uint32_t SampleLength = (Len - sizeof(LWAudioBufferHeader)) / SampleSize;
	std::cout << "Rate: " << SampleRate << " Size: " << SampleSize << " Length: " << SampleLength << std::endl;

	SLDataLocator_AndroidSimpleBufferQueue BufQue = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, LWSound::ReserveSlices - 1 };
	SLDataFormat_PCM Format = { SL_DATAFORMAT_PCM, Header->m_Channels, Header->m_Rate * 1000/* milliHerz , Header->m_SampleSize * 8, Header->m_SampleSize * 8, 0/*! Stereo channels , SL_BYTEORDER_LITTLEENDIAN };
	SLDataLocator_OutputMix DataLoc = { SL_DATALOCATOR_OUTPUTMIX, m_Context.m_OutputMixItf };
	SLDataSink DataSink = { &DataLoc, nullptr };
	SLDataSource DataSrc = { &BufQue, &Format };

	if ((*m_Context.m_EngineItf)->CreateAudioPlayer(m_Context.m_EngineItf, &Context->m_PlayerItf, &DataSrc, &DataSink, InterfaceCnt, SLInterfaces, SLReqs) != SL_RESULT_SUCCESS) return nullptr;

	bool Success = true;
	if ((*Context->m_PlayerItf)->Realize(Context->m_PlayerItf, false) != SL_RESULT_SUCCESS) Success = false;
	else if ((*Context->m_PlayerItf)->GetInterface(Context->m_PlayerItf, SL_IID_PLAY, &Context->m_PlayItf) != SL_RESULT_SUCCESS) Success = false;
	else if ((*Context->m_PlayerItf)->GetInterface(Context->m_PlayerItf, SL_IID_BUFFERQUEUE, &Context->m_BufferQueueItf) != SL_RESULT_SUCCESS) Success = false;
	else if ((*Context->m_PlayerItf)->GetInterface(Context->m_PlayerItf, SL_IID_VOLUME, &Context->m_VolumeItf) != SL_RESULT_SUCCESS) Success = false;

	if (!Success) {
		(*Context->m_PlayerItf)->Destroy(Context->m_PlayerItf);
		return nullptr;
	}
	(*Context->m_VolumeItf)->EnableStereoPosition(Context->m_VolumeItf, true);
	(*Context->m_VolumeItf)->SetStereoPosition(Context->m_VolumeItf, 0);
	
	Target->SeekTo(0).SetUserData(UserData).SetSampleSize(SampleSize).SetSampleRate(SampleRate);
	Target->SetSampleLength(SampleLength).SetVolume(Volume).SetPan(Pan).SetPitch(Pitch).SetLoopCount(LoopCount);
	Target->SetFlag(Flags | LWSound::Dirty | (StreamFormat << LWSound::ActiveStreamFormatOffset));
	Target->SetSamplesLoaded(0).SetTimePlayed(0).SetFinishCount(0);
	Target->SetActiveBuffer(nullptr).SetActiveStream(Stream);
	if (m_CallBacks[CallbackCreate]) m_CallBacks[CallbackCreate](Target, this);
	m_ActiveSounds++;
	return Target;
}

LWSound *LWAudioDriver::CreateSound(char *StreamBuffer, uint32_t StreamLen, uint32_t StreamFormat, uint32_t Flags, void *UserData, uint32_t LoopCount, float Volume, float Pitch, float Pan) {
	if (m_ActiveSounds >= MaxSounds) return nullptr;
	LWSound *Target = m_ActiveSoundPool[m_ActiveSounds];
	char *Reserve = Target->GetReserveBuffer() + Target->GetReserveIndex()*LWSound::ReserveSize;

	const int32_t InterfaceCnt = 2;
	const SLInterfaceID SLInterfaces[InterfaceCnt] = { SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
	const SLboolean SLReqs[InterfaceCnt] = { true, true };
	uint32_t Len = 0;
	//WAVEFORMATEX WaveFmt;
	if (StreamFormat == FormatWAV) {
		Len = LoadSoundWAV(StreamBuffer, StreamLen, Reserve, LWSound::ReserveSize, 0);
	} else if (StreamFormat == FormatVORBIS) {
		Len = LoadSoundOGG(StreamBuffer, StreamLen, Reserve, LWSound::ReserveSize, 0);
	} else {
		return nullptr;
	}
	LWAudioBufferHeader *Header = (LWAudioBufferHeader*)Reserve;
	LWSoundContext *Context = Target->GetContext();

	uint32_t SampleRate = Header->m_Rate;
	uint32_t SampleSize = Header->m_SampleSize * Header->m_Channels;
	uint32_t SampleLength = (Len - sizeof(LWAudioBufferHeader)) / SampleSize;
	std::cout << "Rate: " << SampleRate << " Size: " << SampleSize << " Length: " << SampleLength << std::endl;

	SLDataLocator_AndroidSimpleBufferQueue BufQue = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, LWSound::ReserveSlices - 1 };
	SLDataFormat_PCM Format = { SL_DATAFORMAT_PCM, Header->m_Channels, Header->m_Rate*1000/* milliHerz, Header->m_SampleSize*8, Header->m_SampleSize*8, 0/*! Stereo channels, SL_BYTEORDER_LITTLEENDIAN };
	SLDataLocator_OutputMix DataLoc = { SL_DATALOCATOR_OUTPUTMIX, m_Context.m_OutputMixItf };
	SLDataSink DataSink = { &DataLoc, nullptr };
	SLDataSource DataSrc = { &BufQue, &Format };

	if ((*m_Context.m_EngineItf)->CreateAudioPlayer(m_Context.m_EngineItf, &Context->m_PlayerItf, &DataSrc, &DataSink, InterfaceCnt, SLInterfaces, SLReqs) != SL_RESULT_SUCCESS) return nullptr;

	bool Success = true;
	if ((*Context->m_PlayerItf)->Realize(Context->m_PlayerItf, false) != SL_RESULT_SUCCESS) Success = false;
	else if ((*Context->m_PlayerItf)->GetInterface(Context->m_PlayerItf, SL_IID_PLAY, &Context->m_PlayItf) != SL_RESULT_SUCCESS) Success = false;
	else if ((*Context->m_PlayerItf)->GetInterface(Context->m_PlayerItf, SL_IID_BUFFERQUEUE, &Context->m_BufferQueueItf) != SL_RESULT_SUCCESS) Success = false;
	else if ((*Context->m_PlayerItf)->GetInterface(Context->m_PlayerItf, SL_IID_VOLUME, &Context->m_VolumeItf) != SL_RESULT_SUCCESS) Success = false;
	
	if (!Success) {
		(*Context->m_PlayerItf)->Destroy(Context->m_PlayerItf);
		return nullptr;
	}

	(*Context->m_VolumeItf)->EnableStereoPosition(Context->m_VolumeItf, true);
	(*Context->m_VolumeItf)->SetStereoPosition(Context->m_VolumeItf, 0);
	Target->SeekTo(0).SetUserData(UserData).SetSampleSize(SampleSize).SetSampleRate(SampleRate);
	Target->SetSampleLength(SampleLength).SetVolume(Volume).SetPan(Pan).SetPitch(Pitch).SetLoopCount(LoopCount);
	Target->SetFlag(Flags | LWSound::Dirty | (StreamFormat << LWSound::ActiveStreamFormatOffset));
	Target->SetSamplesLoaded(0).SetTimePlayed(0).SetFinishCount(0);
	Target->SetActiveBuffer(StreamBuffer).SetActiveStream((LWFileStream*)StreamLen);
	if (m_CallBacks[CallbackCreate]) m_CallBacks[CallbackCreate](Target, this);
	m_ActiveSounds++;
	return Target;
}*/

LWAudioDriver::LWAudioDriver(void *UserData, LWAudioCallback FinishedCallback, LWAudioCallback CreateCallback, LWAudioCallback ReleaseCallback) : m_Flag(Dirty|NoFocusAudio) {
	m_CallBacks[CallbackCreate] = CreateCallback;
	m_CallBacks[CallbackFinished] = FinishedCallback;
	m_CallBacks[CallbackReleased] = ReleaseCallback;
	for (uint32_t i = 0; i < MaxSounds; i++) m_ActiveSoundPool[i] = m_SoundPool + i;

	if (slCreateEngine(&m_Context.m_EngineObjectItf, 0, nullptr, 0, nullptr, nullptr) != SL_RESULT_SUCCESS) m_Flag |= Error;
	else if ((*m_Context.m_EngineObjectItf)->Realize(m_Context.m_EngineObjectItf, false) != SL_RESULT_SUCCESS) m_Flag |= Error;
	else if ((*m_Context.m_EngineObjectItf)->GetInterface(m_Context.m_EngineObjectItf, SL_IID_ENGINE, &m_Context.m_EngineItf) != SL_RESULT_SUCCESS) m_Flag |= Error;
	else if ((*m_Context.m_EngineItf)->CreateOutputMix(m_Context.m_EngineItf, &m_Context.m_OutputMixItf, 0, nullptr, nullptr) != SL_RESULT_SUCCESS) m_Flag |= Error;
	else if ((*m_Context.m_OutputMixItf)->Realize(m_Context.m_OutputMixItf, false) != SL_RESULT_SUCCESS) m_Flag |= Error;
	static JNIEnv *Env = nullptr;
	static uint32_t EnvCounter = 0;

	static jclass ActivityClass = 0;
	static jmethodID Activity_setVolumeControlStream = 0;
	const int32_t STREAM_DEFAULT = 0x80000000;
	const int32_t STREAM_SYSTEM = 0x1;
	const int32_t STREAM_RING = 0x2;
	const int32_t STREAM_MUSIC = 0x3;
	const int32_t STREAM_ALARM = 0x4;
	const int32_t STREAM_NOTIFICATION = 0x5;
	if (EnvCounter != LWAppContext.m_AppEnvCounter) {
		Env = LWAppContext.m_AppEnv;
		EnvCounter = LWAppContext.m_AppEnvCounter;

		ActivityClass = Env->FindClass("android/app/NativeActivity");
		Activity_setVolumeControlStream = Env->GetMethodID(ActivityClass, "setVolumeControlStream", "(I)V");
	}
	Env->CallVoidMethod(LWAppContext.m_App->clazz, Activity_setVolumeControlStream, STREAM_MUSIC);
}

LWAudioDriver::~LWAudioDriver() {
	for (uint32_t i = 0; i < m_ActiveSounds; i++) ReleaseSound(m_ActiveSoundPool[i]);
	Update(0, nullptr);
	(*m_Context.m_OutputMixItf)->Destroy(m_Context.m_OutputMixItf);
	(*m_Context.m_EngineObjectItf)->Destroy(m_Context.m_EngineObjectItf);
	static JNIEnv *Env = nullptr;
	static uint32_t EnvCounter = 0;

	static jclass ActivityClass = 0;
	static jmethodID Activity_setVolumeControlStream = 0;
	const int32_t STREAM_DEFAULT = 0x80000000;
	const int32_t STREAM_SYSTEM = 0x1;
	const int32_t STREAM_RING = 0x2;
	const int32_t STREAM_MUSIC = 0x3;
	const int32_t STREAM_ALARM = 0x4;
	const int32_t STREAM_NOTIFICATION = 0x5;
	if (EnvCounter != LWAppContext.m_AppEnvCounter) {
		Env = LWAppContext.m_AppEnv;
		EnvCounter = LWAppContext.m_AppEnvCounter;

		ActivityClass = Env->FindClass("android/app/NativeActivity");
		Activity_setVolumeControlStream = Env->GetMethodID(ActivityClass, "setVolumeControlStream", "(I)V");
	}
	Env->CallVoidMethod(LWAppContext.m_App->clazz, Activity_setVolumeControlStream, STREAM_DEFAULT);
}