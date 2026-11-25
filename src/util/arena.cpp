#include "arena.h"
#include "../macros.h"
#include <Windows.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <errhandlingapi.h>
#include <memoryapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <securitybaseapi.h>
#include <sysinfoapi.h>
#include <winbase.h>
#include <winnt.h>
#include <winsock.h>
#include <winuser.h>

#ifdef DEBUG
#include <string>
#endif

bool mem::create_arena(const size_t capacity, arena &out_arena) noexcept
{
	// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc2
	// Might be able to do better but need to get some progress. Like larger
	// page size?
	// https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-mem_extended_parameter

	// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc2#:~:text=with%20specific%20alignment.-,C%2B%2B,-Copy
	// MEM_ADDRESS_REQUIREMENTS address_requirements{PVOID{nullptr},
	//                                               PVOID{nullptr}, alignment};
	//
	// https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-mem_address_requirements
	MEM_ADDRESS_REQUIREMENTS address_requirements{};
	MEM_EXTENDED_PARAMETER param{};

	// Idk if this is how it should be done, but I just looked at the types and
	// did it based on that. "Seems to be working"
	SYSTEM_INFO sys_info{};
	LPSYSTEM_INFO sys_info_p = &sys_info;
	GetSystemInfo(sys_info_p);

	if (sys_info_p == nullptr)
	{
		LOG("Failed getting sys info");
		return false;
	}

	// LOG("Page size: " << sys_info.dwPageSize);
	// LOG("Allocation granularity: " << sys_info.dwAllocationGranularity);

	const size_t large_page_minimum = GetLargePageMinimum();
	// LOG("Large page minimum: " << large_page_minimum);

	// address_requirements.Alignment = 65536 * 2;
	address_requirements.Alignment = large_page_minimum;

	param.Type = MemExtendedParameterAddressRequirements;
	param.Pointer = &address_requirements;

	// HANDLE token = GetCurrentThreadToken();
	// char token_info[256 * 4]{};
	// unsigned long return_length{};
	// GetTokenInformation(token, TOKEN_INFORMATION_CLASS::TokenPrivileges,
	//                     (void *)token_info, 256 * 4, &return_length);

	// LOG("Priv: " << token_info);
	// LOG("Worked?"); // No

	// void *mem = VirtualAlloc2(GetCurrentProcess(), nullptr, capacity,
	//                           MEM_COMMIT | MEM_RESERVE | MEM_LARGE_PAGES,
	//                           PAGE_READWRITE, &param, 1);

	// This is only if you use MEM_64K_PAGES
	GOODERT((capacity % (UINT16_MAX + 1) == 0),
	        "Capacity '" << capacity
	                     << "' must be a multiple of: " << UINT16_MAX + 1
	                     << " mod result: " << capacity % (UINT16_MAX + 1));

	void *mem = VirtualAlloc2(GetCurrentProcess(), nullptr, capacity,
	                          MEM_COMMIT | MEM_RESERVE | MEM_64K_PAGES,
	                          PAGE_READWRITE, &param, 1);

	if (mem == nullptr)
	{
		LOG("Mem is nullptr");
		return false;
	}

	out_arena.buffer = reinterpret_cast<std::byte *>(mem);
	out_arena.base = 0;
	out_arena.capacity = capacity;

	return true;
}

bool mem::destroy_arena(arena &in_arena) noexcept
{
	// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualfree#:~:text=Value-,Meaning,-MEM_DECOMMIT
	BOOL result = VirtualFree((void *)in_arena.buffer, 0, MEM_RELEASE);
	DWORD error = GetLastError();

#ifdef DEBUG
	// From ai, to get string error code
	auto get_win_error = [](unsigned long err)
	{
		char buf[512];
		DWORD len = FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
			err, 0 /* default language */, buf, sizeof(buf), nullptr);

		if (len == 0) // unknown code, or the message didn't fit in buf
			return "Unknown Win32 error " + std::to_string(err);

		// System messages end in "\r\n"
		while (len > 0 && (buf[len - 1] == '\r' || buf[len - 1] == '\n' ||
		                   buf[len - 1] == ' '))
			--len;
		return std::string(buf, len);
	};
	if (result == 0)
	{
		fprintf(stderr, "VirtualAlloc reserve failed (%lu): %s\n", error,
		        get_win_error(error).c_str());
	}
#endif

	GOODERT((result == 1), "Failed freeing mem");

	if (result == 0)
	{
		return false;
	}

	return true;
}

bool mem::allocate_same_alignment(const size_t bytes, arena &in_arena,
                                  void **out_mem) noexcept
{
	size_t new_arena_base = in_arena.base + bytes;
	if (new_arena_base > in_arena.capacity)
	{
		LOG("Can't allocate over capacity");
		return false;
	}

	// void *allocation_base = in_arena.buffer + in_arena.base;
	*out_mem = in_arena.buffer + in_arena.base;

	in_arena.base = new_arena_base;

	return true;
}

bool mem::get_obj_at_chill(const arena &in_arena, const size_t obj_size,
                           const size_t index, void **out_obj) noexcept
{
	if (!mem::obj_in_range(in_arena, obj_size, index))
	{
		LOG("Obj is outside allocated space");
		return false;
	}

	const size_t obj_base = obj_size * index;
	*out_obj = (void *)(in_arena.buffer + obj_base);

	return true;
}
