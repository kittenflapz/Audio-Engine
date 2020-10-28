#pragma once
#include "SoundEngine.h"
#include "NotePlayer.h"
#include "FileHelpers.h"
#include <windows.h>
#include <dos.h>
#include <stdio.h>
#include <vector>

// WELCOME TO THE SOUNDERDOME 
// Functions for playing and stopping sounds, making effects, 
// playing sounds with those effects, and creating new sounds in the form of wav files


// BASIC PLAY/STOP FUNCTIONS 

void PlayASound(const char* fileName, bool looping, FX effectType, float volume = DSBVOLUME_MAX, float frequency = DSBFREQUENCY_ORIGINAL, float pan = DSBPAN_CENTER)
{
	DWORD flags = (looping) ? DSBPLAY_LOOPING : 0;
	SoundEngine::GetInstance().PlaySound(fileName, flags, effectType, volume, frequency, pan);
}

void StopSound(const char* fileName)
{
	SoundEngine::GetInstance().StopSound(fileName);
}

bool IsSoundPlaying(const char* fileName)
{
	return SoundEngine::GetInstance().IsPlaying(fileName);
}

// FUN STUFF

// Array of notes
std::vector<std::string> notes = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };

// Function to make .wav file of every note from c0 to b8 (if they don't exist already in the Sounds folder)
void MakeNotes()
{
	for (string note : notes)
	{
		for (int i = 0; i < 9; i++)
		{
			// check if file exists
			if (fileExists("./Sounds/" + note + to_string(i) + ".wav"))
			{
				// do nothing
				cout << "Sound file exists, not creating it" << endl;
			}
			else
			{
				CreateWavFile(note + to_string(i));
			}
		}
	}
}

// EFFECTS ZONE

// CHORUS

// wetDryMix: 0-100, default 50
// depth: 0-1000, default 10
// feedback: 0-99, default 25
// frequency: 0-10, default 1.1
// waveform: default DSFXCHORUS_WAVE_SIN, or defined TRIANGLE
// delay: 0-20, default 16
// phase: 0-4
void SetChorusParams(float wetDryMix, float depth, float feedback, float frequency, long waveform, float delay, long phase)
{
	SoundEngine::GetInstance().SetChorusParams(wetDryMix, depth, feedback, frequency, waveform, delay, phase);
}


// COMPRESSOR

// gain: -60 to 60, default 0
// attack: 0.01 to 500, default 10
// release: 50 to 3000, default 200
// threshold: -60 to 0, default -20
// ratio: 1 to 100, default 3 (meaning 3:1 compression)
// predelay: 0 to 4, default 4
void SetCompressorParams(float gain, float attack, float release, float threshold, float ratio, float predelay)
{
	SoundEngine::GetInstance().SetCompressorParams(gain, attack, release, threshold, ratio, predelay);
}


// DISTORTION

// gain: -60 to 0, default -18
// edge: 0 to 100, default 15
// post EQ center freq: 100 to 8000, default 2400
// post EQ bandwidth:  100 to 8000, default 2400
// pre lowpass cutoff: 100 to 8000, default 8000
void SetDistortionParams(float gain, float edge, float postEQCenterFreq, float postEQBandwidth, float preLowpassCutoff)
{
	SoundEngine::GetInstance().SetDistortionParams(gain, edge, postEQCenterFreq, postEQBandwidth, preLowpassCutoff);
}

// ECHO

/*wet dry mix: 0 to 100, default 50
feedback: 0 to 100, default 50
left delay: 1 to 2000, default 500
right delay: 1 to 2000, default 500
Pan delay: Value that specifies whether to swap left and right delays with each successive echo. 
The default value is zero, meaning no swap. Possible values are defined as DSFXECHO_PANDELAY_MIN 
(equivalent to FALSE) and DSFXECHO_PANDELAY_MAX (equivalent to TRUE).*/
void SetEchoParams(float wetDryMix, float feedback, float leftDelay, float rightDelay, long panDelay)
{
	SoundEngine::GetInstance().SetEchoParams(wetDryMix, feedback, leftDelay, rightDelay, panDelay);
}

// FLANGER

/*
wet dry mix: 0 to 100, default 50
depth: 0 to 100, default 100
feedback: -99 to 99, default -50
frequency: 0 to 10, default is 0.25
waveform: DSFXFLANGER_WAVE_TRIANGLE or DSFXFLANGER_WAVE_SINE
delay: 0 to 4, default 2
phase: 0-4, default 2
*/
void SetFlangerParams(float wetDryMix, float depth, float feedback, float frequency, long waveform, float delay, long phase) 
{
	SoundEngine::GetInstance().SetFlangerParams(wetDryMix, depth, feedback, frequency, waveform, delay, phase);
}

// GARGLE

/*
rate: 1 to 1000, default 20
wave shape: DSFXGARGLE_WAVE_TRIANGLE or DSFXGARGLE_WAVE_SQUARE
*/
void SetGargleParams(float rateHz, float waveShape)
{
	SoundEngine::GetInstance().SetGargleParams(rateHz, waveShape);
}

// PARAMETRIC EQ

/*
centre: 80 to 16000, default 8000
bandwidth: 1 to 36, default 12
gain: -15 to 15, default 0
*/
void SetParamEQParams(float centre, float bandwidth, float gain)
{
	SoundEngine::GetInstance().SetParamEQ(centre, bandwidth, gain);
}

// REVERB

/* input gain: -96 to  0, default 0
reverb mix: -96 to 0, default 0
reverb time: 0.001 to 3000, default 1000
high freq reverb time ratio: 0.001 to 0.999, default 0.001
*/
void SetReverbParams(float inputGain, float reverbMix, float reverbTime, float HFRTRatio)
{
	SoundEngine::GetInstance().SetReverbParams(inputGain, reverbMix, reverbTime, HFRTRatio);
}