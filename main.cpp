#include <iostream>
#include <Windows.h>
using namespace std;

int InjectDLL(DWORD, char*);
int getDLLpath(char*);
int getPID(int*);
int getProc(HANDLE*, DWORD);

int getDLLpath(char* dll)
{
	std::cout << "Enter the path to your DLL file: \n";
	cin >> dll;
	return 1;
}

int getPID(int* PID)
{
	std::cout << "Enter the PID to your target process: \n";
	cin >> *PID;
	return 1;
}

int getProc(HANDLE* handleToProc, DWORD pid)
{
	*handleToProc = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	DWORD dwLastError = GetLastError();
	if (*handleToProc == NULL)
	{
		std::cout << "Unable to open process. \n";
		return -1;
	}
	else
	{
		std::cout << "Process opened. \n";
	}
}

int InjectDLL(DWORD PID, char* dll)
{
	HANDLE handleToProc;
	LPVOID LoadLibAddr;
	LPVOID baseAddr;
	HANDLE remThread;
	// get lenght dll
	int dllLength = strlen(dll) + 1;
	if (getProc(&handleToProc, PID) < 0) {
		return -1;
	}
	LoadLibAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	if (!LoadLibAddr)
		return -1;

	baseAddr = VirtualAllocEx(handleToProc, NULL, dllLength, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (!baseAddr)
		return -1;

	if (!WriteProcessMemory(handleToProc, baseAddr, dll, dllLength, NULL))
		return -1;

	remThread = CreateRemoteThread(handleToProc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddr, baseAddr, 0, NULL);
	if (!remThread)
		return -1;
	WaitForSingleObject(remThread, INFINITE);

	VirtualFreeEx(handleToProc, baseAddr, dllLength, MEM_RELEASE);

	if (CloseHandle(remThread) == 0)
	{
		std::cout << "Failed to close handle to remote thread";
		return -1;
	}
	if (CloseHandle(handleToProc) == 0)
	{
		std::cout << "Failed to close handle target to process. \n";
		return -1;
	}
}


int main()
{
	SetConsoleTitle("Console Injector");

	int PID = -1;
	char* dll = new char[255];

	getDLLpath(dll);
	getPID(&PID);

	InjectDLL(PID, dll);
	system("pause");
	return 0;
}