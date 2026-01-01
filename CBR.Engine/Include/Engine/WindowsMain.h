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

		/// <summary>
		/// 轮询并处理系统消息,把消息交给窗口过程函数处理,检测窗口是否收到退出信号
		/// </summary>

	private:
		static HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	};
};

