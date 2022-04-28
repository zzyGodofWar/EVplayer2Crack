#include <Windows.h>
#include <iostream>
using namespace std;
#include <TlHelp32.h>
#include <psapi.h>

#include "inject.dll.h"


DWORD GetProcessIdByName(char * name);
char * UnicodeToAnsi(const wchar_t * szStr);
HMODULE InjectDLL(DWORD pid, char *dllpath);
BOOL GetProcessPathByPID(DWORD PID, LPWSTR path);

int main() {
	SetConsoleTitleW(L"反反截图工具 - By Fizzy");
	DWORD tpid = GetProcessIdByName("EVPlayer2.exe");
	if (!tpid) {
		MessageBoxW(0, L"请先运行 EV加密播放2", L"ERROR", 0);
	}

	wchar_t outputpath[256];
	GetProcessPathByPID(tpid, outputpath);
	wcscat(outputpath, L"zyHook.dll");

	HANDLE file = CreateFileW(outputpath, GENERIC_ALL, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file != INVALID_HANDLE_VALUE) {
		DeleteFile(outputpath);
		CloseHandle(file);
	}

	file = CreateFileW(outputpath, GENERIC_ALL, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		return false;
	}

	DWORD sizeWriten, dwSize = sizeof(hjbuffer);
	BYTE* pByte = new BYTE[dwSize + 1];
	memcpy(pByte, hjbuffer, dwSize);

	BYTE *tmpBuf = pByte;
	do {
		WriteFile(file, tmpBuf, dwSize, &sizeWriten, NULL);
		dwSize -= sizeWriten;
		tmpBuf += sizeWriten;

	} while (sizeWriten>0);
	delete[] pByte;
	CloseHandle(file);

	char *dllpath = UnicodeToAnsi(outputpath);

	if (!InjectDLL(tpid, dllpath)) {
		MessageBoxW(0, L"注入失败，请以右键管理员方式运行", L"ERROR", 0);
	}
	else {
		MessageBoxW(0, L"注入成功\nBy Fizzy", L"SUCCESS", 0);
	}

	return 0;
}

BOOL GetProcessPathByPID(DWORD PID,LPWSTR path) {

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (hProcess <= 0) {
		return 0;
	}
	wchar_t lpFilename[1024];
	GetModuleFileNameEx(hProcess, NULL, lpFilename, 1024);
	CloseHandle(hProcess);

	for (int i = wcslen(lpFilename)-1; i > 0; i--) {
		if (lpFilename[i] == L'\\' || lpFilename[i] == L'//') {
			lpFilename[i + 1] = L'\0';
			break;
		}
	}
	wcscpy(path, lpFilename);
	return 1;
}

BOOL GetProcessNameByPID(DWORD PID,LPWSTR exename) {
	HANDLE hSnapShot;

	PROCESSENTRY32W probuffer;
	if (PID == 0) {
		return 0;
	}

	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	probuffer.dwSize = sizeof(PROCESSENTRY32);
	if (hSnapShot == INVALID_HANDLE_VALUE) {
		return 0;
	}
	if (Process32First(hSnapShot, &probuffer)) {
		do {
			if (probuffer.th32ProcessID == PID) {
				wcscpy(exename, probuffer.szExeFile);
				CloseHandle(hSnapShot);
				return 1;
			}
		} while (Process32Next(hSnapShot, &probuffer));
	}
	else {
		CloseHandle(hSnapShot);
		return 0;
	}
	CloseHandle(hSnapShot);
	return 0;
}



DWORD GetProcessIdByName(char *name) {
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (handle == INVALID_HANDLE_VALUE) {
		return 0;
	}
	PROCESSENTRY32 tProcess = { 0 };
	tProcess.dwSize = sizeof(tProcess);

	if (Process32First(handle, &tProcess)) {
		do {
			char *cntExe = UnicodeToAnsi(tProcess.szExeFile);
			if (!strcmp(cntExe, name)) {
				CloseHandle(handle);
				delete[]cntExe;
				return tProcess.th32ProcessID;
			}
			delete[]cntExe;
		} while (Process32Next(handle, &tProcess));
	}
	CloseHandle(handle);
	return 0;
}

inline char* UnicodeToAnsi(const wchar_t* szStr)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
	{
		return NULL;
	}
	char* pResult = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}

HMODULE InjectDLL(DWORD pid, char *dllpath) {
	HANDLE phandle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	if (!phandle) {
		return 0;
	}

	int namelen = strlen(dllpath) + 1;
	DWORD Addr_dllname = (DWORD)VirtualAllocEx(phandle, 0, namelen, MEM_COMMIT, PAGE_READWRITE);
	if (!Addr_dllname) {
		CloseHandle(phandle);
		return 0;
	}
	if (!WriteProcessMemory(phandle, (LPVOID)Addr_dllname, dllpath, namelen, NULL)) {
		VirtualFreeEx(phandle, (LPVOID)Addr_dllname, 0, MEM_RELEASE);
		CloseHandle(phandle);
		return 0;
	}

	DWORD Addr_LoadLibrary = (DWORD)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA");

	HANDLE thandle = CreateRemoteThread(phandle, 0, 0, (LPTHREAD_START_ROUTINE)Addr_LoadLibrary, (LPVOID)Addr_dllname, 0, 0);
	if (!thandle) {
		VirtualFreeEx(phandle, (LPVOID)Addr_dllname, 0, MEM_RELEASE);
		CloseHandle(phandle);
		return 0;
	}

	DWORD dllhandle;

	WaitForSingleObject(thandle, INFINITE);
	GetExitCodeThread(thandle, &dllhandle);

	VirtualFreeEx(phandle, (LPVOID)Addr_dllname, 0, MEM_RELEASE);
	CloseHandle(thandle);
	CloseHandle(phandle);

	return (HMODULE)dllhandle;
}