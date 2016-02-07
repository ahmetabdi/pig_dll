DWORD PlayerSpeedJmpBack = 0;
//	__asm mov[esi + 0x2BC], 0x43cf8000
__declspec(naked) void PlayerSpeedFunc()
{
	__asm mov dword ptr[esi + 0x2BC], 0x43cf8000
	__asm jmp[PlayerSpeedJmpBack]
}