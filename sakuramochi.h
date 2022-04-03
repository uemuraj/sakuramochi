#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wtsapi32.h>
#include <ktmw32.h>

#include <chrono>
#include <ctime>
#include <cwchar>
#include <system_error>

namespace wts
{
	class CurrentSessionInformation
	{
		void * m_buff;
		DWORD m_size;
	public:
		CurrentSessionInformation(WTS_INFO_CLASS infoClass) : m_buff(nullptr), m_size(0)
		{
			if (!::WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, WTS_CURRENT_SESSION, infoClass, (LPWSTR *) &m_buff, &m_size))
			{
				throw std::system_error(::GetLastError(), std::system_category(), __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")");
			}
		}

		~CurrentSessionInformation()
		{
			::WTSFreeMemory(m_buff);
		}

		template<typename T>
		operator T () const
		{
			return *(T *) m_buff;
		}

		CurrentSessionInformation(const CurrentSessionInformation &) = delete;
		CurrentSessionInformation & operator=(const CurrentSessionInformation &) = delete;
	};

	inline bool IsConsoleSession()
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/wtsapi32/ne-wtsapi32-wts_info_class
		CurrentSessionInformation info(WTSClientProtocolType);

		return (USHORT) info == 0;
	}
}

namespace win32
{
	class Transaction
	{
		HANDLE m_handle;
	public:
		Transaction() : m_handle(::CreateTransaction(nullptr, 0, 0, 0, 0, 0, nullptr))
		{
			if (m_handle == INVALID_HANDLE_VALUE)
			{
				throw std::system_error(::GetLastError(), std::system_category(), __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")");
			}
		}

		~Transaction()
		{
			if (std::uncaught_exceptions() == 0)
				::CommitTransaction(m_handle);

			::CloseHandle(m_handle);
		}

		operator HANDLE() const
		{
			return m_handle;
		}

		Transaction(const Transaction &) = delete;
		Transaction & operator=(const Transaction &) = delete;
	};

	class RegistryKey
	{
		HKEY m_key;
	public:
		RegistryKey() : m_key(nullptr)
		{}

		~RegistryKey()
		{
			if (m_key != nullptr)
				::RegCloseKey(m_key);
		}

		operator HKEY() const
		{
			return m_key;
		}

		operator PHKEY()
		{
			if (m_key != nullptr)
				::RegCloseKey(m_key), m_key = nullptr;

			return &m_key;
		}

		RegistryKey(const RegistryKey &) = delete;
		RegistryKey & operator=(const RegistryKey &) = delete;
	};
}
