#include <LWAudio/LWSound.h>
#include <LWAudio/LWAudioStream.h>
#include <LWAudio/LWAudioDriver.h>
#include <LWCore/LWTimer.h>
#include <LWCore/LWMath.h>
#include <cstring>

uint32_t LWSound::CalculatePosition(float Time, uint32_t SampleRate) {
	return (uint32_t)(SampleRate*Time);
}

float LWSound::CalculateTime(uint32_t SamplePos, uint32_t SampleRate) {
	return (float)SamplePos / (float)SampleRate;
}

LWSound &LWSound::SetVolume(float Volume, bool MakeEvent) {
	Volume = std::min<float>(std::max<float>(0.0f, Volume), 1.0f);
	if(MakeEvent){
		uint32_t Event = LWAudioDriver::EncodeEventVolume(Volume);
		m_AudioDriver->PushEvent(this, Event | LWAudioDriver::Event_Volume);
	} else m_Volume = Volume;
	return *this;
}

LWSound &LWSound::SetPan(float Pan, bool MakeEvent) {
	Pan = std::min<float>(std::max<float>(-1.0f, Pan), 1.0f);
	if (MakeEvent) {
		uint32_t Event = LWAudioDriver::EncodeEventPan(Pan) | LWAudioDriver::Event_Pan;
		m_AudioDriver->PushEvent(this, Event);
	} else m_Pan = Pan;
	return *this;
}

LWSound &LWSound::SetSpeed(float Speed, bool MakeEvent){
	Speed = std::min<float>(std::max<float>(0.0f, Speed), 10.0f);
	if (MakeEvent) {
		uint32_t Event = LWAudioDriver::EncodeEventSpeed(Speed) | LWAudioDriver::Event_Speed;
		m_AudioDriver->PushEvent(this, Event);
	} else m_PlaybackSpeed = Speed;
	return *this;
}

LWSound &LWSound::SetCurveDistance(float CurveDistance) {
	m_CurveDistance = CurveDistance;
	return *this;
}

LWSound &LWSound::SeekTo(uint32_t SeekPos) {
	m_AudioDriver->PushEvent(this, LWAudioDriver::Event_Seek | SeekPos);
	return *this;
}

bool LWSound::Release(void) {
	return m_AudioDriver->PushEvent(this, LWAudioDriver::Event_Release);
}

LWSound &LWSound::Mute(void) {
	m_AudioDriver->PushEvent(this, LWAudioDriver::Event_Mute);
	return *this;
}

LWSound &LWSound::Unmute(void) {
	m_AudioDriver->PushEvent(this, LWAudioDriver::Event_Unmute);
	return *this;
}

LWSound &LWSound::SetTotalSamples(uint64_t TotalSamples) {
	m_TotalSamples = TotalSamples;
	return *this;
}

LWSound &LWSound::SetTimePlayed(uint64_t TimePlayed) {
	m_TimePlayed = TimePlayed;
	return *this;
}

LWSound &LWSound::SetSamplesLoaded(uint64_t LoadedSamples) {
	m_SamplesLoaded = LoadedSamples;
	return *this;
}

LWSound &LWSound::SetFinishCount(uint32_t FinishCount) {
	m_FinishCount = FinishCount;
	return *this;
}

LWSound &LWSound::SetUserData(void *UserData) {
	m_UserData = UserData;
	return *this;
}

LWSound &LWSound::SetFlag(uint32_t Flag) {
	m_Flag = Flag;
	return *this;
}

LWSound &LWSound::Set3DPosition(const LWVector3f &Position) {
	const float e = std::numeric_limits<float>::epsilon();
	LWVector3f Dis = Position - m_Position;
	m_Position = Position;
	if (fabs(Dis.x) <= e && fabs(Dis.y) <= e && fabs(Dis.z) <= e) return *this;
	m_Flag |= PositionChanged;
	return *this;
}



LWSound &LWSound::Pause(void) {
	m_AudioDriver->PushEvent(this, LWAudioDriver::Event_Stop);
	return *this;
}

LWSound &LWSound::Resume(void) {
	m_AudioDriver->PushEvent(this, LWAudioDriver::Event_Play);
	return *this;
}

uint64_t LWSound::GetTimePlayed(void) const{
	return m_TimePlayed;
}

uint64_t LWSound::GetSamplesLoaded(void) const {
	return m_SamplesLoaded;
}

bool LWSound::isPlaying(void) const {
	return (m_Flag&(Playing|SystemPaused)) == Playing;
}

bool LWSound::isStreaming(void) const {
	return (m_Flag&Streaming) != 0;
}

bool LWSound::isMuted(void) const {
	return (m_Flag&(Muted | SystemMuted)) != 0;
}

bool LWSound::isFinished(void) const {
	return m_FinishCount >= GetPlayCount();
}

float LWSound::GetPosition(void) const {
	float fTimePlayed = (float)m_TimePlayed / (float)LWTimer::GetResolution();
	return fmodf(fTimePlayed, m_AudioStream->GetLength());
}

uint64_t LWSound::GetTotalSamples(void) const {
	return m_TotalSamples;
}

uint32_t LWSound::GetPlayCount(void) const {
	return(uint32_t)( m_TotalSamples / m_AudioStream->GetSampleLength());
}

float LWSound::GetVolume(void) const {
	return m_Volume;
}

float LWSound::GetPan(void) const {
	return m_Pan;
}

float LWSound::GetSpeed(void) const {
	return m_PlaybackSpeed;
}

uint32_t LWSound::GetType(void) const {
	return (m_Flag&TypeBits);
}

uint32_t LWSound::GetFlag(void) const {
	return m_Flag;
}

uint32_t LWSound::GetFinishedCount(void) const {
	return m_FinishCount;
}

uint32_t LWSound::GetChannel(void) const {
	return m_Channel;
}

LWVector3f LWSound::Get3DPosition(void) const {
	return m_Position;
}

LWSoundContext &LWSound::GetContext(void) {
	return m_Context;
}

LWAudioStream *LWSound::GetAudioStream(void) {
	return m_AudioStream;
}

float LWSound::GetCurveDistance(void) const {
	return m_CurveDistance;
}

void *LWSound::GetUserData(void) {
	return m_UserData;
}

LWSound::LWSound(LWAudioDriver *Driver, LWAudioStream *Stream,uint32_t Type, uint32_t Channel, uint32_t LoopCount, void *UserData) : m_Channel(Channel), m_AudioDriver(Driver), m_AudioStream(Stream), m_UserData(UserData), m_Flag(Type) {
	m_TotalSamples = (uint64_t)(LoopCount + 1)*(uint64_t)m_AudioStream->GetSampleLength();
}
