#include "Memory.h"

Memory::Memory(const wchar_t* processName) noexcept {
	/*
	Busco el pid del proceso de nombre processName
	El handler (OpenProcess) me da acceso a los recursos del proceso, como su memoria y threads
	*/
	::PROCESSENTRY32 entry = { };
	entry.dwSize = sizeof(::PROCESSENTRY32);
	const auto snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	while (::Process32Next(snapShot, &entry)) {
		if (!_wcsicmp(processName, entry.szExeFile)) {
			processId = entry.th32ProcessID;
			processHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
			break;
		}
	}
	if (snapShot)
		::CloseHandle(snapShot);
}

const std::uintptr_t Memory::GetModuleAddress(const wchar_t* moduleName) const noexcept
{
	::MODULEENTRY32 entry = { };
	entry.dwSize = sizeof(::MODULEENTRY32);

	const auto snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);

	std::uintptr_t result = 0;

	while (::Module32Next(snapShot, &entry)) {
		if (!_wcsicmp(moduleName, entry.szModule)) {
			result = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
			break;
		}
	}

	if (snapShot)
		::CloseHandle(snapShot);

	return result;
}

Memory::~Memory() {
	if (processHandle)
		::CloseHandle(processHandle);
}