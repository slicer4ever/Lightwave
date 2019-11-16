#ifndef LWAUDIODRIVER_H
#define LWAUDIODRIVER_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWMatrix.h"
#include "LWAudio/LWTypes.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"
#include "LWAudio/LWSound.h"
#include <functional>
#include <vector>

/*!< \brief when loading sound data, this header is placed in front for the underlying audio driver to process. */
struct LWAudioBufferHeader {
	uint32_t m_Type;
	uint32_t m_SampleType;
	uint32_t m_Channels;
	uint32_t m_Rate;
	uint32_t m_SampleSize;
};

struct LWAudioEvent {
	LWSound *m_Source;
	uint32_t m_Event;
};

/*!< \brief the function definition for an audio callback. */
typedef std::function<void(LWSound *S, LWAudioDriver *Driver)> LWAudioCallback;

/*!< \brief audio driver which initiates the audio context, and prepares any sound processing. this object must be created on the heap, as alot of buffers are generated for streaming data.
	 \note if using OpenAL for audio backend, stereo sounds can not be panned or 3d spatialized, only mono sources can be properly positioned.
*/
class LWAudioDriver {
public:
	enum {
		Error = 0x40000000,  /*!< \brief flag that indicates an error occurred during initialization. */
		FormatWAV = 0x0, /*!< \brief when loading audio data, certain formats require knowledge of the audio data for playback. */
		FormatVORBIS = 0x1, /*!< \brief ogg audio format, when loading audio data, certain formats require knowledge of the audio data for playback. */
		CallbackCreate = 0x0, /*!< \brief the callback used when a sound is created. */
		CallbackFinished = 0x1, /*!< \brief the callback used when a sound has finished playing a stream(this is called on every loop finish. ) */
		CallbackReleased = 0x2, /*!< \brief the callback used when a sound if actually released. */
		CallbackCount, /*!< \brief total number of callbacks. */
		Paused=0x1, /*!< \brief flag which indicates the audio system is in a paused state. */
		Muted=0x2, /*!< \brief flag which indicates the audio system is in a muted state. */
		PausedSystem=0x4, /*!< \brief flag used by platforms which pause the system when out of focus event occurs. do not set manually. */
		MutedSystem=0x8, /*!< \brief flag used by platforms which indicate the system should be silenced when out of focus event occurs.  do not set manually. */
		MuteFocusAudio=0x10, /*!< \brief flag which indicates audio should automatically be muted when focus is lost, and resume when gained.  this flag is defaulted on for mobile platforms, and off for desktop platforms. */
		PauseFocusAudio=0x20, /*!< \brief flag which indicates audio should automatically deactivate when focus is lost, and resume when gained.  this flag is off by default. */
		UpdateSoundPositions=0x40, /*!< \brief flag indicating 3d sound positions should change. */
		ListernerPositionChanged = 0x80, /*!< \brief internal flag indicating the 3d listener's position has changed, and prep's UpdateListenerPosition to be set on the next 3d update pass. */
		UpdateListenerPosition = 0x100, /*!< \brief flag to signal to all sounds that the listener's position has changed. */

		ChannelBackgound = 0, /*!< \brief id for volume channel for sounds on the background track. */
		ChannelVoice, /*!< \brief id for volume channel for sounds on the voices track. */
		ChannelEffects, /*!< \brief id for volume channel for sounds on the effects track. */
		ChannelUserA, /*!< \brief id for user defined volume channel. */
		ChannelUserB, /*!< \brief id for user defined volume channel. */
		ChannelUserC, /*!< \brief id for user defined volume channel. */
		ChannelCount, /*!< \brief total channel count. */

		Event_Play = 0, /*!< \brief event id to start playing a sound. */
		Event_Stop = 0x10000000, /*!< \brief event id to stop playing a sound. */
		Event_Seek = 0x20000000, /*!< \brief event id to seek a sound to a specific position. */
		Event_Volume = 0x30000000, /*!< \brief event id for a volume change event. */
		Event_Pan = 0x40000000, /*!< \brief event id for a pan change event. */
		Event_Speed = 0x50000000, /*!< \brief event id for a speed change event. */
		Event_Release = 0x60000000, /*!< \brief event id for releasing a sound. */
		Event_Mute = 0x70000000, /*!< \brief event id for muting the sound. */
		Event_Unmute = 0x80000000, /*!< \brief event id  for unmuting the sound. */
		Event_FocusMute = 0x90000000, /*!< \brief event id for focus muting the sound. */
		Event_FocusPause = 0xA0000000, /*!< \brief event id for focus pausing the sound. */
		Event_Created = 0xB0000000, /*!< \brief event id for sound created. */
		Event_ChannelVolume = 0xC0000000, /*!< \brief event id for volume channel change event. */
		Event_ListenerChanged = 0xD0000000, /*!< \brief event id that listener position has changed. */
		UpdatePositionFrequency = 8, /*!< \brief number of ms to wait before processing sound position changes. */

		Event_ID_Bits = 0xF0000000, /*!< \brief bits that encompass event id. */
		Event_Data_Bits = 0xFFFFFFF, /*!< \brief bits that encompass event data. */
		Event_Data_Channel_Bits = 0xF000000, /*!< \brief bits that encompass an channel event's id. */
		Event_Data_Channel_Volume_Bits = 0xFFFFFF, /*!< \brief bits that encompass an event channel's volume data. */
		Event_Data_Channel_Bitoffset = 0x18, /*!< \brief bitoffset to get the channel id for a channel volume event. */
		EventTableSize = 256 /*!< \brief total size of event queue. */
	};
	/*!< \brief decodes an volume event. */
	static float DecodeEventVolume(uint32_t Event);

	/*!< \brief decodes an channel volume event. */
	static float DecodeEventChannelVolume(uint32_t Event, uint32_t &ChannelID);

	/*!< \brief decodes an pan event. */
	static float DecodeEventPan(uint32_t Event);

	/*!< \brief decodes an speed event. */
	static float DecodeEventSpeed(uint32_t Event);

	/*!< \brief encodes a volume event. */
	static uint32_t EncodeEventVolume(float Volume);

	/*!< \brief encodes a channel volume event. */
	static uint32_t EncodeEventChannelVolume(float Volume, uint32_t ChannelID);

	/*!< \brief encodes a pan event. */
	static uint32_t EncodeEventPan(float Pan);

	/*!< \brief encodes a speed event. */
	static uint32_t EncodeEventSpeed(float Pitch);

	/*!< \brief modify the master volume. */
	LWAudioDriver &SetMasterVolume(float Volume);

	/*!< \brief modify the listener's matrix. */
	LWAudioDriver &SetListener(const LWVector3f &Pos, const LWVector3f &Fwrd, const LWVector3f &Up);

	/*!< \brief modify an individual channel's volume. */
	LWAudioDriver &SetChannelVolume(uint32_t Channel, float Volume);

	/*!< \brief set's the MuteFocusAudio flag if MuteAudioOnFocusLoss is set.  set's the PauseFocusAudio flag if PauseAudioOnFocusLoss is set to true, otherwise disables it. */
	LWAudioDriver &SetAudioFocusBehavior(bool MuteAudioOnFocusLoss, bool PauseAudioOnFocusLoss);

	/*!< \brief sets the user data for the audio driver. */
	LWAudioDriver &SetUserData(void *UserData);

	/*!< \brief increments the audio driver(if the underlying audio driver doesn't already implement multi-threading system).
		 \return true if successfully able to update audio, false if an error occurred.
		 \note mobile platforms default to silencing all audio when not in focus, upon return to focus all audio is returned to normal volume.
	*/
	bool Update(uint64_t lCurrentTime, LWWindow *Window);

	/*!< \brief creates a 2D sound from the specified audio stream.
		 \param Stream the audio stream to play from.
		 \param UserData data attached to the sound object for the application to track the sound.
		 \param Playing indicate if the sound should start playing.
		 \param LoopCount the number of times the sound should loop, the finished callback will be called multiple times on each finish however.
		 \param Volume the individual volume for this sound.
		 \param Speed the playback rate of the sound.
		 \param Pan the control of left / right speaker volume control(-1 = full left, +1 = full right).
		 \return object if sound was created, null if failure to make a sound object.
	*/
	LWSound *CreateSound2D(LWAudioStream *Stream, void *UserData, uint32_t Channel, bool Playing, uint32_t LoopCount = 0, float Volume = 1.0f, float Speed = 1.0f, float Pan = 0.0f);

	/*!< \brief creats a 3D sound from the specified audio stream.
		 \param Stream the audio stream to play from.
		 \param UserData data attached to the sound object for the application to track the sound.
		 \param Playing indicate if the sound should start playing.
		 \param Position spatial position of the sound.
		 \param Volume the individual volume for this sound.
		 \param DistanceCurve the curve for how loud the sound is.
		 \param Speed the playback speed of the sound.
	*/
	LWSound *CreateSound3D(LWAudioStream *Stream, void *UserData, uint32_t Channel, bool Playing, const LWVector3f &Position, uint32_t LoopCount = 0, float Volume = 1.0f, float DistanceCurve = 14.0f, float Speed = 1.0f);

	/*!< \brief pauses all active sounds from playing. */
	LWAudioDriver &PauseAll(void);

	/*!< \brief resumes all active sounds to continue playing. */
	LWAudioDriver &ResumeAll(void);

	/*!< \brief mutes all active sounds that are playing. */
	LWAudioDriver &MuteAll(void);

	/*!< \brief unmutes all active sounds that are playing. */
	LWAudioDriver &UnmuteAll(void);

	/*!< \brief returns the number of active sounds currently being played. */
	uint32_t GetActiveSounds(void) const;
	
	/*!< \brief returns the active sound at the specified index. */
	LWSound *GetSound(uint32_t i);

	/*!< \brief returns the current flag of the audio driver. */
	uint32_t GetFlag(void) const;

	/*!< \brief returns true if the system has been paused(through PauseAll or focusPaused). */
	bool isPaused(void) const;

	/*!< \brief returns true if the system has been muted(through from focus lost. */
	bool isMuted(void) const;

	/*!< \brief pushs an event to be handled by the audio driver in it's next update cycle. 
		 \return returns true if the event could be inserted.
	*/
	bool PushEvent(LWSound *Source, uint32_t Event);

	/*!< \brief returns the master volume for the audio driver. */
	float GetMasterVolume(void) const;

	/*!< \brief returns the volume for the specified channel. */
	float GetChannelVolume(uint32_t ChannelID) const;

	/*!< \brief returns the matrix used on 3D sound positions when calculating where a sound should be coming from. */
	LWMatrix4f GetListenerMatrix(void) const;

	/*!< \brief returns the context used by the audio driver, the application should not require this unless it's targeting a specific platform. */
	LWAudioDriverContext *GetContext(void);

	/*!< \brief returns the user data for the audio driver. */
	void *GetUserData(void);

	/*!< \brief returns true if the UpdateSoundPositions flag is set. */
	bool isUpdatingSoundPosition(void) const;

	/*!< \brief returns true if the UpdateListenerPosition flag is set. */
	bool isUpdatingListenerPosition(void) const;

	/*!< \brief default audio driver constructor. */
	LWAudioDriver(void *UserData, LWAllocator &Allocator, LWAudioCallback FinishedCallback, LWAudioCallback CreateCallback, LWAudioCallback ReleaseCallback);

	/*!< \brief default audio driver destructor. */
	~LWAudioDriver();
private:

	bool PreUpdateSoundsPlatform(uint64_t ElapsedTime);

	bool PostUpdateSoundsPlatform(uint64_t ElapsedTime);

	bool ProcessSoundCreatedEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundReleaseEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundPlayEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundStopEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundMuteEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundUnmuteEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundVolumeEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundPanEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundSpeedEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundSeekEvent(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundCreatedEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundReleaseEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundPlayEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundStopEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundMuteEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundUnmuteEventPlatform(LWSound *Sound, uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessSoundVolumeEventPlatform(LWSound *Sound, float Volume, uint64_t ElapsedTime);

	bool ProcessSoundPanEventPlatform(LWSound *Sound, float Pan, uint64_t ElapsedTime);

	bool ProcessSoundSpeedEventPlatform(LWSound *Sound, float Speed, uint64_t ElapsedTime);

	bool ProcessSoundSeekEventPlatform(LWSound *Sound, uint32_t Seek, uint64_t ElapsedTime);

	bool ProcessPlayEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessStopEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessMuteEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessUnmuteEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessVolumeEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessFocusPauseEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessFocusMuteEvent(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessVolumeEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessPlayEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessStopEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessMuteEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessUnmuteEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessFocusPauseEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool ProcessFocusMuteEventPlatform(uint32_t EventID, uint32_t EventData, uint64_t ElapsedTime);

	bool UpdateSoundPlatform(LWSound *Sound, uint64_t ElapsedTime);

	bool CreateSoundPlatform(LWSound *Sound);

	LWAllocator &m_Allocator;
	LWAudioDriverContext m_Context;
	LWAudioCallback m_CallBacks[CallbackCount];
	std::vector<LWSound*> m_SoundList;
	LWAudioEvent m_EventTable[EventTableSize];
	LWMatrix4f m_ListenerMatrix;
	uint64_t m_LastTime = 0;
	uint64_t m_LastPositionUpdateTime = 0;
	void *m_UserData;
	float m_MasterVolume = 1.0f;
	float m_ChannelVolumes[ChannelCount];
	uint32_t m_Flag = 0;
	uint32_t m_EventReadPosition = 0;
	uint32_t m_EventWritePosition = 0;
};

#endif