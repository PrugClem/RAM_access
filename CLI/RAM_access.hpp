#pragma once

#include <exception>
#include <sstream>
#include <vector>
#include <string>

#include <Windows.h>
#include <WinUser.h>

namespace RAM
{
	class error_t : std::exception
	{
	private:
		DWORD ecode;
		std::string emessage;
	public:
		error_t() : ecode(0) {}
		error_t(const std::string& context);
		error_t& operator=(const error_t& other);
		bool is_error() const { return this->ecode != ERROR_SUCCESS; }
		operator error_t* () { return this; }
		/**
		 * \brief std::exception overloading of what()
		 */
		virtual const char* what() const throw();
		/**
		 * \brief handle default internal error generation
		 *
		 * \param err pointer to the error variable, optional nullptr for exception
		 * \param context string context for the error message
		 */
		static void cause(error_t* err, const std::string& context);
	};

	class handle_t
	{
	public:
		/**
		 * \brief native Windows Process ID handle
		 */
		HANDLE handle;
		handle_t() : handle(0) {}
		/**
		 * \constructor to create a handle_t from HANDLE
		 */
		handle_t(HANDLE handle) : handle(handle) {}
		/**
		 * \brief cast operator to make handle_t to HANDLE
		 */
		operator HANDLE() { return this->handle; }
	};

	class win_t
	{
	private:
		/**
		 * \brief callback function
		 * 
		 * \param HWND window handle to store
		 * \param pointer to a std::vector of win_t to store the handle
		 * \return TRUE
		 */
		static BOOL __stdcall __store_window(HWND, LPARAM);
	public:
		/**
		 * \brief native windows window handle
		 */
		HWND hwnd;
		win_t() : hwnd(0) {}
		/**
		 * \brief constructor to create a win_t from HWND
		 */
		win_t(HWND hwnd) : hwnd(hwnd) {}
		/**
		 * cast operator to create a HWND from a win_t
		 */
		operator HWND() { return this->hwnd; }
		/** 
		 * \brief get the title of this window 
		 */
		std::string get_title() const;
		RAM::handle_t get_handle(error_t* err = nullptr) const;

		static win_t find_window(const std::string& name);
		/**
		 * \brief get all open windows and store all handles in dst
		 * 
		 * \param dst specify where to store the handles to
		 */
		static void get_all_windows(std::vector<win_t>& dst);
		/**
		 * \brief filter windows in src by title, if the filter is a substring of the window title, the window handle is copied into dst
		 * 
		 * \param src source set of window handles
		 * \param dst destination buffer to write the results into
		 * \param filter string to filter the windows
		 */
		static void filter_windows(const std::vector<win_t>& src, std::vector<win_t>& dst, const std::string& filter);
		/**
		 * \brief filter windows in parameter by title (in-place), if the filter is a substring of the windo title, the window handle is kept in the buffer
		 * 
		 * \param windows input / output vector of window handles
		 * \param filter string to filter the windows
		 */
		static void filter_windows(std::vector<win_t>& windows, const std::string& filter);
	};

	template<typename T>
	class buffer : std::vector<T>
	{
	public:
		/**
		 * \brief read bytes from another process and store them into this buffer
		 * 
		 * \param handle process handle to read from
		 * \param proc_addr start address of the other processes' memory area, reads as much data as this buffer has allocated
		 * \param err error handling pointer, use nullptr for exceptions
		 * \return true if the call was successful
		 */
		bool read(handle_t handle, const void* proc_addr, error_t* err = nullptr)
		{
			SIZE_T len;
			if (ReadProcessMemory(handle, proc_addr, this->data(), this->size() * sizeof(T), &len) == 0)
			{
				error_t::cause(err, "buffer::read: ReadProcessMemory() call failed");
				return false;
			}
			this->resize(len / sizeof(T));
			return true;
		}
		/**
		 * \brief write bytes to a different process from this buffer
		 * 
		 * \param handle process handle to write to
		 * \param proc_addr start address of the other processes' memory area, writes as much data as this buffer holds
		 * \param err error handling pointer, use nullptr for exceptions
		 * \return true if the call was successful
		 */
		bool write(handle_t handle, const void* proc_addr, error_t* err = nullptr)
		{
			SIZE_T len_written;
			if (WriteProcessMemory(handle, proc_addr, this->data, this->size() * sizeof(T), &len_written) == 0)
			{
				error_t::cause(err, "buffer::read: WriteProcessMemory() call failed");
				return false;
			}
			return false;
		}
	};

	template<typename T>
	class variable
	{
	public:
		const void* proc_addr;
		handle_t handle;
		variable() : proc_addr(nullptr) {}
		variable(const void* proc_addr) : proc_addr(proc_addr) {}
		// get the value from the other process
		bool get_value(T& out, error_t* err = nullptr)
		{
			SIZE_T len;
			if (ReadProcessMemory(this->handle, this->proc_addr, &out, sizeof(out), &len) == 0)
			{
				error_t::cause(err, "variable::get_value: ReadProcessMemory() call failed");
				return false;
			}
			return true;
		}
		bool set_value(const T& in, error_t* err = nullptr)
		{
			SIZE_T len_written;
			if (WriteProcessMemory(this->handle, this->proc_addr, &in, sizeof(in), &len_written) == 0)
			{
				error_t::cause(err, "variable::get_value: ReadProcessMemory() call failed");
				return false;
			}
			return true;
		}
	};
}
