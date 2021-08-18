#include "LWPlatform/LWWindow.h"
#ifndef LW_USEOPENAL
#include "LWAudio/LWAudioDriver.h"
#include "LWAudio/LWAudioStream.h"
#include "LWCore/LWAllocator.h"
#include "LWCore/LWTimer.h"
#include "LWPlatform/LWFileStream.h"

#include <algorithm>
#include <iostream>
#include <vorbis/codec.h>

class VCallbacks : public IXAudio2VoiceCallback {
public:

	void STDMETHODCALLTYPE OnBufferEnd(void *Context) {
		//std::cout << "Buffer finished: " << (uint32_t)(uintptr_t)Context << std::endl;
		return;
	}

	void STDMETHODCALLTYPE OnStreamEnd() {}
	void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() {}
	void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32) {}
	void STDMETHODCALLTYPE OnBufferStart(void*) {}
	void STDMETHODCALLTYPE OnLoopEnd(void*) {}
	void STDMETHODCALLTYPE OnVoiceError(void *, HRESULT) {}
};

bool LWAudioDriver::PreUpdateSoundsPlatform(uint64_t ElapsedTime) {
	if (!isUpdatingListenerPosition()) return true;
	LWVector3f Up = -m_ListenerMatrix.m_Rows[1].xyz();
	LWVector3f Fwrd = m_ListenerMatrix.m_Rows[2].xyz();
	LWVector3f Pos = m_ListenerMatrix.m_Rows[3].xyz();
	m_Context.m_Listener.OrientFront = X3DAUDIO_VECTOR(Fwrd.x, Fwrd.y, Fwrd.z);
	m_Context.m_Listener.OrientTop = X3DAUDIO_VECTOR(Up.x, Up.y, Up.z);
	m_Context.m_Listener.Position = X3DAUDIO_VECTOR(Pos.x, Pos.y, Pos.z);
	return true;
}

bool LWAudioDriver::PostUpdateSoundsPlatform(uint64_t ElapsedTime) {
	if (m_Context.m_OpChanged) m_Context.m_Device->CommitChanges(m_Context.m_OpSet);
	m_Context.m_OpChanged = false;
	m_Context.m_OpSet++;
	return true;
}

bool LWAudioDriver::ProcessSoundCreatedEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	return true;
}

bool LWAudioDriver::ProcessSoundReleaseEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime){
	LWSoundContext &Context = Sound->GetContext();
	Context.m_Source->DestroyVoice();
	return true;
}

bool LWAudioDriver::ProcessSoundPlayEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	Context.m_Source->Start(0, m_Context.m_OpSet);
	m_Context.m_OpChanged = true;
	return true;
}

bool LWAudioDriver::ProcessSoundStopEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	Context.m_Source->Stop(0, m_Context.m_OpSet);
	m_Context.m_OpChanged = true;
	return true;
}

bool LWAudioDriver::ProcessSoundMuteEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	Context.m_Source->SetVolume(0.0f, m_Context.m_OpSet);
	m_Context.m_OpChanged = true;
	return true;
}

bool LWAudioDriver::ProcessSoundUnmuteEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	Context.m_Source->SetVolume(Sound->GetVolume() * m_MasterVolume * m_ChannelVolumes[Sound->GetChannel()], m_Context.m_OpSet);
	m_Context.m_OpChanged = true;
	return true;
}

bool LWAudioDriver::ProcessSoundVolumeEventPlatform(LWSound *Sound, float Volume, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	Volume = Volume * m_MasterVolume*m_ChannelVolumes[Sound->GetChannel()];
	if (Sound->isMuted()) Volume = 0.0f;
	Context.m_Source->SetVolume(Volume, m_Context.m_OpSet);
	m_Context.m_OpChanged = true;
	return true;
}

bool LWAudioDriver::ProcessSoundPanEventPlatform(LWSound *Sound, float Pan, uint64_t ElapsedTime) {
	float OutputMatrix[16];
	LWSoundContext &Context = Sound->GetContext();
	if (Sound->GetType() == LWSound::Sound3D) return true;
	std::fill(OutputMatrix, OutputMatrix + 16, 0.0f);
	DWORD OutputMask = 0;
	m_Context.m_MasterVoice->GetChannelMask(&OutputMask);
	XAUDIO2_VOICE_DETAILS VDetails;
	XAUDIO2_VOICE_DETAILS MDetails;
	Context.m_Source->GetVoiceDetails(&VDetails);
	m_Context.m_MasterVoice->GetVoiceDetails(&MDetails);

	float Left = 0.5f - Pan*0.5f;
	float Right = 0.5f + Pan*0.5f;
	if (VDetails.InputChannels == 1) {
		if (OutputMask == SPEAKER_MONO) {
			OutputMatrix[0] = 1.0f;
		} else if (OutputMask == SPEAKER_STEREO || OutputMask == SPEAKER_2POINT1 || OutputMask == SPEAKER_SURROUND) {
			OutputMatrix[0] = Left;
			OutputMatrix[1] = Right;
		} else if (OutputMask == SPEAKER_QUAD) {
			OutputMatrix[0] = OutputMatrix[2] = Left;
			OutputMatrix[1] = OutputMatrix[3] = Right;
		} else if (OutputMask == SPEAKER_4POINT1) {
			OutputMatrix[0] = OutputMatrix[3] = Left;
			OutputMatrix[1] = OutputMatrix[4] = Right;
		} else if (OutputMask == SPEAKER_5POINT1 || OutputMask == SPEAKER_7POINT1 || OutputMask == SPEAKER_5POINT1_SURROUND) {
			OutputMatrix[0] = OutputMatrix[4] = Left;
			OutputMatrix[1] = OutputMatrix[5] = Right;
		} else if (OutputMask == SPEAKER_7POINT1_SURROUND) {
			OutputMatrix[0] = OutputMatrix[4] = OutputMatrix[6] = Left;
			OutputMatrix[1] = OutputMatrix[5] = OutputMatrix[7] = Right;
		}
	} else if (VDetails.InputChannels == 2) {
		if (OutputMask == SPEAKER_MONO) {
			OutputMatrix[0] = 1.0f;
		} else if (OutputMask == SPEAKER_STEREO || OutputMask == SPEAKER_2POINT1 || OutputMask == SPEAKER_SURROUND) {
			OutputMatrix[0] = Left;
			OutputMatrix[3] = Right;
		} else if (OutputMask == SPEAKER_QUAD) {
			OutputMatrix[0] = OutputMatrix[4] = Left;
			OutputMatrix[3] = OutputMatrix[7] = Right;
		} else if (OutputMask == SPEAKER_4POINT1) {
			OutputMatrix[0] = OutputMatrix[6] = Left;
			OutputMatrix[3] = OutputMatrix[9] = Right;
		} else if (OutputMask == SPEAKER_5POINT1 || OutputMask == SPEAKER_7POINT1 || OutputMask == SPEAKER_5POINT1_SURROUND) {
			OutputMatrix[0] = OutputMatrix[8] = Left;
			OutputMatrix[3] = OutputMatrix[11] = Right;
		} else if (OutputMask == SPEAKER_7POINT1_SURROUND) {
			OutputMatrix[0] = OutputMatrix[8] = OutputMatrix[12] = Left;
			OutputMatrix[3] = OutputMatrix[11] = OutputMatrix[15] = Right;
		}
	}
	Context.m_Source->SetOutputMatrix(m_Context.m_MasterVoice, VDetails.InputChannels, MDetails.InputChannels, OutputMatrix, m_Context.m_OpSet);
	m_Context.m_OpChanged = true;
	return true;
}

bool LWAudioDriver::ProcessSoundSpeedEventPlatform(LWSound *Sound, float Speed, uint64_t ElapsedTime) {
	LWSoundContext &Context = Sound->GetContext();
	Context.m_Source->SetFrequencyRatio(Speed, m_Context.m_OpSet);
	m_Context.m_OpChanged = true;
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
	LoadedSamples += TargetSlice*SamplesPerSlice;
	TimePlayed += FinishCount * TimePerSample*Stream->GetSampleLength();
	Sound->SetSamplesLoaded(LoadedSamples).SetTimePlayed(TimePlayed);
	Context.m_SeekSamples = Seek - (TargetSlice*SamplesPerSlice);
	Context.m_Source->Stop(0);
	Context.m_Source->FlushSourceBuffers();
	m_Context.m_OpChanged = true;
	if (Sound->isPlaying()) {
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
		Context.m_Source->SetVolume(Vol, m_Context.m_OpSet);
		m_Context.m_OpChanged = true;
	}
	return true;
}

bool LWAudioDriver::ProcessPlayEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	return true;
}

bool LWAudioDriver::ProcessStopEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	return true;
}

bool LWAudioDriver::ProcessMuteEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime){
	return true;
}

bool LWAudioDriver::ProcessUnmuteEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime){
	return true;
}

bool LWAudioDriver::ProcessFocusPauseEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime){
	return true;
}

bool LWAudioDriver::ProcessFocusMuteEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime){
	return true;
}

bool LWAudioDriver::UpdateSoundPlatform(LWSound *Sound, uint64_t ElapsedTime) {
	X3DAUDIO_DSP_SETTINGS dspSettings = { 0 };
	float SndMatrix[16];
	float channelThetas[2];
	LWSoundContext &Context = Sound->GetContext();
	LWAudioStream *Stream = Sound->GetAudioStream();
	uint32_t Flag = Sound->GetFlag();
	uint32_t SoundType = Sound->GetType();
	if (SoundType == LWSound::Sound3D) {
		if((Flag&LWSound::PositionChanged)!=0 || isUpdatingListenerPosition()){
			Sound->SetFlag(Flag&~LWSound::PositionChanged);
			channelThetas[0] = 0.0f;
			channelThetas[1] = LW_PI;
			XAUDIO2_VOICE_DETAILS VDetails;
			XAUDIO2_VOICE_DETAILS DDetails;
			Context.m_Source->GetVoiceDetails(&VDetails);
			m_Context.m_MasterVoice->GetVoiceDetails(&DDetails);
			LWVector3f Pos = Sound->Get3DPosition();
			Context.m_Emitter.Position = X3DAUDIO_VECTOR(Pos.x, Pos.y, Pos.z);
			Context.m_Emitter.pChannelAzimuths = channelThetas;
			Context.m_Emitter.CurveDistanceScaler = Sound->GetCurveDistance();
			dspSettings.SrcChannelCount = VDetails.InputChannels;
			dspSettings.DstChannelCount = DDetails.InputChannels;
			dspSettings.pMatrixCoefficients = SndMatrix;
			
			X3DAudioCalculate(m_Context.m_3DHandle, &m_Context.m_Listener, &Context.m_Emitter, X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_LPF_DIRECT, &dspSettings);
			if (VDetails.InputChannels == 2) { //X3DAudioCalculate seems to place data in the wrong order for stereo sources).
				for (uint32_t i = 1; i < DDetails.InputChannels*2; i += 4) {
					std::swap(SndMatrix[i], SndMatrix[i + 1]);
				}
			}

			HRESULT Res = Context.m_Source->SetOutputMatrix(nullptr, VDetails.InputChannels, DDetails.InputChannels, SndMatrix, m_Context.m_OpSet);
			XAUDIO2_FILTER_PARAMETERS Filter = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI/6.0f * dspSettings.LPFDirectCoefficient), 1.0f };
			Res = Context.m_Source->SetFilterParameters(&Filter, m_Context.m_OpSet);
			m_Context.m_OpChanged = true;
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
	//LWSOUND_RESEVECNT-1 cause 1 or 2 samples may still need to be played, and are not cleared from the buffer yet, so we leave one block always open.
	while (PreloadedSlices < (LWSOUND_RESERVECNT - 1) && LoadedSamples != Sound->GetTotalSamples()) {
		uint32_t SliceIdx = (LoadedSlices%TotalSlices);
		uint32_t SliceSamples = (SliceIdx == (TotalSlices - 1)) ? (Stream->GetSampleLength() - (SamplesPerSlice*SliceIdx)) : SamplesPerSlice;
		uint32_t ReserveIdx = Context.m_ReserveIdx%LWSOUND_RESERVECNT;

		char *Reserve = Context.m_ReserveBuffer[ReserveIdx];
		char *Samples = Stream->DecodeSamples(Reserve, SliceIdx*SamplesPerSlice, SliceSamples);
		if (Reserve == Samples) Context.m_ReserveIdx++;
		XAUDIO2_BUFFER XBuf = { 0, SliceSamples*FrameSize, (BYTE*)Samples, Context.m_SeekSamples, SliceSamples - Context.m_SeekSamples, 0, 0, 0, (void*)(uintptr_t)ReserveIdx };
		HRESULT Res = Context.m_Source->SubmitSourceBuffer(&XBuf, nullptr);
		if (FAILED(Res)) {
			fmt::print("Error 'SubmitSourceBuffer': {:#x}\n", Res);
			return false;
		}
		LoadedSamples += SliceSamples;
		PreloadedSlices++;
		LoadedSlices++;
		Context.m_SeekSamples = 0;
	}
	Sound->SetSamplesLoaded(LoadedSamples);
	Sound->SetTimePlayed(Time);
	
	return true;
}

bool LWAudioDriver::CreateSoundPlatform(LWSound *Sound) {
	LWAudioStream *Stream = Sound->GetAudioStream();
	LWSoundContext &Context = Sound->GetContext();
	uint32_t SoundType = Sound->GetType();
	float VolumeMatrix[16];

	//LinearPCM, LinearIEEEFloat
	uint16_t FormatIDs[] = { WAVE_FORMAT_PCM, WAVE_FORMAT_IEEE_FLOAT };
	WAVEFORMATEX WaveFmt;
	uint32_t FrameSize = Stream->GetSampleSize()*Stream->GetChannels();

	memset(&WaveFmt, 0, sizeof(WAVEFORMATEX));
	WaveFmt.wFormatTag = FormatIDs[Stream->GetSampleType()];
	WaveFmt.nChannels = (uint16_t)Stream->GetChannels();
	WaveFmt.nSamplesPerSec = Stream->GetSampleRate();
	WaveFmt.nAvgBytesPerSec = Stream->GetSampleRate()*FrameSize;
	WaveFmt.nBlockAlign = FrameSize;
	WaveFmt.wBitsPerSample = Stream->GetSampleSize() * 8;
	WaveFmt.cbSize = 0;
	if (FAILED(m_Context.m_Device->CreateSourceVoice(&Context.m_Source, &WaveFmt, XAUDIO2_VOICE_USEFILTER, 10.0f))) return false;
	if (SoundType == LWSound::Sound3D) {
		XAUDIO2_VOICE_DETAILS MDetails;
		m_Context.m_MasterVoice->GetVoiceDetails(&MDetails);
		std::fill(VolumeMatrix, VolumeMatrix + 16, 0.0f);
		Context.m_Emitter = { 0 };
		Context.m_Emitter.ChannelCount = Stream->GetChannels();
		Context.m_Emitter.InnerRadius = 2.0f;
		Context.m_Emitter.InnerRadiusAngle = LW_PI_4;

		Context.m_Emitter.ChannelRadius = 5.0f;
		Context.m_Emitter.CurveDistanceScaler = 14.0f;
		Context.m_Emitter.DopplerScaler = 1.0f;
		Context.m_Emitter.pVolumeCurve = nullptr;
		Context.m_Emitter.OrientFront = X3DAUDIO_VECTOR(0.0f, 0.0f, 1.0f);
		Context.m_Emitter.OrientTop = X3DAUDIO_VECTOR(0.0f, 1.0f, 0.0f);
		//Silence 3d audio until it's properly positioned.
		Context.m_Source->SetOutputMatrix(nullptr, Context.m_Emitter.ChannelCount, MDetails.InputChannels, VolumeMatrix);
		Sound->Set3DPosition(LWVector3f());
	} else Sound->SetPan(0.0f);


	return true;
}

LWAudioDriver::LWAudioDriver(void *UserData, LWAllocator &Allocator, LWAudioCallback FinishedCallback, LWAudioCallback CreateCallback, LWAudioCallback ReleaseCallback) : m_Allocator(Allocator), m_UserData(UserData), m_MasterVolume(1.0f), m_Flag(MuteFocusAudio) {
	std::fill(m_ChannelVolumes, m_ChannelVolumes + ChannelCount, 1.0f);
	m_CallBacks[CallbackCreate] = CreateCallback;
	m_CallBacks[CallbackFinished] = FinishedCallback;
	m_CallBacks[CallbackReleased] = ReleaseCallback;
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(XAudio2Create(&m_Context.m_Device, 0, XAUDIO2_DEFAULT_PROCESSOR))) m_Flag |= Error;
	else if (FAILED(m_Context.m_Device->CreateMasteringVoice(&m_Context.m_MasterVoice))) m_Flag |= Error;

	DWORD ChannelMask = 0;
	m_Context.m_MasterVoice->GetChannelMask(&ChannelMask);
	if (FAILED(X3DAudioInitialize(ChannelMask, X3DAUDIO_SPEED_OF_SOUND, m_Context.m_3DHandle))) m_Flag |= Error;
	m_Context.m_Listener = { X3DAUDIO_VECTOR(0.0f, 0.0f, 1.0f), X3DAUDIO_VECTOR(0.0f, 1.0f, 0.0f), X3DAUDIO_VECTOR(0.0f, 0.0f, 0.0f), X3DAUDIO_VECTOR(0.0f, 0.0f, 0.0f), nullptr };
}

LWAudioDriver::~LWAudioDriver() {
	for (auto &&Iter : m_SoundList) Iter->Release();
	Update(0, nullptr);
	m_Context.m_Device->Release();
	CoUninitialize();
}
#endif