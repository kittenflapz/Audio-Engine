#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")

#include <stdio.h>
#include <tchar.h>
#include <SDKDDKVer.h>
#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include "SoundEngine.h"
#include "ConsoleColor.h"
#include <iostream>


// Singleton Accessor - we definitely (probably) don't need more than one sound player
SoundEngine& SoundEngine::GetInstance()
{
	static SoundEngine theSoundClass;
	return theSoundClass;
}

SoundEngine::SoundEngine() : directSound(nullptr), primaryBuffer(nullptr), secondaryBuffer(nullptr)
{
	// Set some values for effects (better than the original MS default values, which are boring)
	SetChorusParams(50, 50, 20, 1.5, DSFXCHORUS_WAVE_SIN, 16, DSFXCHORUS_PHASE_ZERO);
	SetCompressorParams(10, 10, 100, -50, 3, 4);
	SetDistortionParams(-15, 33, 2400, 1600, 8000);
	SetEchoParams(67, 67, 300, 300, DSFXECHO_PANDELAY_MIN);
	SetFlangerParams(75, 75, -99, .25, DSFXFLANGER_WAVE_TRIANGLE, 4, DSFXFLANGER_PHASE_ZERO);
	SetGargleParams(10, DSFXGARGLE_WAVE_TRIANGLE);
	SetParamEQ(800, 30, -15);
	SetReverbParams(-3, 0, 1000, 0.5);
}

SoundEngine::~SoundEngine()
{
	Shutdown();
}

bool SoundEngine::Initialize(HWND hwnd)
{
	// Initialize direct sound and the primary sound buffer.
	return InitializeDirectSound(hwnd);
}

void SoundEngine::Shutdown()
{
	// Release the secondary buffers in our sound map
	for (auto sound : sounds)
	{
		if (sound.second)
		{
			sound.second->Release();
		}
	}
	// Shutdown the Direct Sound API (this releases the primary buffer and the DirectSound interface)
	ShutdownDirectSound();
	sounds.clear();
	return;
}

// Gets an interface pointer to DirectSound and the default primary sound buffer.
bool SoundEngine::InitializeDirectSound(HWND hwnd)
{
	// An HRESULT is an opaque result handle defined to be zero or positive for a successful return from a function, and negative for a failure.
	HRESULT result;

	// The DSBUFFERDESC structure describes the characteristics of a new buffer object
	DSBUFFERDESC bufferDesc;

	// WAVEFORMATEX structure defines the format of waveform-audio data
	WAVEFORMATEX waveFormat;

	// Initialize the direct sound interface pointer for the default sound device.
	result = DirectSoundCreate8(NULL, &directSound, NULL);
	if (FAILED(result))
	{
		std::cout << red << "ERROR: Failed to initialize direct sound interface pointer for default sound device." << white << std::endl;
		return false;
	}

	// Set the cooperative level to priority so the format of the primary sound buffer can be modified.
	result = directSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
	if (FAILED(result))
	{
		std::cout << red << "ERROR: Failed to set cooperative level to priority." << white << std::endl;
		return false;
	}

	// Setup the primary buffer description.

	bufferDesc.dwSize = sizeof(DSBUFFERDESC); // The size of the structure

	// The dwFlags are the important part of this structure since they tell the buffer what capabilities the audio engine will need to use
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = 0; // Size of this buffer, in bytes.
	bufferDesc.dwReserved = 0; // Reserved. Must be 0.
	bufferDesc.lpwfxFormat = NULL; // Address of a WAVEFORMATEX or WAVEFORMATEXTENSIBLE structure specifying the waveform format for the buffer. This value must be NULL for primary buffers.
	bufferDesc.guid3DAlgorithm = GUID_NULL; // Unique identifier of the two-speaker virtualization algorithm to be used by DirectSound3D hardware emulation

	// Get control of the primary sound buffer on the default sound device.
	result = directSound->CreateSoundBuffer(&bufferDesc, &primaryBuffer, NULL);
	if (FAILED(result))
	{
		std::cout << red << "ERROR: Failed to get control of primary sound buffer." << white << std::endl;
		return false;
	}

	// Setup the format of the primary sound bufffer.
	waveFormat.wFormatTag = WAVE_FORMAT_PCM; // Waveform-audio format type
	waveFormat.nSamplesPerSec = 44100; // Sample rate, in samples per second (hertz).
	waveFormat.wBitsPerSample = 16; // Bits per sample for the wFormatTag format type
	waveFormat.nChannels = 2; // Number of channels in the waveform-audio data. Stereo data uses two channels
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels; // Block alignment, in bytes
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign; // Required average data - transfer rate, in bytes per second, for the format tag
	waveFormat.cbSize = 0; // For WAVE_FORMAT_PCM formats (and only WAVE_FORMAT_PCM formats), this member is ignored

	// Set the primary buffer to be the wave format specified.
	result = primaryBuffer->SetFormat(&waveFormat);
	if (FAILED(result))
	{
		std::cout << red << "ERROR: Failed to set format of primary buffer." << white << std::endl;
		return false;
	}

	std::cout << green << "DirectSound successfully initialized!" << white << std::endl;
	return true;
}


void SoundEngine::ShutdownDirectSound()
{
	// Release the primary sound buffer pointer.
	if (primaryBuffer)
	{
		primaryBuffer->Release();
		primaryBuffer = 0;
	}

	// Release the direct sound interface pointer.
	if (directSound)
	{
		directSound->Release();
		directSound = 0;
	}

	return;
}

// Play the sound!

bool SoundEngine::PlaySound(const char* filename, DWORD flags, FX effectType, float volume = DSBVOLUME_MAX, float frequency = DSBFREQUENCY_ORIGINAL, float pan = DSBPAN_CENTER)
{
	HRESULT result;

	// Set position at the beginning of the sound buffer.
	if (sounds[filename] == nullptr)
	{
		std::cout << blue << "INFO: Adding sound to sound map" << white << std::endl;
		result = LoadWaveFile(filename);
		if (FAILED(result))
		{
			std::cout << red << "ERROR: Couldn't add sound to sound map" << white << std::endl;
			return false;
		}
	}

	if (sounds[filename])
	{
		std::cout << blue << "INFO: Playing existing sound" << white << std::endl;
		result = sounds[filename]->SetCurrentPosition(0);
		if (FAILED(result))
		{
			std::cout << red << "ERROR: Couldn't play existing sound" << white << std::endl;
			return false;
		}

		// Set volume of the buffer to 100%.
		result = sounds[filename]->SetVolume(volume);
		if (FAILED(result))
		{
			std::cout << red << "ERROR: Couldn't change volume" << std::endl;
			return false;
		}

		// CHANGE FREQUENCY (takes a number from 100 to 100000. Defaults to ORIGINAL)
		// calculate good number to pass in
		// pass it in
		result = sounds[filename]->SetFrequency(frequency);
		if (FAILED(result))
		{
			std::cout << red << "ERROR: Couldn't change frequency" << std::endl;
			return false;
		}

		// APPLY PAN (takes a number from -10000 to 10000 where -10 is left, 0 is both, 10 is right)
		// calculate good number to pass in
		// pass it in

		result = sounds[filename]->SetPan(pan);
		if (FAILED(result))
		{
			std::cout << red << "ERROR: Couldn't change pan" << std::endl;
			return false;
		}

		// EFFECTS

		// Make an effects structure

		DSEFFECTDESC effectsDesc;
		
		effectsDesc.dwSize = sizeof(DSEFFECTDESC); // Size of the structure
		effectsDesc.dwFlags = DSFX_LOCSOFTWARE; // Effect must be in software!
		effectsDesc.dwReserved1 = 0;
		effectsDesc.dwReserved2 = 0;

		switch (effectType) {
		case FX::NONE:
			break;
		case FX::CHORUS:
			// Add effect to struct
			effectsDesc.guidDSFXClass = GUID_DSFX_STANDARD_CHORUS;
			resultsCodes = nullptr;
			// SetFX
			// DWORD dwEffectsCount - number of elements in the pDSFXDesc and pdwResultCodes arrays - set to 0 to remove all effects
			// LPDSEFFECTDESC pDSFXDesc - address of an array of DSEFFECTDESC structs
			// LPDWORD pdwResultCodes - address of an array of DWORD elements of size dwEffectsCount
			result = sounds[filename]->SetFX(1, &effectsDesc, resultsCodes);
			if (FAILED(result))
			{
				return false;
			}
			fxChorus = nullptr;
			sounds[filename]->GetObjectInPath(GUID_DSFX_STANDARD_CHORUS, 0, IID_IDirectSoundFXChorus8, (LPVOID*)&fxChorus);
			//sounds[filename]->QueryInterface(IID_IDirectSoundFXChorus8, (LPVOID*)fxChorus);
			result = fxChorus->SetAllParameters(&chorus);
			if (FAILED(result))
			{
				std::cout << red << "ERROR: couldn't set chorus params" << white << std::endl;
			}
			break;
		case FX::COMPRESSOR:
			// Add effect to struct
			effectsDesc.guidDSFXClass = GUID_DSFX_STANDARD_COMPRESSOR;
			resultsCodes = nullptr;
			// SetFX
			// DWORD dwEffectsCount - number of elements in the pDSFXDesc and pdwResultCodes arrays - set to 0 to remove all effects
			// LPDSEFFECTDESC pDSFXDesc - address of an array of DSEFFECTDESC structs
			// LPDWORD pdwResultCodes - address of an array of DWORD elements of size dwEffectsCount
			result = sounds[filename]->SetFX(1, &effectsDesc, resultsCodes);
			if (FAILED(result))
			{
				return false;
			}
			fxCompressor = nullptr;
			sounds[filename]->GetObjectInPath(GUID_DSFX_STANDARD_COMPRESSOR, 0, IID_IDirectSoundFXCompressor8, (LPVOID*)&fxCompressor);
			result = fxCompressor->SetAllParameters(&compressor);
			if (FAILED(result))
			{
				std::cout << red << "ERROR: couldn't set compressor params" << white << std::endl;
			}
			break;
		case FX::DISTORTION:
			// Add effect to struct
			effectsDesc.guidDSFXClass = GUID_DSFX_STANDARD_DISTORTION;
			resultsCodes = nullptr;
			// SetFX
			// DWORD dwEffectsCount - number of elements in the pDSFXDesc and pdwResultCodes arrays - set to 0 to remove all effects
			// LPDSEFFECTDESC pDSFXDesc - address of an array of DSEFFECTDESC structs
			// LPDWORD pdwResultCodes - address of an array of DWORD elements of size dwEffectsCount
			result = sounds[filename]->SetFX(1, &effectsDesc, resultsCodes);
			if (FAILED(result))
			{
				return false;
			}
			fxDistortion = nullptr;
			sounds[filename]->GetObjectInPath(GUID_DSFX_STANDARD_DISTORTION, 0, IID_IDirectSoundFXDistortion8, (LPVOID*)&fxDistortion);
			result = fxDistortion->SetAllParameters(&distortion);
			if (FAILED(result))
			{
				std::cout << red << "ERROR: couldn't set distortion params" << white << std::endl;
			}
			break;
		case FX::ECHO:
			// Add effect to struct
			effectsDesc.guidDSFXClass = GUID_DSFX_STANDARD_ECHO;
			resultsCodes = nullptr;
			// SetFX
			// DWORD dwEffectsCount - number of elements in the pDSFXDesc and pdwResultCodes arrays - set to 0 to remove all effects
			// LPDSEFFECTDESC pDSFXDesc - address of an array of DSEFFECTDESC structs
			// LPDWORD pdwResultCodes - address of an array of DWORD elements of size dwEffectsCount
			result = sounds[filename]->SetFX(1, &effectsDesc, resultsCodes);
			if (FAILED(result))
			{
				return false;
			}
			fxEcho = nullptr;
			sounds[filename]->GetObjectInPath(GUID_DSFX_STANDARD_ECHO, 0, IID_IDirectSoundFXEcho8, (LPVOID*)&fxEcho);
			result = fxEcho->SetAllParameters(&echo);
			if (FAILED(result))
			{
				std::cout << red << "ERROR: couldn't set echo params" << white << std::endl;
			}
			break;
		case FX::FLANGER:
			// Add effect to struct
			effectsDesc.guidDSFXClass = GUID_DSFX_STANDARD_FLANGER;
			resultsCodes = nullptr;
			// SetFX
			// DWORD dwEffectsCount - number of elements in the pDSFXDesc and pdwResultCodes arrays - set to 0 to remove all effects
			// LPDSEFFECTDESC pDSFXDesc - address of an array of DSEFFECTDESC structs
			// LPDWORD pdwResultCodes - address of an array of DWORD elements of size dwEffectsCount
			result = sounds[filename]->SetFX(1, &effectsDesc, resultsCodes);
			if (FAILED(result))
			{
				return false;
			}
			fxFlanger = nullptr;
			sounds[filename]->GetObjectInPath(GUID_DSFX_STANDARD_FLANGER, 0, IID_IDirectSoundFXFlanger8, (LPVOID*)&fxFlanger);
			result = fxFlanger->SetAllParameters(&flanger);
			if (FAILED(result))
			{
				std::cout << red << "ERROR: couldn't set flanger params" << white << std::endl;
			}
			break;
		case FX::GARGLE:
			// Add effect to struct
			effectsDesc.guidDSFXClass = GUID_DSFX_STANDARD_GARGLE;
			resultsCodes = nullptr;
			// SetFX
			// DWORD dwEffectsCount - number of elements in the pDSFXDesc and pdwResultCodes arrays - set to 0 to remove all effects
			// LPDSEFFECTDESC pDSFXDesc - address of an array of DSEFFECTDESC structs
			// LPDWORD pdwResultCodes - address of an array of DWORD elements of size dwEffectsCount
			result = sounds[filename]->SetFX(1, &effectsDesc, resultsCodes);
			if (FAILED(result))
			{
				return false;
			}
			fxGargle = nullptr;
			sounds[filename]->GetObjectInPath(GUID_DSFX_STANDARD_GARGLE, 0, IID_IDirectSoundFXGargle8, (LPVOID*)&fxGargle);
			result = fxGargle->SetAllParameters(&gargle);
			if (FAILED(result))
			{
				std::cout << red << "ERROR: couldn't set flanger params" << white << std::endl;
			}
			break;
		case FX::PARAMEQ:
			// Add effect to struct
			effectsDesc.guidDSFXClass = GUID_DSFX_STANDARD_PARAMEQ;
			resultsCodes = nullptr;
			// SetFX
			// DWORD dwEffectsCount - number of elements in the pDSFXDesc and pdwResultCodes arrays - set to 0 to remove all effects
			// LPDSEFFECTDESC pDSFXDesc - address of an array of DSEFFECTDESC structs
			// LPDWORD pdwResultCodes - address of an array of DWORD elements of size dwEffectsCount
			result = sounds[filename]->SetFX(1, &effectsDesc, resultsCodes);
			if (FAILED(result))
			{
				return false;
			}
			fxParamEQ = nullptr;
			sounds[filename]->GetObjectInPath(GUID_DSFX_STANDARD_PARAMEQ, 0, IID_IDirectSoundFXParamEq8, (LPVOID*)&fxParamEQ);
			result = fxParamEQ->SetAllParameters(&paramEQ);
			if (FAILED(result))
			{
				std::cout << red << "ERROR: couldn't set param EQ params" << white << std::endl;
			}
			break;
		case FX::REVERB:
			// Add effect to struct
			effectsDesc.guidDSFXClass = GUID_DSFX_WAVES_REVERB;
			resultsCodes = nullptr;
			// SetFX
			// DWORD dwEffectsCount - number of elements in the pDSFXDesc and pdwResultCodes arrays - set to 0 to remove all effects
			// LPDSEFFECTDESC pDSFXDesc - address of an array of DSEFFECTDESC structs
			// LPDWORD pdwResultCodes - address of an array of DWORD elements of size dwEffectsCount
			result = sounds[filename]->SetFX(1, &effectsDesc, resultsCodes);
			if (FAILED(result))
			{
				return false;
			}
			fxReverb= nullptr;
			sounds[filename]->GetObjectInPath(GUID_DSFX_WAVES_REVERB, 0, IID_IDirectSoundFXWavesReverb8, (LPVOID*)&fxReverb);
			result = fxReverb->SetAllParameters(&reverb);
			if (FAILED(result))
			{
				std::cout << red << "ERROR: couldn't set param EQ params" << white << std::endl;
			}
			break;
		default:
			break;
		}

		// Play the contents of the secondary sound buffer.
		result = sounds[filename]->Play(0, 0, flags);
		if (FAILED(result))
		{
			std::cout << red << "ERROR: Couldn't play sound" << white <<std::endl;
			return false;
		}
		return true;
	}
	return false;
}

// Returns true if the sound passed in is currently playing

bool SoundEngine::IsPlaying(const char* filename)
{
	if (sounds[filename] != nullptr)
	{
		DWORD dwStatus;
		HRESULT result = sounds[filename]->GetStatus(&dwStatus);
		if (!FAILED(result))
		{
			return (dwStatus & DSBSTATUS_PLAYING) != 0;
		}
	}
	return false;
}

void SoundEngine::SetChorusParams(float wetDryMix, float depth, float feedback, float frequency, long waveform, float delay, long phase)
{
	chorus.fDelay = delay;
	chorus.fDepth = depth;
	chorus.fFeedback = feedback;
	chorus.fFrequency = frequency;
	chorus.fWetDryMix = wetDryMix;
	chorus.lPhase = phase;
	chorus.lWaveform = waveform;
}

void SoundEngine::SetCompressorParams(float gain, float attack, float release, float threshold, float ratio, float predelay)
{
	compressor.fGain = gain;
	compressor.fAttack = attack;
	compressor.fRelease = release;
	compressor.fThreshold = threshold;
	compressor.fRatio = ratio;
	compressor.fPredelay = predelay;
}

void SoundEngine::SetDistortionParams(float gain, float edge, float postEQCenterFreq, float postEQBandwidth, float preLowpassCutoff)
{
	distortion.fGain = gain;
	distortion.fEdge = edge;
	distortion.fPostEQCenterFrequency = postEQCenterFreq;
	distortion.fPostEQBandwidth = postEQBandwidth;
	distortion.fPreLowpassCutoff = preLowpassCutoff;
}

void SoundEngine::SetEchoParams(float wetDryMix, float feedback, float leftDelay, float rightDelay, long panDelay)
{
	echo.fWetDryMix = wetDryMix;
	echo.fFeedback = feedback;
	echo.fLeftDelay = leftDelay;
	echo.fRightDelay = rightDelay;
	echo.lPanDelay = panDelay;
}

void SoundEngine::SetFlangerParams(float wetDryMix, float depth, float feedback, float frequency, long waveform, float delay, long phase)
{
	flanger.fDelay = delay;
	flanger.fDepth = depth;
	flanger.fFeedback = feedback;
	flanger.fFrequency = frequency;
	flanger.fWetDryMix = wetDryMix;
	flanger.lPhase = phase;
	flanger.lWaveform = waveform;
}

void SoundEngine::SetGargleParams(DWORD rateHz, DWORD waveShape)
{
	gargle.dwRateHz = rateHz;
	gargle.dwWaveShape = waveShape;
}

void SoundEngine::SetParamEQ(float centre, float bandwidth, float gain)
{
	paramEQ.fCenter = centre;
	paramEQ.fBandwidth = bandwidth;
	paramEQ.fGain = gain;
}

void SoundEngine::SetReverbParams(float inputGain, float reverbMix, float reverbTime, float HFRTRatio)
{
	reverb.fInGain = inputGain;
	reverb.fReverbMix = reverbMix;
	reverb.fReverbTime = reverbTime;
	reverb.fHighFreqRTRatio = HFRTRatio;
}



bool SoundEngine::StopSound(const char* filename)
{
	if (IsPlaying(filename))
	{
		if (sounds[filename] != nullptr)
		{
			HRESULT result = sounds[filename]->Stop();
			return (FAILED(result));
		}
	}
	return false;
}

// Handles loading in a .wav audio file, copying the data onto a secondary buffer.
bool SoundEngine::LoadWaveFile(const char* filename)
{
	int error; // for finding out what went wrong
	FILE* filePtr = nullptr; // for accessing the file
	unsigned int count; // for reading wave file header
	WaveHeaderType waveFileHeader;
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc; // for the secondary buffers
	HRESULT result; // for finding out what went wrong
	IDirectSoundBuffer* tempBuffer;
	unsigned char* waveData;
	unsigned char* bufferPtr;
	unsigned long bufferSize;

	// Create a pointer-to-pointer-to a buffer which will serve as a secondary buffer and assign to it the address of
	// a new buffer in our <string, buffer> map using the filename passed in to the function
	IDirectSoundBuffer8** secondaryBuffer = &sounds[filename];

	// Open the wave file in binary.
	error = fopen_s(&filePtr, filename, "rb");


	// Read in the wave file header.
	count = (unsigned int)fread(&waveFileHeader, sizeof(waveFileHeader), 1, filePtr);
	if (count != 1)
	{
		std::cout << red << "ERROR: Couldn't read wave file!" << white << std::endl;
		return false;
	}

	// Check that the chunk ID is the RIFF format.
	if ((waveFileHeader.chunkId[0] != 'R') || (waveFileHeader.chunkId[1] != 'I') ||
		(waveFileHeader.chunkId[2] != 'F') || (waveFileHeader.chunkId[3] != 'F'))
	{
		std::cout << red << "ERROR: Chunk ID is not RIFF!" << white << std::endl;
		return false;
	}

	// Check that the file format is the WAVE format.
	if ((waveFileHeader.format[0] != 'W') || (waveFileHeader.format[1] != 'A') ||
		(waveFileHeader.format[2] != 'V') || (waveFileHeader.format[3] != 'E'))
	{
		std::cout << red << "ERROR: File format is not WAVE!" << white << std::endl;
		return false;
	}

	// Check that the sub chunk ID is the fmt format.
	if ((waveFileHeader.subChunkId[0] != 'f') || (waveFileHeader.subChunkId[1] != 'm') ||
		(waveFileHeader.subChunkId[2] != 't') || (waveFileHeader.subChunkId[3] != ' '))
	{
		std::cout << red << "ERROR: Sub chunk ID is not fmt!" << white << std::endl;
		return false;
	}

	// Check for the data chunk header.
	if ((waveFileHeader.dataChunkId[0] != 'd') || (waveFileHeader.dataChunkId[1] != 'a') ||
		(waveFileHeader.dataChunkId[2] != 't') || (waveFileHeader.dataChunkId[3] != 'a'))
	{
		std::cout << red << "ERROR: Couldn't find data chunk header!" << white << std::endl;
		return false;
	}

	// Set the wave format of secondary buffer that this wave file will be loaded onto.
	waveFormat.wFormatTag = waveFileHeader.audioFormat;
	waveFormat.nSamplesPerSec = waveFileHeader.sampleRate;
	waveFormat.wBitsPerSample = waveFileHeader.bitsPerSample;
	waveFormat.nChannels = waveFileHeader.numChannels;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// Set the buffer description of the secondary sound buffer that the wave file will be loaded onto.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLFX | DSBCAPS_CTRLPAN;
	bufferDesc.dwBufferBytes = waveFileHeader.dataSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// Create a temporary sound buffer with the specific buffer settings.
	result = directSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
	if (FAILED(result))
	{
		std::cout << red << "ERROR: Couldn't create secondary sound buffer!" << white << std::endl;
		return false;
	}

	// Test the buffer format against the direct sound 8 interface and create the secondary buffer.
	result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&*secondaryBuffer);
	if (FAILED(result))
	{
		std::cout << red << "Temporary buffer did not have correct interface implementation!" << white << std::endl;
		return false;
	}

	// Release the temporary buffer.
	tempBuffer->Release();
	tempBuffer = 0;

	// Move to the beginning of the wave data which starts at the end of the data chunk header
	// (For more on how this works, see WavFileCreator class)
	fseek(filePtr, sizeof(WaveHeaderType), SEEK_SET);

	// Create a temporary buffer to hold the wave file data.
	waveData = new unsigned char[waveFileHeader.dataSize];
	if (!waveData)
	{
		std::cout << red << "ERROR: Couldn't ceate a temporary buffer!" << white << std::endl;
		return false;
	}

	// Read in the wave file data into the newly created buffer.
	count = (unsigned int)fread(waveData, 1, waveFileHeader.dataSize, filePtr);
	if (count != waveFileHeader.dataSize)
	{
		//std::cout << red << "ERROR: Couldn't read wave file data into temp buffer!" << white << std::endl;
		//return false;
	}

	// Close the file once done reading.
	error = fclose(filePtr);
	if (error != 0)
	{
		std::cout << red << "ERROR: Couldn't close wave file!" << white << std::endl;
		return false;
	}

	// Lock the secondary buffer to write wave data into it.
	result = (*secondaryBuffer)->Lock(0, waveFileHeader.dataSize, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, 0);
	if (FAILED(result))
	{
		std::cout << red << "ERROR: Couldn't lock secondary buffer!" << white << std::endl;
		return false;
	}

	// Copy the wave data into the buffer.
	memcpy(bufferPtr, waveData, waveFileHeader.dataSize);

	// Unlock the secondary buffer after the data has been written to it.
	result = (*secondaryBuffer)->Unlock((void*)bufferPtr, bufferSize, NULL, 0);
	if (FAILED(result))
	{
		std::cout << red << "ERROR: Couldn't unlock secondary buffer!" << white << std::endl;
		return false;
	}

	// Release the wave data since it was copied into the secondary buffer and we do not need it anymore
	delete[] waveData;
	waveData = 0;

	return true;
}

