#pragma once
#include <Windows.h>

namespace CBR::Engine
{
	class WindowsMain
	{
	public:
		//WindowsMain(HINSTANCE hInst, int nCmdShow) {}
		// 拷贝构造函数,= delete 用于禁止WindowsMain w2(w1); 这样的写法的构造函数
		// 换句话说，你不能通过拷贝已有的 WindowsMain 对象来新建一个 WindowsMain。
		//WindowsMain(const WindowsMain&) = delete;
		// 拷贝赋值运算符,用于 w2 = w1; 这样的写法的构造函数
		// 换句话说，你不能让一个已经存在的 WindowsMain 对象通过赋值语句来获得另一个 WindowsMain 的状态
		//WindowsMain& operator =(const WindowsMain&) = delete;
		//~WindowsMain();

		// TODO: 创建窗口，初始化引擎等处理的入口，从Main直接调用
		static int Run(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow);

		/// <summary>
		/// 轮询并处理系统消息,把消息交给窗口过程函数处理,检测窗口是否收到退出信号
		/// </summary>

	private:
		static bool ProcessMessages();
		static HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	};
};

