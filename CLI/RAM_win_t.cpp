#include "RAM_access.hpp"

BOOL __stdcall RAM::win_t::__store_window(HWND window, LPARAM _dst)
{
	std::vector<win_t>& dst = *reinterpret_cast<std::vector<win_t>*>(_dst);
	dst.push_back(window);
	return TRUE;
}

std::string RAM::win_t::get_title() const
{
	std::string res;
	char buf[256];
	GetWindowTextA(this->hwnd, buf, sizeof(buf));
	res = buf;
	return res;
}

RAM::handle_t RAM::win_t::get_handle(RAM::error_t *err) const
{
	DWORD processID;
	HANDLE handle;

	GetWindowThreadProcessId(this->hwnd, &processID);
	handle =  OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processID);
	if (handle == nullptr)
	{
		std::stringstream context;
		context << "Error Opening Process (pid: " << processID << ")";
		RAM::error_t::cause(err, context.str());
	}
	return handle;
}
RAM::win_t RAM::win_t::find_window(const std::string& name)
{
	return FindWindowA(nullptr, name.c_str());
}

void RAM::win_t::get_all_windows(std::vector<win_t>& dst)
{
	dst.clear();
	EnumWindows(RAM::win_t::__store_window, LPARAM(&dst));
}

void RAM::win_t::filter_windows(const std::vector<win_t>& src, std::vector<win_t>& dst, const std::string& filter)
{
	dst.clear();
	for (const win_t& win : src)
		if (win.get_title().find(filter) != std::string::npos)
			dst.push_back(win);
}

void RAM::win_t::filter_windows(std::vector<win_t>& par, const std::string& filter)
{
	std::vector<win_t> output;
	filter_windows(par, output, filter);
	par = output;
}
