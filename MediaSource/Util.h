#pragma once
#include <string>

#define WM_GRAPHNOTIFY  WM_APP+1
#define WM_CPATURENOTIFY  WM_APP+2
#define WM_MONITORNOTIFY  WM_APP+3


enum eWparamMonitor
{
	WPARAM_MONITOR_STARTED = 0,
	WPARAM_MONITOR,
	WPARAM_MONITOR_AUDIO,
	WPARAM_MONITOREND
};


#define SUCCESS_HR(x) if(SUCCEEDED(hr)) x;
#define BREAK_HR(x) if(!SUCCEEDED(x)) break;

HRESULT CopyAttribute(IMFAttributes *pSrc, IMFAttributes *pDest, const GUID& key);
std::wstring Subtype2String(const GUID & nSubtype);
GUID String2Subtype(const std::wstring & nsString);
