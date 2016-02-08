// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "PigDll.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason,LPVOID lpReserved)
{
	// Disables the DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications to reduce the size of the working set.
	// DisableThreadLibraryCalls(hModule);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//Run();
		//MessageBoxA(NULL, "Hello from DllMain!", "Reflective Dll Injection", MB_OK);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&RunLoop, hModule, NULL, NULL);
	}
	return TRUE;
}