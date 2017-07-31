#pragma once
using getsysteminfo_t = VOID(WINAPI*)(LPSYSTEM_INFO);
class patcher
{
	bool _bInitialized, _bHooked;
public:

	patcher();
	~patcher();

	bool installHooks();
	bool uninstallHooks();
};

