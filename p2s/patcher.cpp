#include "stdafx.h"
#include "patcher.h"

static getsysteminfo_t pTarget{}, pOrig{};
static constexpr DWORD MaxHardwareConcurrency{ 28 };

static VOID WINAPI GetSystemInfo_hook(LPSYSTEM_INFO lpSystemInfo)
{
	pOrig(lpSystemInfo);
	if (lpSystemInfo && lpSystemInfo->dwNumberOfProcessors > MaxHardwareConcurrency)
	{
		lpSystemInfo->dwNumberOfProcessors = MaxHardwareConcurrency;
	}
}

patcher::patcher() : _bInitialized{ MH_Initialize() == MH_OK }, _bHooked{}
{
}

patcher::~patcher()
{
	if (_bInitialized && uninstallHooks())
		MH_Uninitialize();
}

bool patcher::installHooks()
{
	if (_bInitialized && !_bHooked)
	{
		_bHooked = MH_CreateHookApiEx(L"kernel32", "GetSystemInfo", GetSystemInfo_hook, reinterpret_cast<LPVOID*>(&pOrig), reinterpret_cast<LPVOID*>(&pTarget)) == MH_OK && MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
		return _bHooked;
	}
	else return false;
}

bool patcher::uninstallHooks()
{
	if (_bInitialized && _bHooked)
	{
		_bHooked = !(MH_DisableHook(MH_ALL_HOOKS) == MH_OK && MH_RemoveHook(pTarget) == MH_OK);
		return _bHooked;
	}
	return false;
}
