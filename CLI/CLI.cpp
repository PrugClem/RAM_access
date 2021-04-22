#include "RAM_access.hpp"

#include <iostream>
#include <vector>

int main()
{
	RAM::error_t err;
#if 0
	RAM::win_t window;
	RAM::handle_t handle;
	window = RAM::win_t::find_window("particles");
	std::clog << "Window Name: " << window.get_title() << std::endl;
	handle = window.get_handle(err);
	if (handle == nullptr)
	{
		std::cerr << "ERROR: " << err.what() << std::endl;
	}
	else
	{
		std::clog << "Proces handle: " << handle << std::endl;
	}

	std::cout << "Process pid: " << GetProcessId(handle) << std::endl;
#else
	std::vector<RAM::win_t> windows;
	RAM::handle_t handle;

	RAM::win_t::get_all_windows(windows);
	std::cout << "Number of total Windows found: " << windows.size() << std::endl;
	RAM::win_t::filter_windows(windows, "particles");
	std::clog << "Number of results: " << windows.size() << std::endl;
	if (windows.size() == 0)
	{
		std::cerr << "No Result found" << std::endl;
		return -1;
	}
	std::clog << "Window title: " << windows.at(0).get_title() << std::endl;
	handle = windows.at(0).get_handle(err);
	if (handle == nullptr)
	{
		std::cerr << err.what() << std::endl;
		return -1;
	}
	std::cout << "Process Handle: " << handle << std::endl;

	RAM::variable<uint32_t> var;
	uint32_t out;
	var.handle = handle;
	var.proc_addr = (void*)0x000000B8BF3EFB38;

	if (!var.get_value(out, err))
	{
		std::cerr << "Error reading value: " << err.what() << std::endl;
	}
	std::clog << "Memory content of process at address " << var.proc_addr << ": " << out << std::endl;
	CloseHandle(handle);
#endif
}
