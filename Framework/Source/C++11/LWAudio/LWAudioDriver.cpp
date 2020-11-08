#include "LWAudio/LWAudioDriver.h"
#include "LWAudio/LWAudioStream.h"
#include "LWPlatform/LWFileStream.h"
#include "LWCore/LWAllocator.h"
#include "LWCore/LWByteBuffer.h"
#include "LWCore/LWTimer.h"
#include <algorithm>
#include <iostream>
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

float LWAudioDriver::DecodeEventVolume(uint32_t Event) {
	uint32_t D = Event & Event_Data_Bits;
	return LWDECODE_FLOAT(D, 1.0f, Event_Data_Bits);
}

float LWAudioDriver::DecodeEventChannelVolume(uint32_t Event, uint32_t &ChannelID) {
	uint32_t D = Event & Event_Data_Channel_Volume_Bits;
	ChannelID = (Event&Event_Data_Channel_Bits) >> Event_Data_Channel_Bitoffset;
	return LWDECODE_FLOAT(D, 1.0f, Event_Data_Channel_Volume_Bits);
}

float LWAudioDriver::DecodeEventPan(uint32_t Event) {
	uint32_t D = Event & Event_Data_Bits;
	return LWDECODE_FLOAT(D, 2.0f, Event_Data_Bits) - 1.0f;
}

float LWAudioDriver::DecodeEventSpeed(uint32_t Event) {
	uint32_t D = Event & Event_Data_Bits;
	return LWDECODE_FLOAT(D, 10.0f, Event_Data_Bits);
}

uint32_t LWAudioDriver::EncodeEventVolume(float Volume) {
	return LWENCODE_FLOAT(Volume, 1.0f, Event_Data_Bits);
}

uint32_t LWAudioDriver::EncodeEventPan(float Pan) {
	return LWENCODE_FLOAT(Pan + 1.0f, 2.0f, Event_Data_Bits);
}

uint32_t LWAudioDriver::EncodeEventChannelVolume(float Volume, uint32_t ChannelID) {
	return LWENCODE_FLOAT(Volume, 1.0f, Event_Data_Channel_Volume_Bits) | (ChannelID << Event_Data_Channel_Bitoffset);
}

uint32_t LWAudioDriver::EncodeEventSpeed(float Pitch) {
	return LWENCODE_FLOAT(Pitch, 10.0f, Event_Data_Bits);
}

bool LWAudioDriver::Update(uint64_t lCurrentTime, LWWindow *Window) {
	const uint64_t PositionUpdateFreq = LWTimer::ToHighResolution((uint64_t)UpdatePositionFrequency);
	if (!m_LastTime) m_LastTime = m_LastPositionUpdateTime = lCurrentTime;
	if (Window) {
		if (Window->isFocused()) {
			if ((m_Flag&(PauseFocusAudio | PausedSystem)) == (PauseFocusAudio | PausedSystem)) PushEvent(nullptr, Event_FocusPause);
			if ((m_Flag&(MuteFocusAudio | MutedSystem)) == (MuteFocusAudio | MutedSystem)) PushEvent(nullptr, Event_FocusMute);
		} else {
			if ((m_Flag&(PauseFocusAudio | PausedSystem)) == PauseFocusAudio) PushEvent(nullptr, Event_FocusPause);
			if ((m_Flag&(MuteFocusAudio | MutedSystem)) == MuteFocusAudio) PushEvent(nullptr, Event_FocusMute);
		}
	}

	if (lCurrentTime - m_LastPositionUpdateTime >= PositionUpdateFreq) {
		m_Flag = (m_Flag&~ListernerPositionChanged) | UpdateSoundPositions | ((m_Flag&ListernerPositionChanged)!=0 ? UpdateListenerPosition : 0);
		m_LastPositionUpdateTime += PositionUpdateFreq;
	}
	uint64_t Elapsed = lCurrentTime - m_LastTime;
	if (!PreUpdateSoundsPlatform(Elapsed)) return false;

	for(auto &&Iter : m_SoundList){
		if (!UpdateSoundPlatform(Iter, Elapsed)) return false;
		uint32_t FinishCnt = Iter->GetFinishedCount() + 1;
		if (Iter->GetTimePlayed() >= Iter->GetAudioStream()->GetTotalTime()*FinishCnt) {
			Iter->SetFinishCount(FinishCnt);
			if (m_CallBacks[CallbackFinished]) m_CallBacks[CallbackFinished](Iter, this);
		}
	}

	uint32_t WritePos = m_EventWritePosition;
	while(m_EventReadPosition!=WritePos){
		LWAudioEvent &E = m_EventTable[m_EventReadPosition%EventTableSize];
		uint32_t Type = E.m_Event&Event_ID_Bits;
		uint32_t Data = E.m_Event&Event_Data_Bits;
		bool Result = true;
		if (E.m_Source) {
			if (Type == Event_Play) Result = ProcessSoundPlayEvent(E.m_Source, Type, Data, Elapsed);
			else if (Type == Event_Stop) Result = ProcessSoundStopEvent(E.m_Source, Type, Data, Elapsed);
			else if (Type == Event_Release) Result = ProcessSoundReleaseEvent(E.m_Source, Type, Data, Elapsed);
			else if (Type == Event_Volume) Result = ProcessSoundVolumeEvent(E.m_Source, Type, Data, Elapsed);
			else if (Type == Event_Pan) Result = ProcessSoundPanEvent(E.m_Source, Type, Data, Elapsed);
			else if (Type == Event_Speed) Result = ProcessSoundSpeedEvent(E.m_Source, Type, Data, Elapsed);
			else if (Type == Event_Seek) Result = ProcessSoundSeekEvent(E.m_Source, Type, Data, Elapsed);
			else if (Type == Event_Mute) Result = ProcessSoundMuteEvent(E.m_Source, Type, Data, Elapsed);
			else if (Type == Event_Unmute) Result = ProcessSoundUnmuteEvent(E.m_Source, Type, Data, Elapsed);
			else if (Type == Event_Created) Result = ProcessSoundCreatedEvent(E.m_Source, Type, Data, Elapsed);
			else fmt::print("Received unknown event for sound: {}\n", Type);
		} else {
			if (Type == Event_Volume) Result = ProcessVolumeEvent(Type, Data, Elapsed);
			else if (Type == Event_FocusMute) Result = ProcessFocusMuteEvent(Type, Data, Elapsed);
			else if (Type == Event_FocusPause) Result = ProcessFocusPauseEventPlatform(Type, Data, Elapsed);
			else if (Type == Event_Play) Result = ProcessPlayEvent(Type, Data, Elapsed);
			else if (Type == Event_Stop) Result = ProcessStopEvent(Type, Data, Elapsed);
			else if (Type == Event_Mute) Result = ProcessMuteEvent(Type, Data, Elapsed);
			else if (Type == Event_Unmute) Result = ProcessUnmuteEvent(Type, Data, Elapsed);
			else if (Type == Event_ChannelVolume) Result = ProcessVolumeEvent(Type, Data, Elapsed);
			else if (Type == Event_ListenerChanged) m_Flag |= ListernerPositionChanged;
			else fmt::print("Received unknown event for audio driver: '{}'\n", Type);
		}
		if (!Result) return false;
		m_EventReadPosition++;
	}

	if (!PostUpdateSoundsPlatform(Elapsed)) return false;
	m_Flag &= ~(UpdateSoundPositions|UpdateListenerPosition);
	m_LastTime = lCurrentTime;
	return true;
}

bool LWAudioDriver::ProcessSoundCreatedEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	if (!ProcessSoundCreatedEventPlatform(Sound, EventID, EventData, ElapsedTime)) return false;
	m_SoundList.push_back(Sound);
	if (m_CallBacks[CallbackCreate]) m_CallBacks[CallbackCreate](Sound, this);
	return true;
}

bool LWAudioDriver::ProcessSoundPlayEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	bool Playing = Sound->isPlaying();
	uint32_t Flag = Sound->GetFlag();
	if (EventData) Flag &= ~LWSound::SystemPaused;
	else Flag |= LWSound::Playing;
	Sound->SetFlag(Flag);
	if (Playing || !Sound->isPlaying()) return true;
	if (!ProcessSoundPlayEventPlatform(Sound, EventID, EventData, ElapsedTime)) return false;
	return true;
}

bool LWAudioDriver::ProcessSoundStopEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	bool Playing = Sound->isPlaying();
	uint32_t Flag = Sound->GetFlag();
	if (EventData) Flag |= LWSound::SystemPaused;
	else Flag &= ~LWSound::Playing;
	Sound->SetFlag(Flag);
	if (!Playing || Sound->isPlaying()) return true;
	if (!ProcessSoundStopEventPlatform(Sound, EventID, EventData, ElapsedTime)) return false;
	return true;
}

bool LWAudioDriver::ProcessSoundMuteEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	bool Muted = Sound->isMuted();
	uint32_t Flag = Sound->GetFlag();
	if (EventData) Flag |= LWSound::SystemMuted;
	else Flag |= LWSound::Muted;
	Sound->SetFlag(Flag);
	if (Muted || !Sound->isMuted()) return true;
	if (!ProcessSoundMuteEventPlatform(Sound, EventID, EventData, ElapsedTime)) return false;
	return true;
}

bool LWAudioDriver::ProcessSoundUnmuteEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	bool Muted = Sound->isMuted();
	uint32_t Flag = Sound->GetFlag();
	if (EventData) Flag &= ~LWSound::SystemMuted;
	else Flag &= ~LWSound::Muted;
	Sound->SetFlag(Flag);
	if (!Muted || Sound->isMuted()) return true;
	if (!ProcessSoundUnmuteEventPlatform(Sound, EventID, EventData, ElapsedTime)) return false;
	return true;
}

bool LWAudioDriver::ProcessSoundReleaseEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	auto Iter = m_SoundList.begin();
	for (; Iter != m_SoundList.end(); ++Iter) {
		if (*Iter == Sound) break;
	}
	if (Iter == m_SoundList.end()) return false;
	if (m_CallBacks[CallbackReleased]) m_CallBacks[CallbackReleased](*Iter, this);
	if (!ProcessSoundReleaseEventPlatform(*Iter, EventID, EventData, ElapsedTime)) return false;
	LWAllocator::Destroy(*Iter);
	*Iter = std::move(m_SoundList.back());
	m_SoundList.pop_back();
	return true;
}

bool LWAudioDriver::ProcessSoundVolumeEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	float Vol = DecodeEventVolume(EventData);
	if (!ProcessSoundVolumeEventPlatform(Sound, Vol, ElapsedTime)) return false;
	Sound->SetVolume(Vol, false);
	return true;
}

bool LWAudioDriver::ProcessSoundPanEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	float Pan = DecodeEventPan(EventData);
	if (!ProcessSoundPanEventPlatform(Sound, Pan, ElapsedTime)) return false;
	Sound->SetPan(Pan, false);
	return true;
}

bool LWAudioDriver::ProcessSoundSpeedEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	float Speed = DecodeEventSpeed(EventData);
	if (!ProcessSoundSpeedEventPlatform(Sound, Speed, ElapsedTime)) return false;
	Sound->SetSpeed(Speed, false);
	return true;
}

bool LWAudioDriver::ProcessSoundSeekEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	if (!ProcessSoundSeekEventPlatform(Sound, EventData, ElapsedTime)) return false;
	return true;
}

bool LWAudioDriver::ProcessPlayEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	if ((m_Flag&Paused) == 0) return true;
	m_Flag &= ~Paused;
	if ((m_Flag&PausedSystem) != 0) return true;
	if (!ProcessPlayEventPlatform(EventID, EventData, ElapsedTime)) return false;
	for (auto &&Iter : m_SoundList) {
		if (!ProcessSoundPlayEvent(Iter, Event_Play, 1, ElapsedTime)) return false;
	}
	return true;
}

bool LWAudioDriver::ProcessStopEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	if ((m_Flag&Paused) != 0) return true;
	m_Flag |= Paused;
	if ((m_Flag&PausedSystem) != 0) return true;
	if (!ProcessStopEventPlatform(EventID, EventData, ElapsedTime)) return false;
	for(auto &&Iter : m_SoundList){
		if (!ProcessSoundStopEvent(Iter, Event_Stop, 1, ElapsedTime)) return false;
	}
	return true;
}

bool LWAudioDriver::ProcessMuteEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	if ((m_Flag&Muted) != 0) return true;
	m_Flag |= Muted;
	if ((m_Flag&MutedSystem) != 0) return true;
	if (!ProcessMuteEventPlatform(EventID, EventData, ElapsedTime)) return false;
	for (auto &&Iter : m_SoundList) {
		if (!ProcessSoundMuteEvent(Iter, Event_Mute, 1, ElapsedTime)) return false;
	}
	return true;
}

bool LWAudioDriver::ProcessUnmuteEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	if ((m_Flag&Muted) == 0) return true;
	m_Flag &= ~Muted;
	if ((m_Flag&MutedSystem) != 0) return true;
	if (!ProcessUnmuteEventPlatform(EventID, EventData, ElapsedTime)) return false;
	for (auto &&Iter : m_SoundList) {
		if (!ProcessSoundUnmuteEvent(Iter, Event_Unmute, 1, ElapsedTime)) return false;
	}
	return true;
}


bool LWAudioDriver::ProcessVolumeEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	if (EventID == Event_ChannelVolume) {
		uint32_t ChannelID = 0;
		float Volume = DecodeEventChannelVolume(EventData, ChannelID);
		m_ChannelVolumes[ChannelID] = Volume;
		if (!ProcessVolumeEventPlatform(EventID, EventData, ElapsedTime)) return false;
	} else if (EventID == Event_Volume) {
		m_MasterVolume = DecodeEventVolume(EventData);
		if (!ProcessVolumeEventPlatform(EventID, EventData, ElapsedTime)) return false;
	} else return false;
	return true;
}

bool LWAudioDriver::ProcessFocusPauseEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	bool SystemWasPaused = (m_Flag&PausedSystem) != 0;
	if (!ProcessFocusPauseEventPlatform(EventID, EventData, ElapsedTime)) return false;
	m_Flag ^= PausedSystem;
	if ((m_Flag&Paused) == 0) {
		if (SystemWasPaused) {
			for (auto &&Iter : m_SoundList) {
				if (!ProcessSoundPlayEvent(Iter, Event_Play, 1, ElapsedTime)) return false;
			}
		} else {
			for (auto &&Iter : m_SoundList) {
				if (!ProcessSoundStopEvent(Iter, Event_Stop, 1, ElapsedTime)) return false;
			}
		}
	}
	return true;
}

bool LWAudioDriver::ProcessFocusMuteEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime) {
	bool SystemWasMuted = (m_Flag&MutedSystem) != 0;
	if (!ProcessFocusMuteEventPlatform(EventID, EventData, ElapsedTime)) return false;
	m_Flag ^= MutedSystem;
	if ((m_Flag&Muted) == 0) {
		if (SystemWasMuted) {
			for (auto &&Iter : m_SoundList) {
				if (!ProcessSoundUnmuteEvent(Iter, Event_Unmute, 1, ElapsedTime)) return false;
			}
		} else {
			for (auto &&Iter : m_SoundList) {
				if (!ProcessSoundMuteEvent(Iter, Event_Mute, 1, ElapsedTime)) return false;
			}
		}
	}
	return true;
}

LWSound *LWAudioDriver::CreateSound2D(LWAudioStream *Stream, void *UserData, uint32_t Channel, bool Playing, uint32_t LoopCount, float Volume, float Speed, float Pan) {
	LWSound *Target = m_Allocator.Create<LWSound>(this, Stream, LWSound::Sound2D, Channel, LoopCount, UserData);
	if (!CreateSoundPlatform(Target)) {
		LWAllocator::Destroy(Target);
		return nullptr;
	}
	if (!PushEvent(Target, Event_Created)) {
		ProcessSoundReleaseEventPlatform(Target, Event_Release, 1, m_LastTime);
		LWAllocator::Destroy(Target);
		return nullptr;
	}
	Target->SetVolume(Volume);
	if (fabs(Pan - Target->GetPan()) >= LWEpsilon) Target->SetPan(Pan);
	if (fabs(Speed - Target->GetSpeed()) >= LWEpsilon) Target->SetSpeed(Speed);

	if (m_Flag&(Muted | MutedSystem)) {
		ProcessSoundMuteEvent(Target, Event_Mute, 1, 0);
	}
	if (m_Flag&(Paused | PausedSystem)) {
		ProcessSoundStopEvent(Target, Event_Stop, 1, 0);
	}
	if (Playing) Target->Resume();
	return Target;
}

LWSound *LWAudioDriver::CreateSound3D(LWAudioStream *Stream, void *UserData, uint32_t Channel, bool Playing, const LWVector3f &Position, uint32_t LoopCount, float Volume, float DistanceCurve, float Speed) {
	LWSound *Target = m_Allocator.Create<LWSound>(this, Stream, LWSound::Sound3D, Channel, LoopCount, UserData);
	if (!CreateSoundPlatform(Target)) {
		LWAllocator::Destroy(Target);
		return nullptr;
	}
	if (!PushEvent(Target, Event_Created)) {
		ProcessSoundReleaseEventPlatform(Target, Event_Release, 1, m_LastTime);
		LWAllocator::Destroy(Target);
		return nullptr;
	}
	Target->Set3DPosition(Position).SetCurveDistance(DistanceCurve);
	Target->SetVolume(Volume);
	if (fabs(Speed - Target->GetSpeed()) >= LWEpsilon) Target->SetSpeed(Speed);
	if (m_Flag&(Muted | MutedSystem)) {
		ProcessSoundMuteEvent(Target, Event_Mute, 1, 0);
	}
	if (m_Flag&(Paused | PausedSystem)) {
		ProcessSoundStopEvent(Target, Event_Stop, 1, 0);
	}
	if (Playing) Target->Resume();
	return Target;
}

LWAudioDriver &LWAudioDriver::SetListener(const LWVector3f &Pos, const LWVector3f &Fwrd, const LWVector3f &Up){
	LWVector3f R = Fwrd.Cross(Up).Normalize();
	LWVector3f U = R.Cross(Fwrd);
	m_ListenerMatrix = LWMatrix4f({ R, 0.0f }, { U, 0.0f }, { -Fwrd, 0.0f }, { Pos, 1.0f });
	PushEvent(nullptr, Event_ListenerChanged);
	return *this;
}

LWAudioDriver &LWAudioDriver::SetMasterVolume(float Volume) {
	PushEvent(nullptr, EncodeEventVolume(Volume) | Event_Volume);
	return *this;
}

LWAudioDriver &LWAudioDriver::SetChannelVolume(uint32_t Channel, float Volume) {
	PushEvent(nullptr, EncodeEventChannelVolume(Volume, Channel) | Event_ChannelVolume);
	return *this;
}

LWAudioDriver &LWAudioDriver::SetAudioFocusBehavior(bool MuteAudioOnFocusLoss, bool PauseAudioOnFocusLoss){
	m_Flag = (m_Flag&~(PauseFocusAudio|MuteFocusAudio)) | (MuteAudioOnFocusLoss ? MuteFocusAudio : 0) | (PauseAudioOnFocusLoss ? PauseFocusAudio : 0);
	return *this;
}

bool LWAudioDriver::PushEvent(LWSound *Source, uint32_t Event) {
	if (m_EventWritePosition - m_EventReadPosition >= EventTableSize) return false;
	m_EventTable[m_EventWritePosition%EventTableSize] = { Source, Event };
	m_EventWritePosition++;
	return true;
}

LWAudioDriver &LWAudioDriver::PauseAll(void) {
	PushEvent(nullptr, Event_Stop);
	return *this;
}

LWAudioDriver &LWAudioDriver::ResumeAll(void) {
	PushEvent(nullptr, Event_Play);
	return *this;
}

LWAudioDriver &LWAudioDriver::MuteAll(void) {
	PushEvent(nullptr, Event_Mute);
	return *this;
}

LWAudioDriver &LWAudioDriver::UnmuteAll(void) {
	PushEvent(nullptr, Event_Unmute);
	return *this;
}

bool LWAudioDriver::isPaused(void) const{
	return (m_Flag&(Paused | PausedSystem)) != 0;
}

bool LWAudioDriver::isMuted(void) const {
	return (m_Flag&(Muted|MutedSystem)) != 0;
}

uint32_t LWAudioDriver::GetError(void) const {
	return (m_Flag&Error);
}

LWAudioDriver &LWAudioDriver::SetUserData(void *UserData) {
	m_UserData = UserData;
	return *this;
}

uint32_t LWAudioDriver::GetActiveSounds(void) const{
	return (uint32_t)m_SoundList.size();
}

LWSound *LWAudioDriver::GetSound(uint32_t i) {
	return m_SoundList[i];
}

uint32_t LWAudioDriver::GetFlag(void) const {
	return m_Flag;
}

LWMatrix4f LWAudioDriver::GetListenerMatrix(void) const {
	return m_ListenerMatrix;
}

float LWAudioDriver::GetMasterVolume(void) const {
	return m_MasterVolume;
}

float LWAudioDriver::GetChannelVolume(uint32_t ChannelID) const {
	return m_ChannelVolumes[ChannelID];
}

LWAudioDriverContext *LWAudioDriver::GetContext(void) {
	return &m_Context;
}

void *LWAudioDriver::GetUserData(void){
	return m_UserData;
}

bool LWAudioDriver::isUpdatingSoundPosition(void) const {
	return (m_Flag&UpdateSoundPositions) != 0;
}

bool LWAudioDriver::isUpdatingListenerPosition(void) const {
	return (m_Flag&UpdateListenerPosition) != 0;
}