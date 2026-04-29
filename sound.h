#ifndef SOUND_H
#define SOUND_H

#include <AL/alut.h>

ALuint bufIntro, bufGame;
ALuint srcIntro, srcGame;

inline void sound_init() {
    alutInit(0, NULL);
    alGetError();
    float vec[6] = {0.0f,0.0f,1.0f, 0.0f,1.0f,0.0f};
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListenerfv(AL_ORIENTATION, vec);
    alListenerf(AL_GAIN, 1.0f);
    bufIntro = alutCreateBufferFromFile("./jumpscare.wav");
    bufGame  = alutCreateBufferFromFile("./jumpscare.wav");
    alGenSources(1, &srcIntro);
    alGenSources(1, &srcGame);
    alSourcei(srcIntro, AL_BUFFER, bufIntro);
    alSourcei(srcGame,  AL_BUFFER, bufGame);
    alSourcef(srcIntro, AL_GAIN, 1.0f); alSourcef(srcIntro, AL_PITCH, 1.0f);
    alSourcef(srcGame,  AL_GAIN, 1.0f); alSourcef(srcGame,  AL_PITCH, 1.0f);
}

inline void sound_shutdown() {
    alDeleteSources(1, &srcIntro);
    alDeleteSources(1, &srcGame);
    alDeleteBuffers(1, &bufIntro);
    alDeleteBuffers(1, &bufGame);
    ALCcontext *ctx = alcGetCurrentContext();
    ALCdevice  *dev = alcGetContextsDevice(ctx);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx);
    alcCloseDevice(dev);
}

#endif // SOUND_H
