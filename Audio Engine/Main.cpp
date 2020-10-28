#include <iostream>
#include <string>
#include "Sound.h"

int main()
{
	HWND windowHandle = GetConsoleWindow();
	SoundEngine::GetInstance().Initialize(windowHandle);
	PlayASound("./Bells.wav", true, FX::DISTORTION, DSBVOLUME_MAX, DSBFREQUENCY_ORIGINAL, DSBPAN_CENTER);
	system("pause");
	return 0;
}