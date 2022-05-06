// CourseWork.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "CourseWork.h"
#include <stdio.h>

#include <string>
#include <WinUser.h>
#include "Commdlg.h"
#include<Strsafe.h>
#include "TlHelp32.h"
#include <string.h>
#include <stdint.h>
#include <memory>

#define MAX_LOADSTRING 100
#define DIV 1048576	// = 1MB
#define ID_CPU 2000
#define ID_MEM 2001
#define ID_MEM2 2003
#define ID_EDIT 2002
#define SIZE 400

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

WCHAR cpu_char[6] = { ' ' };
WCHAR mem_char[8] = { ' ' };
WCHAR buffer[8068] = { ' ' };
WCHAR temp[8068] = { ' ' };
double cpu = -1, memory = -1;
int count = 0, cpu_mas[SIZE] = {-1}, mem_mas[SIZE] = {-1};

FILETIME idleTime;
FILETIME kernelTime;
FILETIME userTime;

FILETIME last_idleTime;
FILETIME last_kernelTime;
FILETIME last_userTime;

static THREADS Thread;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI ThreadProc(LPVOID);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_COURSEWORK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COURSEWORK));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	//wcex.hbrBackground = CreatePatternBrush(LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP1)));

	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_COURSEWORK);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;// , edit_cpu, edit_mem, edit_mem2, edit2;
	
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      300, 200, 0, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   HWND edit_cpu = CreateWindow(L"static", L"", WS_CHILD | WS_VISIBLE | ES_CENTER,
   10, 110, 70, 20, hWnd, (HMENU)ID_CPU, hInstance, NULL);
   HWND edit_mem = CreateWindow(L"static", L"", WS_CHILD | WS_VISIBLE | ES_CENTER,
	   10, 240, 70, 20, hWnd, (HMENU)ID_MEM, hInstance, NULL);
   HWND edit_mem2 = CreateWindow(L"static", L"", WS_CHILD | WS_VISIBLE | ES_CENTER,
	   10, 260, 70, 20, hWnd, (HMENU)ID_MEM2, hInstance, NULL);
   HWND edit2 = CreateWindow(L"static", L"", WS_CHILD | WS_VISIBLE,
	   110, 260, 400, 200, hWnd, (HMENU)ID_EDIT, hInstance, NULL);

   Thread.handleThread = NULL;
   Thread.ThreadFucntion = ThreadProc;
   //Thread.idcTimeEdit = IDC_EDIT1;
   Thread.handleDialog = hWnd;
   Thread.time = 0;
   Thread.handleThread = CreateThread(NULL, 0, Thread.ThreadFucntion, NULL, NULL, &Thread.threadId);

   for (int i = SIZE-1; i >= 0; i--){
	   cpu_mas[i] = -1;
	   mem_mas[i] = -1;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	HPEN hpen, hpen_cpu_graph, hpen_mem_graph;
	RECT rect, rect_cpu, rect_cpu_graph, rect_mem, rect_mem_update, rect_mem_graph, rect_window;
	POINT Min;
	MINMAXINFO *pInfo;

	switch (message)
	{
	case WM_GETMINMAXINFO: //Getting message from Windows

		pInfo = (MINMAXINFO *)lParam;
		Min = { 650, 450 };
		pInfo->ptMinTrackSize = Min;
		return 0;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		// Parse the menu selections:
		
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
		GetClientRect(hWnd, &rect_window);
		hdc = BeginPaint(hWnd, &ps);

		SetRect(&rect, 10, 10, 80, 110);
		if (cpu_mas[0] >= 0)
		{
			SetRect(&rect_cpu, 13, 110 - cpu, 77, 110);
		}

		SetRect(&rect_cpu_graph, 100, 8, rect_window.right-10, 110);
		SetRect(&rect_mem, 10, 140, 80, 240);
		SetRect(&rect_mem_update, 13, rect_mem.bottom - memory, 77, rect_mem.bottom);
		SetRect(&rect_mem_graph, 100, rect_mem.top-2, rect_window.right - 10, rect_mem.bottom);

		FillRect(hdc, &rect, CreateSolidBrush(RGB(0, 0, 0)));
		FillRect(hdc, &rect_cpu, CreateSolidBrush(RGB(0, 255, 0)));
		FillRect(hdc, &rect_cpu_graph, CreateSolidBrush(RGB(0, 0, 0)));
		FillRect(hdc, &rect_mem, CreateSolidBrush(RGB(0, 0, 0)));
		FillRect(hdc, &rect_mem_update, CreateSolidBrush(RGB(0, 255, 0)));
		FillRect(hdc, &rect_mem_graph, CreateSolidBrush(RGB(0, 0, 0)));

		hpen = CreatePen(PS_SOLID, 0, RGB(0, 128, 64));
		SelectObject(hdc, hpen);

		//grid
		
		count+=3;

		for (int i = rect_window.right - 10; i > rect_cpu_graph.left; i -= 10)
		{
			if (i - count % 10 >= rect_cpu_graph.left)
			{
				MoveToEx(hdc, i - count % 10, 8, 0);
				LineTo(hdc, i - count % 10, 110);

				MoveToEx(hdc, i - count % 10, rect_mem.top-2, 0);
				LineTo(hdc, i - count % 10, rect_mem.bottom);
			}
			
		}

		for (int i = 10; i < 110; i += 10)
		{
			MoveToEx(hdc, rect_cpu_graph.left, i, 0);
			LineTo(hdc, rect_window.right - 10, i);
			MoveToEx(hdc, rect_cpu_graph.left, i + 130, 0);
			LineTo(hdc, rect_window.right - 10, i+ 130);
		}

		DeleteObject(hpen);

		//cpu_graph
		hpen_cpu_graph = CreatePen(PS_SOLID, 1, RGB(0, 255, 0)); 
		SelectObject(hdc, hpen_cpu_graph);

		for (int i = 1, j = 10; i < SIZE; i++, j+=3)
		{
			if ((cpu_mas[i+1] >= 0) && ((rect_window.right-j) > rect_cpu_graph.left) )
			{
				MoveToEx(hdc, rect_window.right - j, 110 - cpu_mas[i], 0);
				LineTo(hdc, rect_window.right - j - 3, 110 - cpu_mas[i + 1]);
				
			}
		}

		DeleteObject(hpen_cpu_graph);

		//memory_graph
		hpen_mem_graph = CreatePen(PS_SOLID, 2, RGB(50, 172, 225));
		SelectObject(hdc, hpen_mem_graph);

		for (int i = 0, j = 10; i < SIZE-1; i++, j += 3)
		{
			if ((mem_mas[i+1] >= 0) && ((rect_window.right - j) > rect_mem_graph.left))
			{
				MoveToEx(hdc, rect_window.right - j, rect_mem_graph.bottom - mem_mas[i],0);
				LineTo(hdc, rect_window.right - j - 3, rect_mem_graph.bottom - mem_mas[i + 1]);
			}
		}

		DeleteObject(hpen_mem_graph);

		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	THREADS *ti = (THREADS*)lpParam;
	GetSystemTimes(&last_idleTime, &last_kernelTime, &last_userTime);
	for (;;)
	{
		if (GetSystemTimes(&idleTime, &kernelTime, &userTime) != 0)
		{
			//Sleep(100);

			double usr = userTime.dwLowDateTime - last_userTime.dwLowDateTime;
			double ker = kernelTime.dwLowDateTime - last_kernelTime.dwLowDateTime;
			double idl = idleTime.dwLowDateTime - last_idleTime.dwLowDateTime;

			double sys = ker + usr;

			last_idleTime = idleTime;
			last_userTime = userTime;
			last_kernelTime = kernelTime;

			if (sys != 0) {
				cpu = (sys - idl) / sys * 100;

				for (int i = SIZE - 1; i > 0; i--){
					cpu_mas[i] = cpu_mas[i - 1];
				}
				cpu_mas[0] = cpu;
			}

			if (cpu >= 0)
			{
				wsprintfW(cpu_char, L"%d %% CPU", (int)cpu);
				SetDlgItemText(Thread.handleDialog, ID_CPU, cpu_char);
			}
		}

		MEMORYSTATUSEX statex;

		statex.dwLength = sizeof (statex);

		GlobalMemoryStatusEx(&statex);

		
		memory = statex.dwMemoryLoad;
		for (int i = SIZE - 1; i > 0; i--){
			mem_mas[i] = mem_mas[i - 1];
		}
		mem_mas[0] = memory;

		wsprintfW(mem_char, L"%d %% RAM", (int)memory);
		SetDlgItemText(Thread.handleDialog, ID_MEM, mem_char);

		if(((int)memory > 80) && ((int)cpu > 80))
			wsprintfW(buffer, L"Компьютер перегружен. Оптимизируйте его работу, либо обновите конфигурацию.\r\n");
		else
			wsprintfW(buffer, L"Нагрузка в рамках допустимой. Приятной работы!.\r\n");

		SetDlgItemText(Thread.handleDialog, ID_EDIT, buffer);
		
		InvalidateRect(Thread.handleDialog, NULL, TRUE);
		Sleep(1000);
	}

}