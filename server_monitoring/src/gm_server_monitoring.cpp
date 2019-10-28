// GetMemoryUsage, GetTotalMemory, GetCPUUsage from https://stackoverflow.com/a/64166/4061051

#include "GarrysMod/Lua/Interface.h"
#include <stdio.h>

using namespace GarrysMod::Lua;

#if defined(_WIN32) && !defined(__linux__)

	#include "windows.h"
	#include "psapi.h"
	#include "TCHAR.h"
	#include "pdh.h"

	static HANDLE GMOD_PROCESS;

	SIZE_T GetMemoryUsage() {
		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&pmc), sizeof(pmc));
		return pmc.WorkingSetSize;
	}

	DWORDLONG GetTotalMemory() {
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);
		return memInfo.ullTotalPhys;
	}

	static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
	static int numProcessors;
	double GetCPUUsage() {
		FILETIME ftime, fsys, fuser;
		ULARGE_INTEGER now, sys, user;
		double percent;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&now, &ftime, sizeof(FILETIME));

		GetProcessTimes(GMOD_PROCESS, &ftime, &ftime, &fsys, &fuser);
		memcpy(&sys, &fsys, sizeof(FILETIME));
		memcpy(&user, &fuser, sizeof(FILETIME));
		percent = (sys.QuadPart - lastSysCPU.QuadPart) +
			(user.QuadPart - lastUserCPU.QuadPart);
		percent /= (now.QuadPart - lastCPU.QuadPart);
		percent /= numProcessors;
		lastCPU = now;
		lastUserCPU = user;
		lastSysCPU = sys;

		return max(percent * 100, 0);
	}
	void InitGetCPUUsage() {
		SYSTEM_INFO sysInfo;
		FILETIME ftime, fsys, fuser;

		GetSystemInfo(&sysInfo);
		numProcessors = sysInfo.dwNumberOfProcessors;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&lastCPU, &ftime, sizeof(FILETIME));

		GMOD_PROCESS = GetCurrentProcess();
		GetProcessTimes(GMOD_PROCESS, &ftime, &ftime, &fsys, &fuser);
		memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
		memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
	}

#elif defined(__linux__ ) && !defined(_WIN32)

	#include "sys/types.h"
	#include "sys/sysinfo.h"
	#include "stdlib.h"
	#include "stdio.h"
	#include "string.h"
	#include "sys/times.h"
	#include "sys/vtimes.h"
	#include <algorithm>

	int parseLine(char* line) {
		int i = strlen(line);
		const char* p = line;
		while (*p <'0' || *p > '9') p++;
		line[i - 3] = '\0';
		i = atoi(p);
		return i;
	}
	int GetMemoryUsage() {
		FILE* file = fopen("/proc/self/status", "r");
		int result = -1;
		char line[128];

		while (fgets(line, 128, file) != NULL) {
			if (strncmp(line, "VmRSS:", 6) == 0) {
				result = parseLine(line);
				break;
			}
		}
		fclose(file);

		return result;
	}

	long long GetTotalMemory() {
		struct sysinfo memInfo;

		sysinfo(&memInfo);
		long long totalPhysMem = memInfo.totalram;
		totalPhysMem *= memInfo.mem_unit;

		return totalPhysMem;
	}

	static clock_t lastCPU, lastSysCPU, lastUserCPU;
	static int numProcessors;
	double GetCPUUsage() {
		struct tms timeSample;
		clock_t now;
		double percent;

		now = times(&timeSample);
		if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
			timeSample.tms_utime < lastUserCPU) {
			percent = -1.0;
		}
		else {
			percent = (timeSample.tms_stime - lastSysCPU) +
				(timeSample.tms_utime - lastUserCPU);
			percent /= (now - lastCPU);
			percent /= numProcessors;
			percent *= 100;
		}
		lastCPU = now;
		lastSysCPU = timeSample.tms_stime;
		lastUserCPU = timeSample.tms_utime;
		
		return std::max(percent, (double)0);
	}
	void InitGetCPUUsage() {
		FILE* file;
		struct tms timeSample;
		char line[128];

		lastCPU = times(&timeSample);
		lastSysCPU = timeSample.tms_stime;
		lastUserCPU = timeSample.tms_utime;

		file = fopen("/proc/cpuinfo", "r");
		numProcessors = 0;
		while (fgets(line, 128, file) != NULL) {
			if (strncmp(line, "processor", 9) == 0) numProcessors++;
		}
		fclose(file);
	}

#endif

LUA_FUNCTION(ServerMonitoring_GetMemoryUsage)
{
	LUA->PushNumber(GetMemoryUsage());
	return 1;
}

LUA_FUNCTION(ServerMonitoring_GetTotalMemory)
{
	LUA->PushNumber(GetTotalMemory());
	return 1;
}

LUA_FUNCTION(ServerMonitoring_GetCPUUsage)
{
	LUA->PushNumber(GetCPUUsage());
	return 1;
}

GMOD_MODULE_OPEN()
{
	LUA->PushSpecial(SPECIAL_GLOB);

	LUA->CreateTable();

		LUA->PushString("GetMemoryUsage");
			LUA->PushCFunction(ServerMonitoring_GetMemoryUsage);
		LUA->SetTable(-3);

		LUA->PushString("GetTotalMemory");
			LUA->PushCFunction(ServerMonitoring_GetTotalMemory);
		LUA->SetTable(-3);

		LUA->PushString("GetCPUUsage");
			LUA->PushCFunction(ServerMonitoring_GetCPUUsage);
		LUA->SetTable(-3);

	LUA->SetField(-2, "ServerMonitoring");

	LUA->Pop();

	InitGetCPUUsage();

	return 0;
}

GMOD_MODULE_CLOSE()
{
    return 0;
}
