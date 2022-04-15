#include "sakuramochi.h"

void put_gmtime(wchar_t(&dest)[21], std::time_t src)
{
	tm buff{};

	if (auto err = gmtime_s(&buff, &src); err != 0)
	{
		throw std::system_error(err, std::generic_category(), __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")");
	}

	std::wcsftime(dest, _countof(dest), L"%Y-%m-%dT%H:%M:%SZ", &buff);
}

namespace reg
{
	void OpenKey(HKEY root, const wchar_t * subkey, HANDLE hTransaction, PHKEY result)
	{
		if (auto status = ::RegOpenKeyTransacted(root, subkey, 0, KEY_WRITE, result, hTransaction, nullptr))
		{
			throw std::system_error(status, std::system_category(), __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")");
		}
	}

	void CreateValue(HKEY key, const wchar_t * name, const wchar_t * value)
	{
		auto size = std::wcslen(value) * sizeof(wchar_t);

		if (auto status = ::RegSetValueEx(key, name, 0, REG_SZ, (const BYTE *) value, (DWORD) size))
		{
			throw std::system_error(status, std::system_category(), __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")");
		}
	}

	void DeleteValue(HKEY key, const wchar_t * name)
	{
		if (auto status = ::RegDeleteValue(key, name))
		{
			if (status != ERROR_FILE_NOT_FOUND)
				throw std::system_error(status, std::system_category(), __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")");
		}
	}
}

void PauseWindowsUpdate(bool pause)
{
	win32::Transaction tx;
	win32::RegistryKey key;

	reg::OpenKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\WindowsUpdate\\UX\\Settings", tx, key);

	if (pause)
	{
		auto now = std::chrono::system_clock::now();
		auto end = now + std::chrono::hours(24 * 14);

		wchar_t startTime[21]{};
		wchar_t endTime[21]{};

		put_gmtime(startTime, std::chrono::system_clock::to_time_t(now));
		put_gmtime(endTime, std::chrono::system_clock::to_time_t(end));

		reg::CreateValue(key, L"PauseFeatureUpdatesStartTime", startTime);
		reg::CreateValue(key, L"PauseFeatureUpdatesEndTime", endTime);
		reg::CreateValue(key, L"PauseQualityUpdatesStartTime", startTime);
		reg::CreateValue(key, L"PauseQualityUpdatesEndTime", endTime);
		reg::CreateValue(key, L"PauseUpdatesStartTime", startTime);
		reg::CreateValue(key, L"PauseUpdatesExpiryTime", endTime);
	}
	else
	{
		reg::DeleteValue(key, L"PauseFeatureUpdatesStartTime");
		reg::DeleteValue(key, L"PauseFeatureUpdatesEndTime");
		reg::DeleteValue(key, L"PauseQualityUpdatesStartTime");
		reg::DeleteValue(key, L"PauseQualityUpdatesEndTime");
		reg::DeleteValue(key, L"PauseUpdatesStartTime");
		reg::DeleteValue(key, L"PauseUpdatesExpiryTime");
	}
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	// TODO: ログアウトまで待機する
	//
	// * コンソールセッションの場合、ログインしたら解除して、ログアウトしたら延期。
	// * リモートセッションの場合、ログインしたら延期して、ログアウトしても延期。（）
	//
	try
	{
		PauseWindowsUpdate(!wts::IsConsoleSession());
		return 0;
	}
	catch (const std::exception & e)
	{
		if (::IsDebuggerPresent())
		{
			::OutputDebugStringA(e.what());
			::OutputDebugStringA("\r\n");
		}
		else
			::MessageBoxA(nullptr, e.what(), "sakuramochi", MB_ICONHAND | MB_OK);

		return 1;
	}
}
