#ifndef LWAUDIOEMITTER_H
#define LWAUDIOEMITTER_H
#include "LWCore/LWVector.h"
#include "LWAudio/LWAudioDriver.h"
struct LWAudioEmitter2D {
	float m_Pan = 0.0f;
};

struct LWAudioEmitter3D {
	LWVector3f m_Position = LWVector3f();
};

class LWAudioEmitter {
public:

	LWAudioEmitter();
private:
	std::vector<LWSound*> m_SoundList;

	LWAudioEmitter2D m_Emitter2D;
	LWAudioEmitter3D m_Emitter3D;
	uint32_t m_ActiveSounds = 0;
	float m_Volume = 1.0f;
	float m_Pitch = 1.0f;
	uint32_t m_Flag;


};

#endif