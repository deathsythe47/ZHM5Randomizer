// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <windows.h>
#include <Unknwnbase.h>
#include <iostream>
#include <filesystem>
#include "Console.h"
#include "Config.h"
#include "SceneLoadObserver.h"
#include "RandomisationMan.h"


typedef DWORD64(__stdcall *DIRECTINPUT8CREATE)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
DIRECTINPUT8CREATE fpDirectInput8Create;

extern "C" __declspec(dllexport)  DWORD64 __stdcall DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter){
	return fpDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

void loadOriginalDInput() {
	char syspath[320];
	HMODULE hMod;
	GetSystemDirectoryA(syspath, 320);
	strcat_s(syspath, "\\dinput8.dll");
	hMod = LoadLibraryA(syspath);
	if(hMod != NULL)
		fpDirectInput8Create = (DIRECTINPUT8CREATE)GetProcAddress(hMod, "DirectInput8Create");
	else {
		MessageBoxA(NULL, "Failed to load DirectInput dll", "", NULL);
	}
}

RandomisationMan* randomisation_man;
SceneLoadObserver* scene_load_observer;

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved ){
	switch (ul_reason_for_call)
    {
	case DLL_PROCESS_ATTACH:
	{
		loadOriginalDInput();

		Config::base_directory = std::filesystem::current_path().generic_string(); //..\\HITMAN2
		Config::loadConfig();

		if(Config::showDebugConsole)
			Console::spawn();

		randomisation_man = new RandomisationMan();
		scene_load_observer = new SceneLoadObserver();

		auto load_callback = std::bind(&RandomisationMan::initializeRandomizers, randomisation_man, std::placeholders::_1);
		scene_load_observer->registerSceneLoadCallback(load_callback);
	}
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

