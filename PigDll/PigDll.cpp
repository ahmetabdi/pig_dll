// PigDll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "PigDll.h"
#include "Hooks.h"
#include <psapi.h>
#include "Speed.h"

void writeMem(DWORD dwAddress, void* bytes, DWORD dwSize);
MODULEINFO GetModuleInfo(char *szModule);
DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char * szMask);
void PlaceJMP(BYTE *pAddress, DWORD dwJumpTo, DWORD dwLen);

DWORD pEngine = (DWORD)GetModuleHandle("bsengine_Shipping.dll");
MODULEINFO pEngineModule = GetModuleInfo("bsengine_Shipping.dll");
DWORD pEngineSize = pEngineModule.SizeOfImage;

static DWORD InsertKeyPressTime = 0;
static bool InsertToggle = false;
static DWORD DeleteKeyPressTime = 0;
static bool DeleteToggle = false;

DWORD WINAPI RunLoop(HMODULE hModule) {

	// Player Falling Function
	//"bsengine_Shipping.dll"+5A8FF6: F3 0F 11 43 5C        -  movss [ebx+5C],xmm0
	// bsengine_Shipping.dll,F3 0F 11 43 5C 8B 10
	// db F3 0F 11 43 5C
	// db 90 90 90 90 90
	DWORD PlayerFallAddress = FindPattern(pEngine, pEngineSize, (PBYTE)"\xF3\x0F\x11\x43\x5C\x8B\x10", "xxxxxxx");

	// Player Moving Function
	// SpeedStanding = 248, Sprinting = 434 (MAX is 415!)
	// "bsengine_Shipping.dll"+635221: F3 0F 11 86 BC 02 00 00     -  movss [esi+000002BC],xmm0
	// db F3 0F 11 86 BC 02 00 00
	//DWORD PlayerSpeed = FindPattern(pEngine, pEngineSize, (PBYTE)"\xF3\x0F\x11\x86\xBC\x02\x00\x00\x0F", "xxxxxxxxx");
	//PlayerSpeedJmpBack = PlayerSpeed + 0x08;
	
	// Nop Player Moving Function
	// "bsengine_Shipping.dll"+6020B7: D9 9E BC 02 00 00              -  fstp dword ptr [esi+000002BC]
	// db D9 9E BC 02 00 00
	// db 90 90 90 90 90 90
	//DWORD PlayerSpeedDisable = FindPattern(pEngine, pEngineSize, (PBYTE)"\xD9\x9E\xBC\x02\x00\x00\xE8", "xxxxxxx");
	Speed *speed;
	speed = new Speed();

	while (true)
	{
		if (GetAsyncKeyState(VK_INSERT) & 1 && GetTickCount() - InsertKeyPressTime > 1000)
		{
			Beep(750, 100);
			InsertKeyPressTime = GetTickCount();
			InsertToggle = !InsertToggle;
			if (InsertToggle) {
				writeMem(PlayerFallAddress, "\x90\x90\x90\x90\x90", 5);
			}
			else {
				writeMem(PlayerFallAddress, "\xF3\x0F\x11\x43\x5C", 5);
			}

		}

		if (GetAsyncKeyState(VK_DELETE) & 1 && GetTickCount() - DeleteKeyPressTime > 1000) 
		{
			Beep(750, 100);
			DeleteKeyPressTime = GetTickCount();
			DeleteToggle = !DeleteToggle;

			if (DeleteToggle) {
				speed->setSpeedMultiplier(3);
				speed->enableSpeedHack();
			}
			else {
				speed->setSpeedMultiplier(1);
				speed->disableSpeedHack();
			}
			
			/*
			DeleteKeyPressTime = GetTickCount();
			DeleteToggle = !DeleteToggle;
			if (DeleteToggle) {
				PlaceJMP((BYTE*)PlayerSpeed, (DWORD)PlayerSpeedFunc, 8);
				writeMem(PlayerSpeedDisable, "\x90\x90\x90\x90\x90\x90", 6);
			}
			else {
				writeMem(PlayerSpeed, "\xF3\x0F\x11\x86\xBC\x02\x00\x00", 8);
				writeMem(PlayerSpeedDisable, "\xD9\x9E\xBC\x02\x00\x00", 6);
			}
			*/

			//MessageBoxA(NULL, "PlayerSpeed!", "Reflective Dll Injection", MB_OK);
			//PlaceJMP((BYTE*)PlayerSpeed, (DWORD)PlayerSpeedFunc, 8);
		}

		if (GetAsyncKeyState(VK_F12))
		{
			Beep(750, 100);
			break;
		}
		/*
		if (InsertToggle)
		{
			//MessageBoxA(NULL, "Hello from DllMain!", "Reflective Dll Injection", MB_OK);
		}
		*/
	}
	FreeLibraryAndExitThread(hModule, NULL);
	return 0;
}

void writeMem(DWORD dwAddress, void* bytes, DWORD dwSize) {
	DWORD flOldProtect = 0;
	VirtualProtect((void*)dwAddress, dwSize, PAGE_READWRITE, &flOldProtect);
	memcpy((DWORD *)dwAddress, bytes, dwSize);
	VirtualProtect((void*)dwAddress, dwSize, flOldProtect, &flOldProtect);
}

MODULEINFO GetModuleInfo(char *szModule)
{
	MODULEINFO modinfo = { 0 };
	HMODULE hModule = GetModuleHandle(szModule);
	if (hModule == 0) return modinfo;
	GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
	return modinfo;
}

bool bCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask)   return 0;
	return (*szMask) == NULL;
}

DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char * szMask)
{
	for (DWORD i = 0; i<dwLen; i++)
		if (bCompare((BYTE*)(dwAddress + i), bMask, szMask))  return (DWORD)(dwAddress + i);
	return 0;
}

void PlaceJMP(BYTE *Address, DWORD jumpTo, DWORD length = 5)
{
	DWORD dwOldProtect, dwBkup, dwRelAddr;

	//give that address read and write permissions and store the old permissions at oldProtection
	VirtualProtect(Address, length, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	// Calculate the "distance" we're gonna have to jump - the size of the JMP instruction
	dwRelAddr = (DWORD)(jumpTo - (DWORD)Address) - 5;

	// Write the JMP opcode @ our jump position...
	*Address = 0xE9;

	// Write the offset to where we're gonna jump
	//The instruction will then become JMP ff002123 for example
	*((DWORD *)(Address + 0x1)) = dwRelAddr;

	// Overwrite the rest of the bytes with NOPs
	//ensuring no instruction is Half overwritten(To prevent any crashes)
	for (DWORD x = 0x5; x < length; x++)
		*(Address + x) = 0x90;

	// Restore the default permissions
	VirtualProtect(Address, length, dwOldProtect, &dwBkup);
}