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


void AudioQueueCallback(void *UserData, AudioQueueRef In, AudioQueueBufferRef BufferRef) {
}

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
		AudioQueueDispose(Context->m_Queue, false);
		m_ActiveSounds--;
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
		//if (Stream->GetFormatType() == LWAudioStream::FormatRaw) SamplesPerSlice = Stream->GetSampleLength();

		uint64_t TimePerSlice = SamplesPerSlice*TimePerSample;

		if (S->GetFlag()&LWSound::Playing) lCurrentTime += TimeElapsed;


		uint32_t SliceCount = (Stream->GetSampleLength() + (SamplesPerSlice - 1)) / SamplesPerSlice;

		uint32_t SeekSamples = 0;
		uint32_t FinishCount = S->GetFinishedCount();
		uint32_t Flag = S->GetFlag();
		uint64_t LoadedSamples = S->GetSamplesLoaded();

		if (Flag&LWSound::RequestSeek) {
			//(*Context->m_BufferQueueItf)->Clear(Context->m_BufferQueueItf);
			AudioQueueFlush(Context->m_Queue);
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

			AudioQueueBufferRef ABuf = Context->m_QueueBuffers[ReserveIdx];
			Stream->DecodeSamples((char*)ABuf->mAudioData, SliceIdx*SamplesPerSlice, SliceSamples, true);
            ABuf->mAudioDataByteSize = SliceSamples*FrameSize-SeekSamples*FrameSize;
			Context->m_ReserveIdx++;
            AudioStreamPacketDescription Desc = { SeekSamples*FrameSize, 0, (SliceSamples*FrameSize) - SeekSamples*FrameSize};
			OSStatus Err = AudioQueueEnqueueBuffer(Context->m_Queue, ABuf, 1, &Desc);
			if (Err) {
				std::cout << "Error enqueuing: " << Err << std::endl;
			}


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
			AudioQueueStart(Context->m_Queue, nullptr);
			Flag |= LWSound::Playing;
		} else if (Flag&LWSound::RequestStop) {
			AudioQueueStop(Context->m_Queue, true);
			Flag &= ~LWSound::Playing;
		}
		AudioQueueSetParameter(Context->m_Queue, kAudioQueueParam_Volume, S->GetVolume()*m_MasterVolume);
		AudioQueueSetParameter(Context->m_Queue, kAudioQueueParam_Pitch, S->GetPitch());
		AudioQueueSetParameter(Context->m_Queue, kAudioQueueParam_Pan, S->GetPan());
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

LWSound *LWAudioDriver::CreateSound(LWAudioStream *Stream, uint32_t Flags, void *UserData, uint32_t LoopCount /* = 0 */, float Volume /* = 1.0f */, float Pitch /* = 1.0f */, float Pan /* = 0.0f */) {
	if (m_ActiveSounds >= MaxSounds) return nullptr;
	if (m_Flag&(Paused | PausedSystem)) return nullptr;
	LWSound *Target = m_ActiveSoundPool[m_ActiveSounds];
	LWSoundContext *SndCntx = Target->GetContext();
	uint32_t FrameSize = Stream->GetSampleSize()*Stream->GetChannels();
	uint32_t TypeFlags[] = { kAudioFormatFlagIsSignedInteger, kAudioFormatFlagIsFloat};
	
	AudioStreamBasicDescription Desc = { (Float64)Stream->GetSampleRate(),
										kAudioFormatLinearPCM,
										kAudioFormatFlagIsPacked | kAudioFormatFlagsNativeEndian | TypeFlags[Stream->GetSampleType()],
										Stream->GetSampleSize()*Stream->GetChannels(),
										1,
										Stream->GetSampleSize()*Stream->GetChannels(),
										Stream->GetChannels(),
										Stream->GetSampleSize() * 8,
										0 };
	OSStatus Err = AudioQueueNewOutput(&Desc, AudioQueueCallback, nullptr, nullptr, nullptr, 0, &SndCntx->m_Queue);
	if (Err) {
		std::cout << "Error making audio: " << (int32_t)Err << std::endl;
		return nullptr;
	}
	for (uint32_t i = 0; i < LWSOUND_RESERVECNT; i++) {
		Err = AudioQueueAllocateBuffer(SndCntx->m_Queue, LWSOUND_RESERVEBUFFERSIZE, &SndCntx->m_QueueBuffers[i]);
		if (Err) {
			std::cout << "Error making buffer: " << (int32_t)Err << std::endl;
			return nullptr;
		} //else std::cout << "Buffer Size: " << SndCntx->m_QueueBuffers[i]->mAudioDataBytesCapacity << std::endl;
	}

	Target->SetUserData(UserData).SetAudioStream(Stream).SetVolume(Volume).SetPan(Pan).SetPitch(Pitch);
	Target->SetTimePlayed(0).SetSamplesLoaded(0).SetTotalSamples((uint64_t)(LoopCount + 1)*(uint64_t)Stream->GetSampleLength());
	Target->SetFinishCount(0).SetFlag(Flags | LWSound::Dirty);
	if (m_CallBacks[CallbackCreate]) m_CallBacks[CallbackCreate](Target, this);
	m_ActiveSounds++;
	return Target;
}

LWAudioDriver::LWAudioDriver(void *UserData, LWAudioCallback FinishedCallback, LWAudioCallback CreateCallback, LWAudioCallback ReleaseCallback) : m_Flag(Dirty){
    m_CallBacks[CallbackCreate] = CreateCallback;
    m_CallBacks[CallbackFinished] = FinishedCallback;
    m_CallBacks[CallbackReleased] = ReleaseCallback;
    for (uint32_t i = 0; i < MaxSounds; i++) m_ActiveSoundPool[i] = m_SoundPool + i;
}

LWAudioDriver::~LWAudioDriver() {
    for (uint32_t i = 0; i < m_ActiveSounds; i++) ReleaseSound(m_ActiveSoundPool[i]);
    Update(0, nullptr);
}
