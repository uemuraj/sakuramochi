#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wtsapi32.h>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	// https://docs.microsoft.com/en-us/windows/win32/api/wtsapi32/nf-wtsapi32-wtsquerysessioninformationa#remarks

	void * pBuffer{};
	DWORD nBytes{};

	if (::WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, WTS_CURRENT_SESSION, WTSClientProtocolType, (LPWSTR *) &pBuffer, &nBytes))
	{
		if (nBytes == sizeof(USHORT))
		{
			switch (*(USHORT *) pBuffer)
			{
			case 0:
				::OutputDebugString(TEXT("The console session.\r\n"));
				break;
			case 1:
				::OutputDebugString(TEXT("This value is retained for legacy purposes.\r\n"));
				break;
			case 2:
				::OutputDebugString(TEXT("The RDP protocol.\r\n"));
				break;
			}
		}

		::WTSFreeMemory(pBuffer);

		return 0;
	}

	return 1;
}
