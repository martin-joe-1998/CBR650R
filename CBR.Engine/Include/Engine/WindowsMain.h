#pragma once
#include <Windows.h>

namespace CBR::Engine
{
	class WindowsMain
	{
	public:
		using WinProcDelegate = LRESULT(*)(HWND, UINT, WPARAM, LPARAM);
		static void RegisterWndProc(WinProcDelegate pDelegate);
		static void UnregisterWndProc(WinProcDelegate pDelegate);

		// 创建窗口，初始化引擎等处理的入口，从Main直接调用
		static int Run(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow);

		static HWND GetMainWindowHandle();
		static RECT GetDefaultWindowRect();
		static uint32_t GetDefaultScreenWidth();
		static uint32_t GetDefaultScreenHeight();

	private:
		static HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	};
};

