// developed by lexa

#include <Windows.h>
#include <string>
#include <tlhelp32.h>
#include <tchar.h>

#define DEBUG FALSE

DWORD GetPidFromName(PTCHAR processName);
BOOL GetPrivileges();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	DWORD GameProcessId = NULL;
	while (GameProcessId == NULL)
	{
		if (GetAsyncKeyState(VK_END))
		{
			MessageBoxA(NULL, "Hack injector terminated", "Info", MB_ICONINFORMATION);
			return NULL;
		}

		GameProcessId = GetPidFromName(TEXT("BFP4f.exe"));

		Sleep(250);
	}

	TCHAR dllPath[MAX_PATH];
	if (!GetFullPathName(TEXT("inject edecegin dll ismini yaz///"), MAX_PATH, dllPath, NULL))
	{
		MessageBox(NULL, TEXT("Dosya yoksa ne hatası versin///"), TEXT("Hata basligi"), MB_ICONERROR);
		return NULL;
	}

	if (!GetPrivileges()) return NULL;

	HANDLE hGameProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION, FALSE, GameProcessId);
	if (!hGameProcess)
	{
		MessageBox(NULL, TEXT("OpenProcess failed"), TEXT("Error"), MB_ICONERROR);
		return NULL;
	}

	LPVOID hReservedSpaceForDllPath = VirtualAllocEx(hGameProcess, NULL, sizeof(dllPath), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!hReservedSpaceForDllPath)
	{
		MessageBox(NULL, TEXT("VirtualAllocEx failed"), TEXT("Error"), MB_ICONERROR);
		CloseHandle(hGameProcess);
		return NULL;
	}

    HANDLE hRemoteThread = CreateRemoteThread(hGameProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, hReservedSpaceForDllPath, 0, NULL);
    if (!hRemoteThread)
    {
        MessageBox(NULL, TEXT("CreateRemoteThread failed"), TEXT("Error"), MB_ICONERROR);
        CloseHandle(hGameProcess);
        VirtualFreeEx(hGameProcess, hReservedSpaceForDllPath, sizeof(dllPath), MEM_RELEASE);
        return NULL;
    }

    BOOL remoteThreadFinished = FALSE;
    if (hRemoteThread) WaitForSingleObject(hRemoteThread, 5000);

    VirtualFreeEx(hGameProcess, hReservedSpaceForDllPath, sizeof(dllPath), MEM_RELEASE);

    DWORD threadExitCode;
    if (!GetExitCodeThread(hRemoteThread, &threadExitCode))
    {
        MessageBox(NULL, TEXT("GetExitCodeThread failed"), TEXT("Error"), MB_ICONERROR);
        CloseHandle(hGameProcess);
        return NULL;
    }

    if (threadExitCode == STILL_ACTIVE)
    {
        TCHAR buffer[256];
        wsprintf(buffer, TEXT("Remote thread failed.\nThread exit code: 0x%x"), threadExitCode);
        MessageBox(NULL, buffer, TEXT("Info"), MB_ICONERROR);
    }

    CloseHandle(hGameProcess);
    return NULL;
}

DWORD GetPidFromName(PTCHAR processName)
{
    PROCESSENTRY32 proc32entry;
    proc32entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (Process32First(snapshot, &proc32entry) == TRUE)
    {
        while (Process32Next(snapshot, &proc32entry) == TRUE)
        {
            if (_tcsicmp(proc32entry.szExeFile, processName) == 0)
            {
                CloseHandle(snapshot);
                return proc32entry.th32ProcessID;
            }
        }
    }
    CloseHandle(snapshot);
    return NULL;
}

BOOL GetPrivileges()
{
    HANDLE tokenHandle;
    TOKEN_PRIVILEGES tokenPriv;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &tokenHandle) != 0)
    {
        LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tokenPriv.Privileges[0].Luid);
        tokenPriv.PrivilegeCount = 1;
        tokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(tokenHandle, 0, &tokenPriv, sizeof(tokenPriv), NULL, NULL);
    }
    else
    {
        TCHAR buffer[256];
        wsprintf(buffer, TEXT("0x%x"), GetLastError());
        MessageBox(NULL, buffer, TEXT("OpenProcessTokenError"), MB_ICONERROR);
        return FALSE;
    }
}
