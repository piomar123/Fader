/* Fader by Piomar & Tenco
    piomar123[at]gmail.com
    http://ilovewinapi.blogspot.com/
    (C) 2017
    Please place info about author when copying. Thanks.
*/
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "comctl32")

#define WIN32_LEAN_AND_MEAN
#define WINVER 0x0502
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <cstdio>
#include <string>
#include "resource.h"
#include <winuser.h>
#define TMR_FADE 11
#define TMR_UNFADE 12
#define TMR_AUTODESTRUCTION 13
#define HREG_ROOT HKEY_CURRENT_USER
#define REGSUBKEY "SOFTWARE\\PioMarTenco\\Fader"

using namespace std;

LRESULT CALLBACK SaverProcedure(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ConfigureDlgProcedure(HWND, UINT, WPARAM, LPARAM);
void SaveToReg();
void ReadFromReg();

char szClassName[] = "FaderWindowClass";
HWND hwnd;
HBITMAP bitmap;
HDC backDC;
HKEY hKey;
HCURSOR hCursor;

BOOL mouseNoPos = true, fading = true, dataHideMouse = true, dataTurnMonitor = true;
int mouseX = 0, mouseY = 0, dataFadeInt = 50, dataUnfadeInt = 15, dataFadeStep = 1, dataUnfadeStep = 10;

int screenWidth     = GetSystemMetrics(SM_CXVIRTUALSCREEN),
    screenHeight    = GetSystemMetrics(SM_CYVIRTUALSCREEN),
    screenX         = GetSystemMetrics(SM_XVIRTUALSCREEN),
    screenY         = GetSystemMetrics(SM_YVIRTUALSCREEN),
    fadeStep        = 1;


void SaveToReg(){
    LONG result;
    result = RegCreateKeyEx(HREG_ROOT, REGSUBKEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if(result != ERROR_SUCCESS){
        MessageBox(hwnd, "Unable to open register key\nConfiguration is not saved!", "Error", MB_OK|MB_ICONERROR|MB_TOPMOST);
        return;
    }
    RegSetValueEx(hKey, "HideMouse",    0, REG_DWORD, (BYTE*)&dataHideMouse,    sizeof(DWORD));
    RegSetValueEx(hKey, "TurnMonitor",  0, REG_DWORD, (BYTE*)&dataTurnMonitor,  sizeof(DWORD));
    RegSetValueEx(hKey, "FadeInt",      0, REG_DWORD, (BYTE*)&dataFadeInt,      sizeof(DWORD));
    RegSetValueEx(hKey, "UnfadeInt",    0, REG_DWORD, (BYTE*)&dataUnfadeInt,    sizeof(DWORD));
    RegSetValueEx(hKey, "FadeStep",     0, REG_DWORD, (BYTE*)&dataFadeStep,     sizeof(DWORD));
    RegSetValueEx(hKey, "UnfadeStep",   0, REG_DWORD, (BYTE*)&dataUnfadeStep,   sizeof(DWORD));
}

void ReadFromReg(){
    DWORD createResult;
    LONG result;
    result = RegCreateKeyEx(HREG_ROOT, REGSUBKEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &createResult);
    if(result == ERROR_SUCCESS){
        switch(createResult){
            case REG_CREATED_NEW_KEY:
                SaveToReg();
            break;
            case REG_OPENED_EXISTING_KEY:{
                DWORD sizeInt = sizeof(int), sizeBool = sizeof(BOOL);
                RegQueryValueEx(hKey, "HideMouse",      0, NULL, (BYTE*)&dataHideMouse,    &sizeBool);
                RegQueryValueEx(hKey, "TurnMonitor",    0, NULL, (BYTE*)&dataTurnMonitor,  &sizeBool);
                RegQueryValueEx(hKey, "FadeInt",        0, NULL, (BYTE*)&dataFadeInt,       &sizeInt);
                RegQueryValueEx(hKey, "UnfadeInt",      0, NULL, (BYTE*)&dataUnfadeInt,     &sizeInt);
                RegQueryValueEx(hKey, "FadeStep",       0, NULL, (BYTE*)&dataFadeStep,      &sizeInt);
                RegQueryValueEx(hKey, "UnfadeStep",     0, NULL, (BYTE*)&dataUnfadeStep,    &sizeInt);
            }
            break;
            default:
                MessageBox(NULL, "Register key is neither opened nor created. Shit happened.\noO", "Unknown error", MB_OK|MB_ICONWARNING|MB_TOPMOST);
            break;
        }
    } else {
         MessageBox(NULL, "Unable to open register key\nUsing default values\nConfiguration won't be saved.", "Warning", MB_OK|MB_ICONWARNING|MB_TOPMOST);
    }
    RegCloseKey(hKey);
}


int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE, LPSTR lpszArgument, int nCmdShow){
    string argument = lpszArgument;

    if(argument.find("/p") != string::npos || argument.find("/P") != string::npos){
        //if preview do nothing
        return 0;
	}
    if(argument.find("/c") != string::npos || argument.find("/C") != string::npos){
        InitCommonControls();
        ReadFromReg();
        DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), NULL, ConfigureDlgProcedure);
		return 0;
    }
	if(argument.find("off") != string::npos){
        DefWindowProc(GetDesktopWindow(), WM_SYSCOMMAND, SC_MONITORPOWER, 2);
        return 0;
    }
	if(argument.find("/?") != string::npos || argument.find("help") != string::npos){
        MessageBox(NULL, "Fader by Piomar & Tenco\nmail: piomar123[at]gmail.com\n\nUsage:\nYou can use it as a typical screensaver with extension .scr\nor run with argument \"off\" to turn off display instantly.\n\n(C) 2017", "Fader help", MB_ICONINFORMATION);
        return 0;
    }

	ReadFromReg();
	hCursor = dataHideMouse ? NULL : LoadCursor(NULL, IDC_ARROW);

	WNDCLASSEX wincl;
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = SaverProcedure;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wincl.hCursor = hCursor;
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    if(!RegisterClassEx(&wincl)) {
        char errorMsg[200];
        DWORD errorCode = GetLastError();
        sprintf_s(errorMsg, "Cannot register window class: error %ld", errorCode);
        MessageBox(NULL, errorMsg, "Error", MB_ICONERROR);
        return 1;
    }

    hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        szClassName,
        "Fader",
        WS_POPUP,
        screenX,     screenY,
        screenWidth, screenHeight,
        HWND_DESKTOP, NULL, hThisInstance, NULL
    );

    SetLayeredWindowAttributes(hwnd, 0, 1, LWA_ALPHA);
    SetTimer(hwnd, TMR_FADE, dataFadeInt, NULL);
    ShowWindow(hwnd, nCmdShow);

    MSG messages;
    while(GetMessage(&messages, NULL, 0, 0)){
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    return messages.wParam;
}


LRESULT CALLBACK SaverProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    switch (message){
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_SETCURSOR:
            SetCursor(hCursor);
            return TRUE;
        case WM_TIMER:
             switch(wParam){
                case TMR_FADE:
                    fadeStep += dataFadeStep;
                    if(fadeStep >= 255) {
                        SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
                        KillTimer(hwnd, TMR_FADE);
                        if(dataTurnMonitor){
                            DefWindowProc(hwnd, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
                            SetTimer(hwnd, TMR_AUTODESTRUCTION, 500, NULL);
                        }
                    } else {
                        SetLayeredWindowAttributes(hwnd, 0, fadeStep, LWA_ALPHA);
                    }
                    break;
                case TMR_UNFADE:
                    fadeStep -= dataUnfadeStep;
                    if(fadeStep <= 15) {
                         KillTimer(hwnd, TMR_UNFADE);
                         DestroyWindow(hwnd);
                    } else {
                        SetLayeredWindowAttributes(hwnd, 0, fadeStep, LWA_ALPHA);
                    }
                    break;
                case TMR_AUTODESTRUCTION:
                     KillTimer(hwnd, TMR_AUTODESTRUCTION);
                     DestroyWindow(hwnd);
                    break;
                default:
                    break;
             }
             break;

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_KEYDOWN:
        case WM_MOUSEMOVE:
        case WM_MOUSEWHEEL:
            if(fading){
                if(message == WM_MOUSEMOVE){
						if(mouseNoPos) {
                           mouseNoPos = false;
                           mouseX = GET_X_LPARAM(lParam);
                           mouseY = GET_Y_LPARAM(lParam);
                           return true;
						}
                        if(mouseX == GET_X_LPARAM(lParam) && mouseY == GET_Y_LPARAM(lParam)) break;
                 }
				fading = false;
				KillTimer(hwnd, TMR_FADE);
				dataHideMouse = false;
				DefWindowProc(hwnd, WM_SYSCOMMAND, SC_MONITORPOWER, -1);
				SetTimer(hwnd, TMR_UNFADE, dataUnfadeInt, NULL);
	            LONG exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
				exstyle |= WS_EX_TRANSPARENT;
				SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle);
				// TODO: activate previous window
            }
            break;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

BOOL CALLBACK ConfigureDlgProcedure(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
        case WM_INITDIALOG:
            CheckDlgButton(hdlg, IDC_HIDEMOUSE,     dataHideMouse);
            CheckDlgButton(hdlg, IDC_TURNMONITOR,   dataTurnMonitor);
            SetDlgItemInt (hdlg, IDC_FADETIME,      dataFadeInt, true);
            SetDlgItemInt (hdlg, IDC_FADESTEPS,     dataFadeStep, true);
            SetDlgItemInt (hdlg, IDC_UNFADETIME,    dataUnfadeInt, true);
            SetDlgItemInt (hdlg, IDC_UNFADESTEPS,   dataUnfadeStep, true);
            break;
        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case IDOK:
                    dataFadeInt     = GetDlgItemInt(hdlg, IDC_FADETIME, NULL, true);
                    dataFadeStep    = GetDlgItemInt(hdlg, IDC_FADESTEPS, NULL, true);
                    dataUnfadeInt   = GetDlgItemInt(hdlg, IDC_UNFADETIME, NULL, true);
                    dataUnfadeStep  = GetDlgItemInt(hdlg, IDC_UNFADESTEPS, NULL, true);
                    dataHideMouse   = IsDlgButtonChecked(hdlg, IDC_HIDEMOUSE);
                    dataTurnMonitor = IsDlgButtonChecked(hdlg, IDC_TURNMONITOR);
                    SaveToReg();
                    EndDialog(hdlg, true);
                    break;
                case IDCANCEL:
                    EndDialog(hdlg, false);
                    break;
                default:
                    break;
            }
        default:
            return FALSE;
    }
    return TRUE;
}

