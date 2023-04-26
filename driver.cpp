#include "driver.h"

uintptr_t ProcId;
uintptr_t BaseId;
HANDLE driver_handle;

bool driver::find_driver() {
	driver_handle = CreateFile((L"driver.sys"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (driver_handle == INVALID_HANDLE_VALUE) {
		std::cout << "Failed to open driver with error " << GetLastError() << std::endl;
		return false;
	}

	return true;
}

void driver::read_virtual_memory(PVOID address, PVOID buffer, DWORD size) {
	t_virtual arguments = { 0 };

	arguments.security_code = code_security;
	arguments.address = (ULONGLONG)address;
	arguments.buffer = (ULONGLONG)buffer;
	arguments.size = size;
	arguments.process_id = ProcId;
	arguments.virtual_mode = virtual_read;

	DeviceIoControl(driver_handle, code_virtual, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}

void driver::write_virtual_memory(PVOID address, PVOID buffer, DWORD size) {
	t_virtual arguments = { 0 };

	arguments.security_code = code_security;
	arguments.address = (ULONGLONG)address;
	arguments.buffer = (ULONGLONG)buffer;
	arguments.size = size;
	arguments.process_id = ProcId;
	arguments.virtual_mode = virtual_write;

	DeviceIoControl(driver_handle, code_virtual, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}

void driver::read_physical_memory(PVOID address, PVOID buffer, DWORD size) {
	t_virtual arguments = { 0 };

	arguments.security_code = code_security;
	arguments.address = (ULONGLONG)address;
	arguments.buffer = (ULONGLONG)buffer;
	arguments.size = size;
	arguments.process_id = ProcId;
	arguments.virtual_mode = physical_read;

	DeviceIoControl(driver_handle, code_physical, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}

void driver::write_physical_memory(PVOID address, PVOID buffer, DWORD size) {
	t_virtual arguments = { 0 };

	arguments.security_code = code_security;
	arguments.address = (ULONGLONG)address;
	arguments.buffer = (ULONGLONG)buffer;
	arguments.size = size;
	arguments.process_id = ProcId;
	arguments.virtual_mode = physical_write;

	DeviceIoControl(driver_handle, code_physical, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}

uintptr_t driver::find_image() {
	uintptr_t image_address = { NULL };
	t_image arguments = { NULL };

	arguments.security_code = code_security;
	arguments.process_id = ProcId;
	arguments.address = (ULONGLONG*)&image_address;

	DWORD bytesReturned = 0;
	if (!DeviceIoControl(driver_handle, code_image, &arguments, sizeof(arguments), &image_address, sizeof(image_address), &bytesReturned, NULL)) {
		DWORD error_code = GetLastError();
		LPSTR error_message = nullptr;
		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			error_code,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&error_message,
			0,
			nullptr
		);
		std::cout << "DeviceIoControl failed with error " << error_code << ": " << error_message << std::endl;
		LocalFree(error_message);
	}

	return image_address;
}


INT32 driver::find_process(LPCTSTR process_name) {
	PROCESSENTRY32 pt;
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pt.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hsnap, &pt)) {
		do {
			if (!lstrcmpi(pt.szExeFile, process_name)) {
				CloseHandle(hsnap);
				ProcId = pt.th32ProcessID;
				return pt.th32ProcessID;
			}
		} while (Process32Next(hsnap, &pt));
	}
	CloseHandle(hsnap);


	return { NULL };
}