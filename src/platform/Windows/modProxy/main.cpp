/*
 * Copyright (C) 2016 The Jappsy Open Source Project (http://jappsy.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WINVER
#define WINVER 0x0501	// requires WinME or Win2000 minimum for Display functions
#endif

#include <windows.h>

#include <stdio.h>
#include <conio.h>

#include "modProxy.h"

static HANDLE hThread = 0;
static DWORD dwThreadId = 0;
static HANDLE hExitEvent = 0;
static HANDLE hQueryExitEvent = 0;

LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	void* userData = (void*)GetWindowLong(hWnd,GWL_USERDATA);
	switch (msg) {
		case WM_CREATE:
			SetWindowLong(hWnd,GWL_USERDATA,(long)((LPCREATESTRUCT)lParam)->lpCreateParams);
			//userData = (void*)lParam;
			break;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			SetEvent(hExitEvent);
			break;

		default:
			break;
	}
	return DefWindowProc(hWnd,msg,wParam,lParam);
}

uint32_t threadProc(void* userData) {
	const char wndClass[] = "modProxy@events";
	const char appName[] = "modProxy";

	HINSTANCE hInstance = GetModuleHandle(0);

	WNDCLASSEX wcx;
	wcx.cbSize = sizeof(wcx);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = wndProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hInstance;
	wcx.hIcon = 0;
	wcx.hCursor = LoadCursor(0,IDC_ARROW);
	wcx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcx.lpszMenuName =  0;
	wcx.lpszClassName = wndClass;
	wcx.hIconSm = 0;
	RegisterClassEx(&wcx);

	HWND hMainWnd = CreateWindowEx(WS_EX_TOOLWINDOW, wndClass, appName, WS_POPUP, 0, 0, 640, 480, 0, (HMENU)0, hInstance, (LPVOID)0);

	ShowWindow(hMainWnd,SW_SHOWMINIMIZED);

	MSG msg;
	HANDLE hList[2];
    hList[0] = hExitEvent;
    hList[1] = hQueryExitEvent;

	while (true) {
		DWORD objIndex = MsgWaitForMultipleObjects(2,hList,FALSE,INFINITE,QS_ALLINPUT);
		if (objIndex == WAIT_OBJECT_0) {
			Sleep(0);
			break;
		} else {
			if (objIndex == (WAIT_OBJECT_0+1)) {
				PostMessage(hMainWnd, WM_CLOSE, 0, 0);
			}

			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {
					SetEvent(hQueryExitEvent);
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			Sleep(1);
		}
	}

	ExitThread(0);
}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
			hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)threadProc, 0, CREATE_SUSPENDED, &dwThreadId);
			hExitEvent = CreateEvent(0, false, false, 0);
			hQueryExitEvent = CreateEvent(0, false, false, 0);
			ResumeThread(hThread);
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
			SetEvent(hQueryExitEvent);
			WaitForSingleObject(hThread, INFINITE);
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
