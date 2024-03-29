#pragma once

#include <windows.h>
#include <wtsapi32.h>
#include <ktmw32.h>

#include <chrono>
#include <ctime>
#include <cwchar>
#include <system_error>

#define MACRO_SOURCE_LOCATION() __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")"

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
				throw std::system_error(::GetLastError(), std::system_category(), MACRO_SOURCE_LOCATION());
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
				throw std::system_error(::GetLastError(), std::system_category(), MACRO_SOURCE_LOCATION());
			}
		}

		~Transaction() noexcept
		{
			if (std::uncaught_exceptions() == 0)
			{
				::CommitTransaction(m_handle);
			}

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

		~RegistryKey() noexcept
		{
			if (m_key != nullptr)
			{
				::RegCloseKey(m_key);
			}
		}

		operator HKEY() const
		{
			return m_key;
		}

		operator PHKEY()
		{
			if (m_key != nullptr)
			{
				::RegCloseKey(m_key), m_key = nullptr;
			}

			return &m_key;
		}

		RegistryKey(const RegistryKey &) = delete;
		RegistryKey & operator=(const RegistryKey &) = delete;
	};
}

namespace reg
{
	void OpenKey(HKEY root, const wchar_t * subkey, HANDLE txn, PHKEY result)
	{
		if (auto status = ::RegOpenKeyTransacted(root, subkey, 0, KEY_WRITE, result, txn, nullptr))
		{
			throw std::system_error(status, std::system_category(), MACRO_SOURCE_LOCATION());
		}
	}

	void CreateValue(HKEY key, const wchar_t * name, const wchar_t * value)
	{
		auto size = std::wcslen(value) * sizeof(wchar_t);

		if (auto status = ::RegSetValueEx(key, name, 0, REG_SZ, (const BYTE *) value, (DWORD) size))
		{
			throw std::system_error(status, std::system_category(), MACRO_SOURCE_LOCATION());
		}
	}

	void DeleteValue(HKEY key, const wchar_t * name)
	{
		if (auto status = ::RegDeleteValue(key, name))
		{
			if (status != ERROR_FILE_NOT_FOUND)
			{
				throw std::system_error(status, std::system_category(), MACRO_SOURCE_LOCATION());
			}
		}
	}
}
