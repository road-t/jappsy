/*
 * Created by VipDev on 01.04.16.
 *
 * Copyright (C) 2016 The Jappsy Open Source Project
 *
 * Project Web Page http://jappsy.com
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
#include <dbt.h>
#include <wtsapi32.h>

#include <stdio.h>
#include <conio.h>

#include "modDisplay.h"
#include "uWinLock.h"

const char s_display_0[] = "display";
const char s_display_n[] = "\\\\.\\DISPLAY%d";

void dis_ok(const char* display, const char* message) {
//	printf("Display(%s) %s\r\n", display, message);
}

void dis_error(const char* display, const char* message) {
//	printf("Display(%s) Error: %s\r\n", display, message);
}

void dis_status(const char* display, LONG res) {
	switch (res) {
		case DISP_CHANGE_SUCCESSFUL:    dis_ok(display, "The settings change was successful."); break;
		case DISP_CHANGE_BADDUALVIEW:   dis_error(display, "The settings change was unsuccessful because the system is DualView capable."); break;
		case DISP_CHANGE_BADFLAGS:      dis_error(display, "An invalid set of flags was passed in."); break;
		case DISP_CHANGE_BADMODE:       dis_error(display, "The graphics mode is not supported."); break;
		case DISP_CHANGE_BADPARAM:      dis_error(display, "An invalid parameter was passed in. This can include an invalid flag or combination of flags."); break;
		case DISP_CHANGE_FAILED:        dis_error(display, "The display driver failed the specified graphics mode."); break;
		case DISP_CHANGE_NOTUPDATED:    dis_error(display, "Unable to write settings to the registry."); break;
		case DISP_CHANGE_RESTART:       dis_error(display, "The computer must be restarted for the graphics mode to work."); break;
	}
}

static HDEVNOTIFY hDeviceNotify = 0;
const GUID GUID_DEVINTERFACE_MONITOR = {0xe6f07b5f, 0xee97, 0x4a90, 0xb0, 0x76, 0x33, 0xf5, 0x7b, 0xf4, 0xea, 0xa7};

#define TERMINAL_SERVER_KEY "SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\"
#define GLASS_SESSION_ID    "GlassSessionId"

bool IsCurrentSessionRemoteable() {
    bool fIsRemoteable = false;

    if (GetSystemMetrics(SM_REMOTESESSION)) {
        fIsRemoteable = true;
    } else {
        HKEY hRegKey = 0;
        LONG lResult;

        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TERMINAL_SERVER_KEY, 0, KEY_READ, &hRegKey);

        if (lResult == ERROR_SUCCESS) {
            DWORD dwGlassSessionId;
            DWORD cbGlassSessionId = sizeof(dwGlassSessionId);
            DWORD dwType;

            lResult = RegQueryValueEx(hRegKey, GLASS_SESSION_ID, 0, &dwType, (BYTE*) &dwGlassSessionId, &cbGlassSessionId);

            if (lResult == ERROR_SUCCESS) {
                DWORD dwCurrentSessionId;

                if (ProcessIdToSessionId(GetCurrentProcessId(), &dwCurrentSessionId)) {
                    fIsRemoteable = (dwCurrentSessionId != dwGlassSessionId);
                }
            }
        }

        if (hRegKey) {
            RegCloseKey(hRegKey);
        }
    }

    return fIsRemoteable;
}

struct dis_mode_info {
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t freq;
};

struct dis_state {
	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t freq;
	uint32_t orientation;
};

#define DIS_MAX_DISPLAYS	4
#define DIS_MAX_MODES		64

static slLock dis_lock = SL_LOCKED;
static int display0 = 0;
static char dis_names[DIS_MAX_DISPLAYS][MAX_PATH];
static dis_state dis_states[DIS_MAX_DISPLAYS];
static dis_mode_info dis_modes[DIS_MAX_DISPLAYS][DIS_MAX_MODES];
static int32_t dis_modes_count[DIS_MAX_DISPLAYS];
static int32_t dis_names_count = 0;

typedef struct _ddevicemode {
  TCHAR dmDeviceName[CCHDEVICENAME];
  WORD  dmSpecVersion;
  WORD  dmDriverVersion;
  WORD  dmSize;
  WORD  dmDriverExtra;
  DWORD dmFields;
  union {
    struct {
      short dmOrientation;
      short dmPaperSize;
      short dmPaperLength;
      short dmPaperWidth;
      short dmScale;
      short dmCopies;
      short dmDefaultSource;
      short dmPrintQuality;
    };
    struct {
      POINTL dmPosition;
      DWORD  dmDisplayOrientation;
      DWORD  dmDisplayFixedOutput;
    };
  };
  short dmColor;
  short dmDuplex;
  short dmYResolution;
  short dmTTOption;
  short dmCollate;
  TCHAR dmFormName[CCHFORMNAME];
  WORD  dmLogPixels;
  DWORD dmBitsPerPel;
  DWORD dmPelsWidth;
  DWORD dmPelsHeight;
  union {
    DWORD dmDisplayFlags;
    DWORD dmNup;
  };
  DWORD dmDisplayFrequency;
#if (WINVER >= 0x0400)
  DWORD dmICMMethod;
  DWORD dmICMIntent;
  DWORD dmMediaType;
  DWORD dmDitherType;
  DWORD dmReserved1;
  DWORD dmReserved2;
#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
  DWORD dmPanningWidth;
  DWORD dmPanningHeight;
#endif
#endif
} __attribute__((aligned(4))) DDEVMODE, *PDDEVMODE, *LPDDEVMODE;

static dis_change_func dis_change = 0;

void dis_update(bool skipCallback = false) {
	int defaultWidth = 640;
	int defaultHeight = 480;
	int defaultDepth = 32;

	DISPLAY_DEVICE disp;
    BOOL bRet = FALSE;
    DWORD dwNum = 0;
    LONG res;
    bool skip = false;
    int dis_id = 0;

	slSingleLock(&dis_lock);

	static int _display0 = 0;
	static char _dis_names[DIS_MAX_DISPLAYS][MAX_PATH];
	static dis_state _dis_states[DIS_MAX_DISPLAYS];
	static dis_mode_info _dis_modes[DIS_MAX_DISPLAYS][DIS_MAX_MODES];
	static int32_t _dis_modes_count[DIS_MAX_DISPLAYS];

    ZeroMemory(_dis_names, sizeof(_dis_names));
    ZeroMemory(_dis_states, sizeof(_dis_states));
    ZeroMemory(_dis_modes, sizeof(_dis_modes));
    ZeroMemory(_dis_modes_count, sizeof(_dis_modes_count));

	do {
		disp.cb = sizeof( DISPLAY_DEVICE );
        bRet = EnumDisplayDevices(0, dwNum, &disp,0);
        if (bRet) {
			if (strcmp(disp.DeviceName, s_display_0) != 0) {
                DDEVMODE dm;
                ZeroMemory ( &dm, sizeof(dm) );
                dm.dmSize = sizeof(dm);
                if ( EnumDisplaySettingsEx ( disp.DeviceName, ENUM_CURRENT_SETTINGS, (DEVMODE*)&dm, 0 ) != FALSE ) {
					strcpy(_dis_names[dis_id], disp.DeviceName);

					_dis_states[dis_id].x = dm.dmPosition.x;
					_dis_states[dis_id].y = dm.dmPosition.y;
					_dis_states[dis_id].width = dm.dmPelsWidth;
					_dis_states[dis_id].height = dm.dmPelsHeight;
					_dis_states[dis_id].depth = dm.dmBitsPerPel;
					_dis_states[dis_id].freq = dm.dmDisplayFrequency;
					_dis_states[dis_id].orientation = dm.dmDisplayOrientation;

					if ((dm.dmPosition.x == 0) && (dm.dmPosition.y == 0)) {
                        sscanf( disp.DeviceName, s_display_n, &_display0 );
                    }

                    int i = 0;
                    int mode_idx = 0;
                    while ( EnumDisplaySettingsEx ( disp.DeviceName, i, (DEVMODE*)&dm, 0 ) != FALSE ) {
                    	int cur_idx = mode_idx;
						if (mode_idx != 0) {
							for (int j = 0; j < mode_idx; j++) {
								if ((_dis_modes[dis_id][j].width == dm.dmPelsWidth) &&
									(_dis_modes[dis_id][j].height == dm.dmPelsHeight)
								) {
									if ((_dis_modes[dis_id][j].depth >= dm.dmBitsPerPel) ||
										(_dis_modes[dis_id][j].freq >= dm.dmDisplayFrequency)
									) {
										cur_idx = j;
									} else {
										cur_idx = -1;
									}
									break;
								}
							}
						}

						i++;

						if (cur_idx >= 0) {
							if (cur_idx == mode_idx) {
								if (mode_idx >= DIS_MAX_MODES) {
									continue;
								}
								mode_idx++;
							}
							_dis_modes[dis_id][cur_idx].width = dm.dmPelsWidth;
							_dis_modes[dis_id][cur_idx].height = dm.dmPelsHeight;
							_dis_modes[dis_id][cur_idx].depth = dm.dmBitsPerPel;
							_dis_modes[dis_id][cur_idx].freq = dm.dmDisplayFrequency;
						}
                    }

                    _dis_modes_count[dis_id] = mode_idx;

                    dis_id++;
                } else if ((!skip) && ((disp.StateFlags & 0x000000F9) == 0)) { // Экран не подключен и требует подключения // было D9 - теперь Removable игнорируются
                	skip = true;
                    if ( EnumDisplaySettingsEx ( disp.DeviceName, ENUM_REGISTRY_SETTINGS, (DEVMODE*)&dm, 0 ) != FALSE ) {
						// Если зашло сюда, значит монитор отключен - включаем

                        dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_POSITION;
						res = ChangeDisplaySettingsEx(disp.DeviceName,(DEVMODE*)&dm,0,CDS_UPDATEREGISTRY|CDS_NORESET,0);
						dis_status(disp.DeviceName, res);

						if (res == DISP_CHANGE_BADMODE) {
                            dm.dmFields &= ~DM_BITSPERPEL;
                            res = ChangeDisplaySettingsEx(disp.DeviceName,(DEVMODE*)&dm,0,CDS_UPDATEREGISTRY|CDS_NORESET,0);
                            dis_status(disp.DeviceName, res);
						}

						res = ChangeDisplaySettingsEx(0, 0, 0, 0, 0);
                    } else {
                    	// Если зашло сюда, значит монитор отключен и нет записи в реестре
                        ZeroMemory( &dm, sizeof(dm) );
                        dm.dmSize = sizeof(dm);
                        dm.dmPelsWidth = defaultWidth;
                        dm.dmPelsHeight = defaultHeight;
                        dm.dmBitsPerPel = defaultDepth;
                        dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
                        res = ChangeDisplaySettingsEx(disp.DeviceName,(DEVMODE*)&dm,0,CDS_UPDATEREGISTRY|CDS_NORESET,0);
                        dis_status(disp.DeviceName, res);

                        if (res == DISP_CHANGE_BADMODE) {
                            dm.dmFields &= ~DM_BITSPERPEL;
                            res = ChangeDisplaySettingsEx(disp.DeviceName,(DEVMODE*)&dm,0,CDS_UPDATEREGISTRY|CDS_NORESET,0);
                            dis_status(disp.DeviceName, res);
                        }

                        res = ChangeDisplaySettingsEx(0, 0, 0, 0, 0);
                    }
                    continue;
                }
			}
        }
        dwNum++;
        skip = false;
    } while ((bRet) && (dis_id < DIS_MAX_DISPLAYS));

	bool changed = false;
	if (display0 != _display0) {
		display0 = _display0;
		changed = true;
	}
	if (memcmp(dis_names, _dis_names, sizeof(_dis_names)) != 0) {
		memcpy(dis_names, _dis_names, sizeof(_dis_names));
		changed = true;
	}
	if (memcmp(dis_states, _dis_states, sizeof(_dis_states)) != 0) {
		memcpy(dis_states, _dis_states, sizeof(_dis_states));
		changed = true;
	}
	if (memcmp(dis_modes, _dis_modes, sizeof(_dis_modes)) != 0) {
		memcpy(dis_modes, _dis_modes, sizeof(_dis_modes));
	}
	if (memcmp(dis_modes_count, _dis_modes_count, sizeof(_dis_modes_count)) != 0) {
		memcpy(dis_modes_count, _dis_modes_count, sizeof(_dis_modes_count));
	}
	dis_names_count = dis_id;

	if ((changed) && (dis_change != 0)) {
		if (!skipCallback) {
			dis_change();
		}
	}

	slSingleUnlock(&dis_lock);

	/* DEBUG
	if (changed) {
		printf("Display Count: %d\r\n", dis_id);

		for (int i = 0; i < DIS_MAX_DISPLAYS; i++) {
			if (dis_names[i][0] == 0) break;
			printf("Display %s\r\n", dis_names[i]);
			printf("  (Current) %d x %d x %d (%d deg / %d Hz)\r\n", dis_states[i].width, dis_states[i].height, dis_states[i].depth, dis_states[i].orientation * 90, dis_states[i].freq);
			for (int j = 0; j < dis_modes_count[i]; j++) {
				printf("  %d x %d x %d (%d Hz)\r\n", dis_modes[i][j].width, dis_modes[i][j].height, dis_modes[i][j].depth, dis_modes[i][j].freq);
			}
		}
	}
	*/
}

extern "C" {

	DLL_EXPORT int __cdecl dis_count() {
		return dis_names_count;
	}

	DLL_EXPORT bool __cdecl dis_info(uint32_t id, dis_rect* current, dis_rect* maximum) {
		if (id >= DIS_MAX_DISPLAYS)
			return false;

		if (dis_names[id][0] == 0)
			return false;

		if (current != 0) {
			current->x = dis_states[id].x;
			current->y = dis_states[id].y;
			current->width = dis_states[id].width;
			current->height = dis_states[id].height;
			current->orientation = dis_states[id].orientation;
		}

		if (maximum != 0) {
			maximum->x = dis_states[id].x;
			maximum->y = dis_states[id].y;
			maximum->orientation = dis_states[id].orientation;
			maximum->width = maximum->height = 0;
			for (int i = 0; i < dis_modes_count[id]; i++) {
				if ((maximum->width < dis_modes[id][i].width) ||
					(maximum->height < dis_modes[id][i].height)
				) {
					maximum->width = dis_modes[id][i].width;
					maximum->height = dis_modes[id][i].height;
				}
			}
		}

		return true;
	}

	DLL_EXPORT void __cdecl dis_onchange(dis_change_func cb) {
		dis_change = cb;
		if (cb != 0) {
			slSingleLock(&dis_lock);
			cb();
			slSingleUnlock(&dis_lock);
		}
	}

	DLL_EXPORT void __cdecl dis_restore() {
		ChangeDisplaySettings(0,0);
	}

	DLL_EXPORT bool __cdecl dis_fullscreen(uint32_t id, uint32_t width, uint32_t height, uint32_t depth, uint32_t orientation) {
		if (id >= DIS_MAX_DISPLAYS)
			return false;

		if (dis_names[id][0] == 0)
			return false;

		DDEVMODE dm;
		ZeroMemory( &dm, sizeof(dm) );
		dm.dmSize = sizeof(dm);
		dm.dmPelsWidth = width;
		dm.dmPelsHeight = height;
		dm.dmBitsPerPel = depth;
		dm.dmDisplayOrientation = orientation;
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_ORIENTATION;
		LONG res = ChangeDisplaySettingsEx(dis_names[id], (DEVMODE*)&dm, 0, 0, 0);
		if (res == DISP_CHANGE_SUCCESSFUL) {
			return true;
		}
		dis_status(dis_names[id], res);

		if (res == DISP_CHANGE_BADMODE) {
			dm.dmFields &= ~DM_BITSPERPEL;
			res = ChangeDisplaySettingsEx(dis_names[id], (DEVMODE*)&dm, 0, 0, 0);
			if (res == DISP_CHANGE_SUCCESSFUL) {
				return true;
			}
			dis_status(dis_names[id], res);
		}

		return false;
	}

}

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

			// Monitor Plug-n-play detect
			DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

			ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
			NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
			NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
			NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_MONITOR;

			hDeviceNotify = RegisterDeviceNotification(hWnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

			// Session change detect (RDP/VNC)
			WTSRegisterSessionNotification(hWnd, NOTIFY_FOR_ALL_SESSIONS);

			slSingleUnlock(&dis_lock);

			//printf("Session: ?, %d\r\n", IsCurrentSessionRemoteable() ? 1 : 0);
			dis_update(true);

			break;

		case WM_CLOSE:
			WTSUnRegisterSessionNotification(hWnd);
			UnregisterDeviceNotification(hDeviceNotify);
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			SetEvent(hExitEvent);
			break;

		case WM_DISPLAYCHANGE:
			dis_update();
			break;

		case WM_DEVICECHANGE: {
			//PDEV_BROADCAST_DEVICEINTERFACE b = (PDEV_BROADCAST_DEVICEINTERFACE) lParam;
			char strBuff[256];

			// Подключение
			// Changed, Arrival
			// Отключение
			// Remove, Changed

			switch (wParam) {
				case DBT_DEVICEARRIVAL:
					// при открытии ноута и при подключении монитора
					//printf("DeviceArrival\r\n");
					dis_update();
					break;
				case DBT_DEVICEREMOVECOMPLETE:
					// при закрытии ноута и при отключении монитора
					//printf("DeviceRemoveComplete\r\n");
					dis_update();
					break;
				case DBT_DEVNODES_CHANGED:
					// при изменении списка мониторов
					//printf("DeviceNodesChanged\r\n");
					dis_update();
					break;
				default:
					break;
			}
			break;
		}

		case WM_WTSSESSION_CHANGE: {
			switch (wParam) {
				case WTS_CONSOLE_CONNECT:
				case WTS_REMOTE_CONNECT:
					//printf("Session: %d, %d\r\n", wParam, IsCurrentSessionRemoteable() ? 1 : 0);
					dis_update();
					break;
			}
			break;
		}

		default:
			break;
	}
	return DefWindowProc(hWnd,msg,wParam,lParam);
}

uint32_t threadProc(void* userData) {
	const char wndClass[] = "modDisplay@events";
	const char appName[] = "modDisplay";

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
