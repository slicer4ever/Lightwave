#ifndef LWSOUND_H
#define LWSOUND_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWVector.h"
#include "LWAudio/LWTypes.h"
#include "LWPlatform/LWPlatform.h"

/*!< \brief sound object which contains the entire sound buffer in ram. */
class LWSound {
public:
	enum {
		Playing = 0x1, /*!< \brief flag indicated sound is playing. */
		Finished = 0x2, /*!< \brief flag indicating sound has finished playing(and no loops remain) */
		Streaming = 0x4, /*!< \brief flag indicating sound is being streamed. */
		Muted = 0x8, /*!< \brief flag indicating sound is currently muted. */
		SystemPaused = 0x10, /*!< \brief flag indicating sound has been paused by the system. */
		SystemMuted = 0x20, /*!< \brief flag indicating sound has been muted by the system. */
		PositionChanged = 0x40, /*!< \brief flag indicating 3D position has changed. */
		Sound2D = 0x0, /*!< \brief flag indicating the sound is a simple 2D sound. */
		Sound3D = 0x80, /*!< \brief flag indicating the sound is a 3D spatial sound. */
		TypeBits = 0x80, /*!< \brief bits to get the sound type. */
		TypeBitoffset = 0x7, /*!< \brief bitoffset to get 0 indexed type id. */
		
	};
	/*!< \brief calculates a sample position based on a time stamp(in seconds). */
	static uint32_t CalculatePosition(float Time, uint32_t SampleRate);

	/*!< \brief calculates a time stamp based on the sample position. */
	static float CalculateTime(uint32_t SamplePos, uint32_t SampleRate); 

	/*!< \brief updates the volume of the sound object. 
		 \param MakeEvent will add an event to the audiodriver to change the volume, this parameter should be ignored by the application.

	*/
	LWSound &SetVolume(float Volume, bool MakeEvent = true);

	/*!< \brief updates the pan(left=-1, right=1, center=0) of the sound object.
		 \param MakeEvent will add an event to the audiodriver to change the Pan, this parameter should be ignored by the application.
	*/
	LWSound &SetPan(float Pan, bool MakeEvent = true);

	/*!< \brief updates the playing speed of the sound object.
		 \param MakeEvent will add an event to the audiodriver to change the speed, this parameter should be ignored by the application.
	*/
	LWSound &SetSpeed(float Speed, bool MakeEvent = true);

	/*!< \brief updates the volume curve distance that the sound can be heard from.
		 \note Lightwave uses an inverse square law for sound distances, as such CurveDistance is the distance where sound is at max stregth, CurveDistance*2 will be half strength, CurveDistance*4 will be 1/4 strength, and so on.
	*/
	LWSound &SetCurveDistance(float CurveDistance);
	
	/*!< \brief marks the sound to be released, once this function is called it is no longer safe to work on this sound object. 
		 \return returns true if successfully released.  this can fail if the event table is currently full when release is called, to be fully safe be sure to check this hasn't happened.
	*/
	bool Release(void);

	/*!< \brief sets the flag of the sound object. */
	LWSound &SetFlag(uint32_t Flag);

	/*!< \brief sets the finish count for the sound. */
	LWSound &SetFinishCount(uint32_t FinishCount);
	
	/*!< \brief sets the total samples to be played. */
	LWSound &SetTotalSamples(uint64_t TotalSamples);

	/*!< \brief sets the number of samples that have been loaded in total. */
	LWSound &SetSamplesLoaded(uint64_t LoadedSamples);

	/*!< \brief sets the user data associated with the sound. */
	LWSound &SetUserData(void *UserData);

	/*!< \brief seeks the sound to the position requested(in samples). */
	LWSound &SeekTo(uint32_t SeekPos);

	/*!< \brief sets the sound's 3D position. */
	LWSound &Set3DPosition(const LWVector3f &Position);

	/*!< \brief pauses the sound on the next update. */
	LWSound &Pause(void);

	/*!< \brief mutes the sound on the next update. */
	LWSound &Mute(void);

	/*!< \brief unmutes the sound on the next update. */
	LWSound &Unmute(void);

	/*!< \brief resumes the sound on the next update. */
	LWSound &Resume(void);

	/*!< \brief sets the total time that the sound has been playing for. */
	LWSound &SetTimePlayed(uint64_t TimePlayed);

	/*!< \brief returns the total time that the sound has played for. */
	uint64_t GetTimePlayed(void) const;

	/*!< \brief returns the total samples that have been loaded into the context's buffers. */
	uint64_t GetSamplesLoaded(void) const;

	/*!< \brief returns the total samples to be played. */
	uint64_t GetTotalSamples(void) const;

	/*!< \brief returns the total number of times this sound object is to be played. */
	uint32_t GetPlayCount(void) const;

	/*!< \brief returns the volume for the sound object. */
	float GetVolume(void) const;

	/*!< \brief returns the left-right pan of the 2D sound object (-1 plays all left, +1 all right, 0 is middle.) */
	float GetPan(void) const;

	/*!< \brief returns the playback speed for the sound object. */
	float GetSpeed(void) const;

	/*!< \brief returns the flag for the sound object. */
	uint32_t GetFlag(void) const;

	/*!< \brief returns the type of sound. */
	uint32_t GetType(void) const;

	/*!< \brief returns true if the sound is currently playing. */
	bool isPlaying(void) const;

	/*!< \brief returns true if the sound is streaming source. */
	bool isStreaming(void) const;
	
	/*!< \brief returns true if the sound is 100% finished playing. */
	bool isFinished(void) const;

	/*!< \brief returns true if the sound is currently muted. */
	bool isMuted(void) const;

	/*!< \brief returns the current time-stamp of the sound in seconds. */
	float GetPosition(void) const;

	/*!< \brief returns the sound object's context. an application should not need to access this directly. */
	LWSoundContext &GetContext(void);

	/*!< \brief returns the audio stream associated with the sound. */
	LWAudioStream *GetAudioStream(void);

	/*!< \brief retrieves the associated user data for the sound. */
	void *GetUserData(void);

	/*!< \brief retrieves the number of times the sound has finished. */
	uint32_t GetFinishedCount(void) const;

	/*!< \brief returns the channel the sound is played on. */
	uint32_t GetChannel(void) const;

	/*!< \brief returns the sound's 3D position. */
	LWVector3f Get3DPosition(void) const;

	/*!< \brief returns the curved distance the 3d sound can be heard, within this distance is full sound, at 2x this distance the sound is heard by half, at 4x the distance is 1/4 as loud, and so on following an inverse square law. */
	float GetCurveDistance(void) const;

	/*!< \brief constructs sound object. */
	LWSound(LWAudioDriver *Driver, LWAudioStream *Stream, uint32_t Type, uint32_t Channel, uint32_t LoopCount, void *UserData);

	/*!< \brief default construct operator. */
	LWSound() = default;
private:
	
	LWAudioDriver *m_AudioDriver;
	LWAudioStream *m_AudioStream;
	LWSoundContext m_Context;
	void *m_UserData = nullptr;
	uint64_t m_TotalSamples = 0;
	uint64_t m_SamplesLoaded = 0;
	uint64_t m_TimePlayed = 0;
	uint32_t m_FinishCount = 0;
	LWVector3f m_Position = LWVector3f();
	float m_CurveDistance = 14.0f;
	float m_Volume = 1.0f;
	float m_Pan = 0.0f;
	float m_PlaybackSpeed = 1.0f;
	uint32_t m_Channel = 0;
	uint32_t m_Flag = 0;
};

#endif