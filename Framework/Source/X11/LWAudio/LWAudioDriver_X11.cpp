#include "LWAudio/LWAudioDriver.h"
#ifndef LW_USEOPENAL
#include "LWAudio/LWAudioStream.h"
#include "LWCore/LWTimer.h"
#include "LWCore/LWAllocator.h"
#include "LWCore/LWLogger.h"
#include "LWPlatform/LWFileStream.h"
#include "LWPlatform/LWWindow.h"
#include <pulse/stream.h>
#include <algorithm>
#include <iostream>
#include <vorbis/codec.h>

void FreeCallbackFunc(void *) {
	return;
}

bool LWAudioDriver::PreUpdateSoundsPlatform(uint64_t ElapsedTime) {
	return true;
}

bool LWAudioDriver::PostUpdateSoundsPlatform(uint64_t ElapsedTime) {
	pa_mainloop_iterate(m_Context.m_MainLoop, false, nullptr);
	return true;
}

bool LWAudioDriver::ProcessSoundReleaseEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	pa_stream_disconnect(Context.m_Source);
	pa_stream_unref(Context.m_Source);
	return true;
}

bool LWAudioDriver::ProcessSoundPlayEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	pa_stream_cork(Context.m_Source, 0, nullptr, nullptr);
	return true;
}

bool LWAudioDriver::ProcessSoundStopEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	pa_stream_cork(Context.m_Source, 1, nullptr, nullptr);
	return true;
}

bool LWAudioDriver::ProcessSoundMuteEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	//Context.m_Source->SetVolume(0.0f, m_Context.m_OpSet);
	//m_Context.m_OpChanged = true;
	return true;
}

bool LWAudioDriver::ProcessSoundUnmuteEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	//Context.m_Source->SetVolume(Sound->GetVolume(), m_Context.m_OpSet);
	//m_Context.m_OpChanged = true;
	return true;
}

bool LWAudioDriver::ProcessSoundVolumeEventPlatform(LWSound *Sound, float Volume, uint64_t ElapsedTime) {
	const uint32_t MinVolume = PA_VOLUME_NORM / 4;
	const uint32_t MaxVolume = PA_VOLUME_NORM;
	LWSoundContext &Context = Sound->GetContext();
	Volume = Volume * m_MasterVolume*m_ChannelVolumes[Sound->GetChannel()];
	if (Sound->isMuted()) Volume = 0.0f;
	pa_cvolume v;
	v.channels = (uint8_t)Sound->GetAudioStream()->GetChannels();
	pa_volume_t pav = (pa_volume_t)MinVolume + (Volume*(float)(MaxVolume - MinVolume));
	for (uint8_t i = 0; i < v.channels; i++) v.values[i] = pav;

	return true;
}

bool LWAudioDriver::ProcessSoundPanEventPlatform(LWSound *Sound, float Pan, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	return true;
}

bool LWAudioDriver::ProcessSoundSpeedEventPlatform(LWSound *Sound, float Speed, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	//float Pitch = LWAudioDriver::DecodeEventPitch(EventData);
	//Context.m_Source->SetFrequencyRatio(Pitch, m_Context.m_OpSet);
	//m_Context.m_OpChanged = true;
	return true;
}

bool LWAudioDriver::ProcessSoundSeekEventPlatform(LWSound *Sound, uint32_t Seek, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	LWAudioStream *Stream = Sound->GetAudioStream();

	uint64_t TimePerSample = Stream->GetTimePerSample();
	uint64_t TimePlayed = TimePerSample * Seek;
	uint32_t FinishCount = Sound->GetFinishedCount();
	uint32_t SamplesPerSlice = Stream->GetSampleSliceSize(LWSOUND_RESERVEBUFFERSIZE);
	uint32_t TargetSlice = (uint32_t)(Seek / SamplesPerSlice);

	uint32_t LoadedSamples = Stream->GetSampleLength()*FinishCount;
	LoadedSamples += TargetSlice * SamplesPerSlice;
	TimePlayed += FinishCount * TimePerSample*Stream->GetSampleLength();
	Sound->SetSamplesLoaded(LoadedSamples).SetTimePlayed(TimePlayed);
	Context.m_SeekSamples = Seek - (TargetSlice*SamplesPerSlice);
	pa_stream_flush(Context.m_Source, nullptr, nullptr);
	if (Sound->isPlaying()) {
		Sound->SetFlag(Sound->GetFlag()&~LWSound::Playing);
		PushEvent(Sound, Event_Play);
	}

	/*
	pa_stream_flush(Context->m_Source, nullptr, nullptr);
	lCurrentTime = TimePerSample * Stream->GetSampleLength()*FinishCount + TimePerSample * S->GetSeekPosition();
	uint32_t TargetSlice = (uint32_t)((lCurrentTime%TotalTime) / TimePerSlice);
	SeekSamples = S->GetSeekPosition() - (TargetSlice*SamplesPerSlice);
	LoadedSamples = FinishCount * Stream->GetSampleLength() + SamplesPerSlice * TargetSlice;
	Flag |= (Flag&LWSound::Playing) ? LWSound::RequestPlay : 0;*/
	return true;
}

bool LWAudioDriver::ProcessVolumeEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	for (auto &&Iter : m_SoundList) {
		LWSoundContext &Context = Iter->GetContext();
		float Vol = Iter->GetVolume()*m_MasterVolume*m_ChannelVolumes[Iter->GetChannel()];
		if (Iter->isMuted()) Vol = 0.0f;
	}
	return true;
}

bool LWAudioDriver::ProcessPlayEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	return true;
}

bool LWAudioDriver::ProcessStopEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	return true;
}

bool LWAudioDriver::ProcessMuteEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	return true;
}

bool LWAudioDriver::ProcessUnmuteEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	return true;
}

bool LWAudioDriver::ProcessFocusPauseEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	return true;
}

bool LWAudioDriver::ProcessFocusMuteEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	return true;
}

bool LWAudioDriver::UpdateSoundPlatform(LWSound *Sound, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	LWAudioStream *Stream = Sound->GetAudioStream();

	pa_stream_state_t State = pa_stream_get_state(Context.m_Source);
	if (State != PA_STREAM_READY) return true; //should also check for failure

	uint64_t Time = Sound->GetTimePlayed();
	uint64_t TimePerSample = Stream->GetTimePerSample();

	uint64_t TotalTime = Stream->GetSampleLength()*TimePerSample;
	uint32_t SamplesPerSlice = Stream->GetSampleSliceSize(LWSOUND_RESERVEBUFFERSIZE);
	uint32_t FrameSize = Stream->GetChannels()*Stream->GetSampleSize();

	uint64_t TimePerSlice = SamplesPerSlice * TimePerSample;
	bool Playing = Sound->isPlaying();
	if (Playing) Time += ElapsedTime*Sound->GetSpeed();

	uint32_t TotalSlices = (Stream->GetSampleLength() + (SamplesPerSlice - 1)) / SamplesPerSlice;
	uint64_t LoadedSamples = Sound->GetSamplesLoaded();

	uint32_t PlayedSamples = (uint32_t)(Time / TimePerSample);
	uint32_t PlayedSlices = (PlayedSamples%Stream->GetSampleLength()) / SamplesPerSlice + TotalSlices * (PlayedSamples / Stream->GetSampleLength());
	uint64_t LoadedSlices = (LoadedSamples%Stream->GetSampleLength()) / SamplesPerSlice + TotalSlices * (LoadedSamples / Stream->GetSampleLength());
	uint64_t PreloadedSlices = LoadedSlices >= PlayedSlices ? (LoadedSlices - PlayedSlices) : 0;
	//LWSOUND_RESEVECNT-1 cause 1 or 2 samples may still need to be played, and are not cleared from the buffer yet, so we leave one block always open.
	while (PreloadedSlices < (LWSOUND_RESERVECNT - 1) && LoadedSamples != Sound->GetTotalSamples()) {
		uint32_t SliceIdx = (LoadedSlices%TotalSlices);
		uint32_t SliceSamples = (SliceIdx == (TotalSlices - 1)) ? (Stream->GetSampleLength() - (SamplesPerSlice*SliceIdx)) : SamplesPerSlice;
		uint32_t ReserveIdx = Context.m_ReserveIdx%LWSOUND_RESERVECNT;

		char *Reserve = Context.m_ReserveBuffer[ReserveIdx];
		char *Samples = Stream->DecodeSamples(Reserve, SliceIdx*SamplesPerSlice, SliceSamples);
		if (Reserve == Samples) Context.m_ReserveIdx++;
		uint32_t Seek = Context.m_SeekSamples*FrameSize;
		int32_t r = pa_stream_write(Context.m_Source, Samples + Seek, SliceSamples*FrameSize - Seek, FreeCallbackFunc, 0, PA_SEEK_RELATIVE);

		LoadedSamples += SliceSamples;
		PreloadedSlices++;
		LoadedSlices++;
		Context.m_SeekSamples = 0;
	}
	Sound->SetSamplesLoaded(LoadedSamples);
	Sound->SetTimePlayed(Time);
	return true;
}
/*
bool LWAudioDriver::Update(uint64_t lCurrentTime, LWWindow *Window) {

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
		uint32_t Flag = S->GetFlag();

		if (Flag&LWSound::Playing) lCurrentTime += TimeElapsed;


		uint32_t SliceCount = (Stream->GetSampleLength() + (SamplesPerSlice - 1)) / SamplesPerSlice;

		uint32_t SeekSamples = 0;
		uint32_t FinishCount = S->GetFinishedCount();
		uint64_t LoadedSamples = S->GetSamplesLoaded();


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
			
			char *Reserve = Context->m_ReserveBuffer + ReserveIdx*LWSOUND_RESERVEBUFFERSIZE;
			char *Samples = Stream->DecodeSamples(Reserve, SliceIdx*SamplesPerSlice, SliceSamples, true);
			if (Reserve == Samples) Context->m_ReserveIdx++;
			int32_t r = pa_stream_write(Context->m_Source, Samples + (SeekSamples*FrameSize), SliceSamples*FrameSize - (SeekSamples*FrameSize), FreeCallbackFunc, 0, PA_SEEK_RELATIVE);
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
			pa_stream_cork(Context->m_Source, 0, nullptr, nullptr);
			Flag |= LWSound::Playing;
		} else if (Flag&LWSound::RequestStop) {
			pa_stream_cork(Context->m_Source, 1, nullptr, nullptr);
			Flag &= ~LWSound::Playing;
		}
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
			if(ReleaseSoundF(S, i, lCurrentTime)) i--;
		} else if (UpdateSoundF(S, TimeElapsed)) Changed = true;
	}
	pa_mainloop_iterate(m_Context.m_MainLoop, false, nullptr);
	m_Flag &= ~Dirty;
	m_LastTime = lCurrentTime;
	return true;
}*/

bool LWAudioDriver::CreateSoundPlatform(LWSound *Sound) {
	LWSoundContext &Context = Sound->GetContext();
	LWAudioStream *Stream = Sound->GetAudioStream();
	uint32_t FrameSize = Stream->GetSampleSize()*Stream->GetChannels();
	pa_buffer_attr Attr = { 0xFFFFFFFF, 0xFFFFFFFF, 0, 0xFFFFFFFF, 0xFFFFFFFF };
	//Try to make a reasonable volume distribution.
	pa_sample_format FormatIDs[] = { (pa_sample_format)0, PA_SAMPLE_U8, PA_SAMPLE_S16LE, (pa_sample_format)0, PA_SAMPLE_FLOAT32LE };
	//std::cout << "Format: " << Stream->GetSampleType() << " rate: " << Stream->GetSampleRate() << " Size: " << Stream->GetSampleSize() << " Samples: " << Stream->GetSampleLength() << " Channels: " << Stream->GetChannels() << std::endl;
	pa_sample_spec specs = { FormatIDs[Stream->GetSampleSize()], Stream->GetSampleRate(), (uint8_t)Stream->GetChannels() };
	if (!(Context.m_Source = pa_stream_new(m_Context.m_Context, "LWStream", &specs, nullptr))) return false;
	pa_stream_connect_playback(Context.m_Source, nullptr, &Attr, (pa_stream_flags_t)(PA_STREAM_START_CORKED | PA_STREAM_AUTO_TIMING_UPDATE), nullptr, nullptr);
	Sound->SetVolume(1.0f);

	return true;
}

LWAudioDriver::LWAudioDriver(void *UserData, LWAllocator &Allocator, LWAudioCallback FinishedCallback, LWAudioCallback CreateCallback, LWAudioCallback ReleaseCallback) : m_Allocator(Allocator), m_UserData(UserData), m_MasterVolume(1.0f), m_Flag(MuteFocusAudio) {
	std::fill(m_ChannelVolumes, m_ChannelVolumes + ChannelCount, 1.0f);
	m_CallBacks[CallbackCreate] = CreateCallback;
	m_CallBacks[CallbackFinished] = FinishedCallback;
	m_CallBacks[CallbackReleased] = ReleaseCallback;
	if (!(m_Context.m_MainLoop = pa_mainloop_new())) m_Flag |= Error;
	m_Context.m_MainLoopAPI = pa_mainloop_get_api(m_Context.m_MainLoop);
	if (pa_signal_init(m_Context.m_MainLoopAPI) != 0) m_Flag |= Error;
	if (!(m_Context.m_Context = pa_context_new(m_Context.m_MainLoopAPI, "LWAudioDriver_X11Client"))) m_Flag |= Error;
	if (pa_context_connect(m_Context.m_Context, nullptr, (pa_context_flags)0, nullptr) < 0) m_Flag |= Error;
	pa_context_state_t State = PA_CONTEXT_CONNECTING;
	while (true) {
		pa_mainloop_iterate(m_Context.m_MainLoop, false, nullptr);
		State = pa_context_get_state(m_Context.m_Context);
		if (State == PA_CONTEXT_READY) break;
		else if (State == PA_CONTEXT_FAILED) {
			LWLogEvent("AudioDriver State failed.");
			m_Flag |= Error;
			break;
		}
	}

}

LWAudioDriver::~LWAudioDriver() {
	for (auto &Iter : m_SoundList) Iter->Release();
	Update(0, nullptr);
	
	
}
#endif