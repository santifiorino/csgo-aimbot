#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>

class Memory {
private:
	std::uintptr_t processId = 0;
	void* processHandle = nullptr;

public:
	Memory(const wchar_t* processName) noexcept;
	const std::uintptr_t GetModuleAddress(const wchar_t* moduleName) const noexcept;

	bool isProcessOpened() const {
		return processHandle != nullptr;
	}

	template <typename T>
	constexpr const T Read(const std::uintptr_t& address) const noexcept {
		T value = { };
		::ReadProcessMemory(processHandle, reinterpret_cast<const void*>(address), &value, sizeof(T), NULL);
		return value;
	}

	template <typename T>
	constexpr void Write(const std::uintptr_t& address, const T& value) const noexcept {
		::WriteProcessMemory(processHandle, reinterpret_cast<void*>(address), &value, sizeof(T), NULL);
	}

	~Memory();

};