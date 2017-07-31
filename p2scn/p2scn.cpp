// p2scn.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace experimental::filesystem;

const wchar_t *DllFileName = L"p2s.dll";
const wchar_t *Portal2ExecutableName = L"portal2.exe";
const wchar_t *Portal2RegistryKey = L"SOFTWARE\\Classes\\Applications\\portal2.exe\\shell\\open\\command";

static path ExeDirectory;

static bool getPortalExePath(wstring &exePath)
{
	exePath = ExeDirectory / Portal2ExecutableName;
	if (exists(exePath))
		return true;
	LONG size{};
	if (RegQueryValue(HKEY_LOCAL_MACHINE, Portal2RegistryKey, nullptr, &size) == ERROR_SUCCESS && size > 0)
	{
		vector<wchar_t> buffer(static_cast<size_t>(size));
		if (RegQueryValue(HKEY_LOCAL_MACHINE, Portal2RegistryKey, buffer.data(), &size) == ERROR_SUCCESS && size > 0)
		{
			exePath.assign(buffer.data(), size);
			size_t start = exePath.find(L'\"', 0), end;
			if (start != string::npos && (end = exePath.find(L'\"', ++start)) != string::npos)
			{
				exePath = exePath.substr(start, end - start);
				return exists(exePath);
			}
		}
	}
	return false;
}

static bool getDllPath(wstring &dllPath)
{
	dllPath = ExeDirectory / DllFileName;
	return exists(dllPath);
}

static bool injectDll(HANDLE hProcess, HANDLE hThread, const wstring &dllPath)
{
	auto pLoadLibraryW = GetProcAddress(GetModuleHandle(L"kernel32"), "LoadLibraryW");
	if (pLoadLibraryW)
	{
		const auto numBytes = sizeof(wchar_t) * (dllPath.length() + 1);
		SIZE_T numWritten{};
		auto pRemoteMem = VirtualAllocEx(hProcess, nullptr, numBytes, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (pRemoteMem && WriteProcessMemory(hProcess, pRemoteMem, dllPath.data(), numBytes, &numWritten))
		{
			if (!QueueUserAPC(reinterpret_cast<PAPCFUNC>(pLoadLibraryW), hThread, reinterpret_cast<ULONG_PTR>(pRemoteMem)))
			{
				VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
				ResumeThread(hThread);
				return false;
			}
			return true;
		}
	}
	return false;
}

static bool startAndFixPortal(const wstring &portalPath, const wstring &dllPath, const wstring &args)
{
	assert(exists(portalPath));
	assert(exists(dllPath));
	bool result{};
	STARTUPINFO si{};
	PROCESS_INFORMATION pi{};
	si.cb = sizeof(si);
	if (CreateProcess(portalPath.data(), vector<wchar_t>{args.cbegin(), args.cend()}.data(), nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi))
	{
		if (injectDll(pi.hProcess, pi.hThread, dllPath))
		{
			result = ResumeThread(pi.hThread) != -1;
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return result;
}

int wmain(int argc, wchar_t *argv[])
{
	ExeDirectory = path(argv[0]).remove_filename();
	wstring portalPath, dllPath;
	if (getPortalExePath(portalPath))
	{		
		if (getDllPath(dllPath))
		{		
			wcout << L"Portal path: " << portalPath << endl;
			wcout << L"Dll path: " << dllPath << endl;
			if (startAndFixPortal(portalPath, dllPath, argc > 1 ? argv[1] : L""))
			{
				wcout << L"Started" << endl;
				return 0;
			}
			else
			{
				wcout << L"Unable to start game" << endl;
			}
		}
		else
		{
			wcout << L"Unable to locate " << DllFileName << endl;
		}
	}
	else
	{
		wcout << L"Unable to locate " << Portal2ExecutableName << endl;
	}
    return system("pause");
}

