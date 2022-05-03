#include "LWPlatform/LWWindow.h"
#ifdef LW_USEOPENAL
#include "LWAudio/LWAudioDriver.h"
#include "LWAudio/LWAudioStream.h"
#include "LWCore/LWAllocator.h"
#include "LWCore/LWTimer.h"
#include "LWPlatform/LWFileStream.h"

#include <algorithm>
#include <iostream>
#include <vorbis/codec.h>
#include <thread>


bool LWAudioDriver::PreUpdateSoundsPlatform(uint64_t ElapsedTime) {
	if (!isUpdatingListenerPosition()) return true;
	LWVector3f Up = m_ListenerMatrix.m_Rows[1].xyz();
	LWVector3f Fwrd = m_ListenerMatrix.m_Rows[2].xyz();
	LWVector3f Pos = m_ListenerMatrix.m_Rows[3].xyz();
	float OrientVals[6] = { Fwrd.x, Fwrd.y, Fwrd.z, Up.x, Up.y, Up.z };
	alListenerfv(AL_POSITION, &Pos.x);
	alListenerfv(AL_ORIENTATION, OrientVals);
	return true;
}

bool LWAudioDriver::PostUpdateSoundsPlatform(uint64_t ElapsedTime) {
	return true;
}

bool LWAudioDriver::ProcessSoundCreatedEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	return true;
}

bool LWAudioDriver::ProcessSoundReleaseEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	alDeleteSources(1, &Context.m_Source);
	alDeleteBuffers(LWSOUND_RESERVECNT, Context.m_Buffers);
	return true;
}

bool LWAudioDriver::ProcessSoundPlayEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	alSourcePlay(Context.m_Source);
	return true;
}

bool LWAudioDriver::ProcessSoundStopEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	alSourcePause(Context.m_Source);
	return true;
}

bool LWAudioDriver::ProcessSoundMuteEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	alSourcef(Context.m_Source, AL_GAIN, 0.0f);
	return true;
}

bool LWAudioDriver::ProcessSoundUnmuteEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	alSourcef(Context.m_Source, AL_GAIN, Sound->GetVolume()*m_MasterVolume*m_ChannelVolumes[Sound->GetChannel()]);
	return true;
}

bool LWAudioDriver::ProcessSoundVolumeEventPlatform(LWSound *Sound, float Volume, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	Volume = Volume * m_MasterVolume*m_ChannelVolumes[Sound->GetChannel()];
	if (Sound->isMuted()) Volume = 0.0f;
	alSourcef(Context.m_Source, AL_GAIN, Volume);
	return true;
}

bool LWAudioDriver::ProcessSoundPanEventPlatform(LWSound *Sound, float Pan, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	Pan = Pan * 0.5f;
	LWVector3f Dir = LWVector3f(Pan, 0.0f, -sqrtf(1.0f - Pan * Pan));
	alSourcefv(Context.m_Source, AL_POSITION, &Dir.x);
	return true;
}

bool LWAudioDriver::ProcessSoundSpeedEventPlatform(LWSound *Sound, float Speed, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	alSourcef(Context.m_Source, AL_PITCH, Speed);
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
	alSourcePause(Context.m_Source);
	alSourceUnqueueBuffers(Context.m_Source, LWSOUND_RESERVECNT, Context.m_Buffers);
	if(Sound->isPlaying()){
		Sound->SetFlag(Sound->GetFlag()&~LWSound::Playing);
		PushEvent(Sound, Event_Play);
	}
	return true;
}

bool LWAudioDriver::ProcessVolumeEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	for (auto &&Iter : m_SoundList) {
		LWSoundContext &Context = Iter->GetContext();
		float Vol = Iter->GetVolume()*m_MasterVolume*m_ChannelVolumes[Iter->GetChannel()];
		if (Iter->isMuted()) Vol = 0.0f;
		alSourcef(Context.m_Source, AL_GAIN, Vol);
		alSourcef(Context.m_Source, AL_MAX_GAIN, Vol);
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
	//X3DAUDIO_DSP_SETTINGS dspSettings = { 0 };
	//float SndMatrix[16];
	//float channelThetas[2];
	LWSoundContext &Context = Sound->GetContext();
	LWAudioStream *Stream = Sound->GetAudioStream();
	uint32_t Flag = Sound->GetFlag();
	uint32_t SoundType = Sound->GetType();
	if (SoundType == LWSound::Sound3D) {
		if ((Flag&LWSound::PositionChanged) != 0 || isUpdatingListenerPosition()) {
			LWVector3f Pos = Sound->Get3DPosition();
			float CurveDistance = Sound->GetCurveDistance();
			float Volume = Sound->GetVolume();
			alSourcef(Context.m_Source, AL_REFERENCE_DISTANCE, CurveDistance);
			alSourcefv(Context.m_Source, AL_POSITION, &Pos.x);
			Sound->SetFlag(Flag & ~LWSound::PositionChanged);
		}
	}
	uint64_t Time = Sound->GetTimePlayed();
	uint64_t TimePerSample = Stream->GetTimePerSample();

	uint64_t TotalTime = Stream->GetSampleLength()*TimePerSample;
	uint32_t SamplesPerSlice = Stream->GetSampleSliceSize(LWSOUND_RESERVEBUFFERSIZE);
	uint32_t FrameSize = Stream->GetChannels()*Stream->GetSampleSize();

	uint64_t TimePerSlice = SamplesPerSlice * TimePerSample;
	bool Playing = Sound->isPlaying();
	if (Playing) Time += (uint64_t)(ElapsedTime*Sound->GetSpeed());
	uint32_t TotalSlices = (Stream->GetSampleLength() + (SamplesPerSlice - 1)) / SamplesPerSlice;
	uint64_t LoadedSamples = Sound->GetSamplesLoaded();

	uint32_t PlayedSamples = (uint32_t)(Time / TimePerSample);
	uint32_t PlayedSlices = (PlayedSamples%Stream->GetSampleLength()) / SamplesPerSlice + TotalSlices * (PlayedSamples / Stream->GetSampleLength());
	uint64_t LoadedSlices = (LoadedSamples%Stream->GetSampleLength()) / SamplesPerSlice + TotalSlices * (LoadedSamples / Stream->GetSampleLength());
	uint64_t PreloadedSlices = LoadedSlices >= PlayedSlices ? (LoadedSlices - PlayedSlices) : 0;
	uint32_t ALFormat = (Stream->GetChannels() == 1 ? (Stream->GetSampleSize() == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16) : (Stream->GetSampleSize() == 1 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16));

	//LWSOUND_RESEVECNT-1 cause 1 or 2 samples may still need to be played, and are not cleared from the buffer yet, so we leave one block always open.
	while (PreloadedSlices < (LWSOUND_RESERVECNT - 1) && LoadedSamples != Sound->GetTotalSamples()) {
		uint32_t SliceIdx = (LoadedSlices%TotalSlices);
		uint32_t SliceSamples = (SliceIdx == (TotalSlices - 1)) ? (Stream->GetSampleLength() - (SamplesPerSlice*SliceIdx)) : SamplesPerSlice;
		uint32_t ReserveIdx = Context.m_ReserveIdx%LWSOUND_RESERVECNT;
		uint32_t BufferIdx = Context.m_BufferIdx%LWSOUND_RESERVECNT;
		char *Reserve = Context.m_ReserveBuffer[ReserveIdx];
		char *Samples = Stream->DecodeSamples(Reserve, SliceIdx*SamplesPerSlice, SliceSamples);
		if (Reserve == Samples) Context.m_ReserveIdx++;
		uint32_t Seek = Context.m_SeekSamples*FrameSize;
		//Unqueue first:
		if (Context.m_BufferIdx >= LWSOUND_RESERVECNT) alSourceUnqueueBuffers(Context.m_Source, 1, &Context.m_Buffers[BufferIdx]);
		alBufferData(Context.m_Buffers[BufferIdx], ALFormat, Samples+Seek, SliceSamples*FrameSize-Seek, Stream->GetSampleRate());
		alSourceQueueBuffers(Context.m_Source, 1, &Context.m_Buffers[BufferIdx]);

		Context.m_BufferIdx++;
		LoadedSamples += SliceSamples;
		PreloadedSlices++;
		LoadedSlices++;
		Context.m_SeekSamples = 0;
	}
	if (Playing) {
		int32_t State = 0;
		alGetSourcei(Context.m_Source, AL_SOURCE_STATE, &State);
		if (State != AL_PLAYING) alSourcePlay(Context.m_Source);
	}
	Sound->SetSamplesLoaded(LoadedSamples);
	Sound->SetTimePlayed(Time);

	return true;
}

bool LWAudioDriver::CreateSoundPlatform(LWSound *Sound) {
	LWAudioStream *Stream = Sound->GetAudioStream();
	LWSoundContext &Context = Sound->GetContext();
	uint32_t SoundType = Sound->GetType();
	uint32_t FrameSize = Stream->GetSampleSize()*Stream->GetChannels();
	alGenSources(1, &Context.m_Source);
	alGenBuffers(LWSOUND_RESERVECNT, Context.m_Buffers);
	if (SoundType == LWSound::Sound3D) {
		alSourcef(Context.m_Source, AL_MAX_GAIN, 1.0f);
		alSourcef(Context.m_Source, AL_MIN_GAIN, 0.0f);
	} else {
		alSourcef(Context.m_Source, AL_ROLLOFF_FACTOR, 0.0f);
		alSourcei(Context.m_Source, AL_SOURCE_RELATIVE, 1);
		Sound->SetPan(0.0f).SetVolume(1.0f);
	}
	return true;
}


LWAudioDriver::LWAudioDriver(void *UserData, LWAllocator &Allocator, LWAudioCallback FinishedCallback, LWAudioCallback CreateCallback, LWAudioCallback ReleaseCallback) : m_Allocator(Allocator), m_UserData(UserData), m_MasterVolume(1.0f), m_Flag(MuteFocusAudio) {
	std::fill(m_ChannelVolumes, m_ChannelVolumes + ChannelCount, 1.0f);
	m_CallBacks[CallbackCreate] = CreateCallback;
	m_CallBacks[CallbackFinished] = FinishedCallback;
	m_CallBacks[CallbackReleased] = ReleaseCallback;

	if (!(m_Context.m_Device = alcOpenDevice(nullptr))) m_Flag |= Error;
	else if (!(m_Context.m_Context = alcCreateContext(m_Context.m_Device, nullptr))) m_Flag |= Error;
	alcMakeContextCurrent(m_Context.m_Context);
	alDistanceModel(AL_INVERSE_DISTANCE);
}

LWAudioDriver::~LWAudioDriver() {
	for (auto &&Iter : m_SoundList) Iter->Release();
	Update(0, nullptr);
	if (m_Context.m_Context) alcDestroyContext(m_Context.m_Context);
	if (m_Context.m_Device) alcCloseDevice(m_Context.m_Device);
}

#endif