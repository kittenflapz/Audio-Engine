// The new audio engine! 
// Warning: aggressively commented for learning purposes
// Console color codes for Audio Engine:
// RED - ERROR
// GREEN - SUCCESS
// BLUE - INFO

#ifndef _SOUNDENGINE_H_
#define _SOUNDENGINE_H_

#include <dsound.h>
#include <map>
#include <string>


#include <stdio.h>
#include <tchar.h>
#include <SDKDDKVer.h>
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <iostream>

/* 
From Game Engine Architecture, Third Edition:
WAV is an uncompressed file format created by Microsoft and IBM. Its
use is commonplace on the Windows operating system. Its correct name
is “waveform audio file format” although it is also rarely referred to as
“audio for windows.” The WAV file format is actually one of a family
of formats known as resource interchange file format (RIFF). The contents
of a RIFF file are arranged in chunks, each with a four-character code
(FOURCC) that defines the contents of the chunk and a chunk size field.
The bitstream in a WAV file conforms to the linear pulse-code modulation (LPCM) format. 
WAV files can also contain compressed audio, but
they are most commonly used for storing uncompressed audio data.
*/

// WAV is a RIFF that contains CHUNKS

// Code for wav loading/playing is based on https://www.rastertek.com/dx10tut14.html

enum class FX
{
	NONE,
	CHORUS,
	COMPRESSOR,
	DISTORTION, 
	ECHO,
	FLANGER,
	GARGLE,
	PARAMEQ,
	REVERB
};

class SoundEngine
{
private:
	// We're supporting wave files and need to know about them using their HEADER in order to load in the data
	struct WaveHeaderType
	{
		char			chunkId[4];
		unsigned long	chunkSize;
		char			format[4];
		char			subChunkId[4];
		unsigned long	subChunkSize;
		unsigned short	audioFormat;
		unsigned short	numChannels;
		unsigned long	sampleRate;
		unsigned long	bytesPerSecond;
		unsigned short	blockAlign;
		unsigned short	bitsPerSample;
		char			dataChunkId[4];
		unsigned long	dataSize;
	};

public:
	static SoundEngine& GetInstance();
	SoundEngine();
	~SoundEngine();

	// HWND is a 'Window Handle', and it will tell DirectSound which window to play the sound from
	// Get this from Ogre's RenderWindow->GetCustomAttribute() function
	bool Initialize(HWND);
	void Shutdown();

	// A DWORD is an unsigned int with a range 0 to 4,294,967,295. Windows likes them
	// Pass in a string to the filename, obviously
	bool PlaySound(const char* filename, DWORD flags, FX effectType, float volume, float frequency, float pan);
	bool StopSound(const char* filename);
	bool IsPlaying(const char* filename);

	// Effects parameter settings
	void SetChorusParams(float wetDryMix, float depth, float feedback, float frequency, long waveform, float delay, long phase);
	void SetCompressorParams(float gain, float attack, float release, float threshold, float ratio, float predelay);
	void SetDistortionParams(float gain, float edge, float postEQCenterFreq, float postEQBandwidth, float preLowpassCutoff);
	void SetEchoParams(float wetDryMix, float feedback, float leftDelay, float rightDelay, long panDelay);
	void SetFlangerParams(float wetDryMix, float depth, float feedback, float frequency, long waveform, float delay, long phase);
	void SetGargleParams(DWORD rateHz, DWORD waveShape);
	void SetParamEQ(float centre, float bandwidth, float gain);
	void SetReverbParams(float inputGain, float reverbMix, float reverbTime, float HFRTRatio);

private:
	// For talking to DirectSound
	bool InitializeDirectSound(HWND);
	void ShutdownDirectSound();

	// Where the magic loading happens
	bool LoadWaveFile(const char*);

private:
	// For creating buffer objects, managing devices, and setting up the environment in DirectSound
	IDirectSound8* directSound;

	// Primary buffer for playing sounds
	IDirectSoundBuffer* primaryBuffer;

	// Seconary buffer for loading in sounds
	IDirectSoundBuffer8* secondaryBuffer;

	// A map of sounds, so we can load in and play multiple sounds
	std::map<std::string, IDirectSoundBuffer8*> sounds;

	// ********************** Effects ******************************* //
	DSFXChorus chorus;
	IDirectSoundFXChorus8* fxChorus;

	DSFXCompressor compressor;
	IDirectSoundFXCompressor8* fxCompressor;

	DSFXDistortion distortion;
	IDirectSoundFXDistortion8* fxDistortion;

	DSFXEcho echo;
	IDirectSoundFXEcho8* fxEcho;

	DSFXFlanger flanger;
	IDirectSoundFXFlanger8* fxFlanger;

	DSFXGargle gargle;
	IDirectSoundFXGargle8* fxGargle;

	DSFXParamEq paramEQ;
	IDirectSoundFXParamEq8* fxParamEQ;

	DSFXWavesReverb reverb;
	IDirectSoundFXWavesReverb8* fxReverb;


	LPDWORD resultsCodes;


};

#endif