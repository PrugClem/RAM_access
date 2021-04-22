#include "RAM_access.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

#define LINE_BREAK "==============================================================="
using memory_type = size_t;

void print_window_names(std::vector<RAM::win_t>& windows)
{
	std::cout << LINE_BREAK << std::endl
		      << "listing " << windows.size() << " window names" << std::endl;
	for (size_t i = 0; i < windows.size(); i++) // print listed windows
		std::cout << i << ": \t" << windows.at(i).get_title() << std::endl;
	std::cout << LINE_BREAK << std::endl;
}
bool select_window(std::vector<RAM::win_t>& windows, RAM::win_t& output)
{
	size_t idx;
	print_window_names(windows);
	std::cout << "enter selection from the list above: " << std::flush;
	std::cin >> idx;
	if (idx >= windows.size()) // error case
		return false;
	output = windows.at(idx);
	return true;
}
void filter_windows(std::vector<RAM::win_t>& windows)
{
	std::string filter;

	std::cout << "Enter filter to filter windows: " << std::flush;
	std::cin >> filter;
	RAM::win_t::filter_windows(windows, filter);
}
bool menu_specify_window(std::vector<RAM::win_t>& windows, RAM::win_t& output)
{
	char input;
	if (windows.size() == 0)
	{
		std::cout << "No windows found!" << std::endl
			      << "Please make sure that your desired window is open!" << std::endl;
		exit(1);
	}
	std::cout	<< "Current Number of windows: " << windows.size() << std::endl
				<< "0: abort" << std::endl
		        << "1: list windows" << std::endl
				<< "2: filter windows" << std::endl
				<< "3: select window" << std::endl << "> ";
	std::cin.clear();
	std::cin >> input;
	switch (input)
	{
	case '0':
		exit(0);
	case '1':
		print_window_names(windows);
		return false;
	case '2':
		filter_windows(windows);
		std::cout << "applied filter, " << windows.size() << " windows remaining" << std::endl;
		if (windows.size() == 1)
		{
			std::cout << "window selected" << std::endl;
			output = windows.at(0);
			return true;
		}
		return false;
	case '3':
		select_window(windows, output);
		return true;
	default:
		std::cout << "Unknown menu point " << input << std::endl;
		return false;
	}
}

template<typename T>
bool process_search_memory(RAM::handle_t handle, std::vector<RAM::variable<T> >& out, const void* start, const void* end, size_t step_size, T filter)
{
	RAM::error_t err;
	RAM::buffer<T> buffer;
	uint8_t* cur = (uint8_t*)start;

	out.clear();
	while(cur < end)
	{
		if (cur + step_size > end)
			step_size = (uint8_t*)end - cur;
		buffer.resize(step_size / sizeof(T) );
		//std::clog << "Reading from 0x" << (void*)cur << " to 0x" << (void*)(cur + step_size) << " , buffer size: " << buffer.size() << std::endl;
		if ( buffer.size() != (step_size / sizeof(T)) )
		{
			std::cerr << "Error resizeing input buffer" << std::endl;
		}
		memset(buffer.data(), 0, step_size);
		buffer.read(handle, cur, err);
		for (size_t i = 0; i < buffer.size(); i++)
		{
			if (buffer.at(i) == filter)
			{
				out.push_back(RAM::variable<T>(cur + (i * sizeof(T)) ) );
				//std::cout << "found variable in block [0x" << (void*)cur << " - 0x" << (void*)(cur + step_size) << "] at offset " << (i * sizeof(T)) << std::endl;
			}
		}
		cur += step_size;
	}
	return true;
}
template<typename T>
void update_value(RAM::handle_t handle, std::vector<RAM::variable<T> >& data, const T& new_filter)
{
	RAM::error_t err;
	data.erase(	std::remove_if(data.begin(), data.end(),
				[&](RAM::variable<T>& var)
				{
					T val;
					var.get_value(handle, val, err);
					return (val != new_filter);
				}),
				data.end()
			);
}
template<typename T>
void set_values(RAM::handle_t handle, std::vector<RAM::variable<T> >& data, const T& new_value)
{
	RAM::error_t err;
	for (RAM::variable<T>& var : data)
	{
		var.set_value(handle, new_value, err);
	}
}
template<typename T>
void value_menu(RAM::handle_t handle, std::vector<RAM::variable<T> >& data)
{
	T new_value;
	char input;
	std::cout << std::endl << LINE_BREAK << std::endl
		      << "Currently selected values: " << data.size() << std::endl
		      << "Please specify an action:" << std::endl
			  << "0: exit program" << std::endl
			  << "1: print current found variable addresses" << std::endl
		      << "2: refresh value from the source program" << std::endl
			  << "3: overwrite all values" << std::endl
			  << "> " << std::flush;
	std::cin.clear();
	std::cin >> input;
	switch (input)
	{
	case '0':
		exit(0);
	case '1':
		std::cout << std::endl;
		for (size_t i = 0; i < data.size(); i++)
		{
			std::cout << "result #" << i << " at memory address " << data.at(i).proc_addr << std::endl;
		}
		break;
	case '2':
		std::cout << "Enter the new value: " << std::flush;
		std::cin >> new_value;
		update_value(handle, data, new_value);
		break;
	case '3':
		std::cout << "Enter value to overwrite: " << std::flush;
		std::cin >> new_value;
		set_values(handle, data, new_value);
		break;
	default:
		std::cout << "unknown menu point" << std::endl;
		break;
	}
}
template<typename T>
void memory_manip_value_search(RAM::handle_t handle)
{
	std::vector<RAM::variable<T> > data;
	void* start, * end;
	memory_type filter;
	size_t step_size;

	std::cout << "== specify the memory area to read from == " << std::endl
		<< "start address:  " << std::flush;
	std::cin >> start;
	std::cout << "end address:    " << std::flush;
	std::cin >> end;
	std::cout << "initial filter: " << std::flush;
	std::cin >> filter;
	step_size = 0x8000;
	std::cout << "reading from 0x" << start << " to 0x" << end << " with filter " << filter << std::endl;
	if (process_search_memory(handle, data, start, end, step_size, filter))
	{
		std::cout << "reading successful" << std::endl;
	}
	else
	{
		std::cout << "error reading" << std::endl;
		exit(1);
	}

	//std::cout << "results found: " << variables.size() << std::endl;
	for (;;)value_menu(handle, data);
}

int main()
{
	RAM::error_t err;
	RAM::win_t window;
	std::vector<RAM::win_t> windows;
	RAM::handle_t handle;

	RAM::win_t::get_all_windows(windows);
	while (!menu_specify_window(windows, window));

	std::cout << std::endl << LINE_BREAK << std::endl << std::endl
			  << "selected window: " << window.get_title() << std::endl;

	handle = window.get_handle(err);
	if (handle == nullptr)
	{
		std::cerr << "Error getting the specified process handle: " << err.what() << std::endl;
		exit(1);
	}

	memory_manip_value_search<memory_type>(handle);

	//                 0000007C110FF788:
	//start = (void*)0x0000007C11000000;
	//end   = (void*)0x0000007C110FFFFF;

	handle.close();
}
