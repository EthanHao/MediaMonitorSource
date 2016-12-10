#pragma once

#include "ISampleDealer.h"
#include "SampleGrabber.h"
#include "AsfSourceDealer.h"

enum eStatus
{
	eNothing = 0,
	ePreviewing = 1,
	eMonitoringNetwork = 2,
};


class CMediaStreamManager
{
private:
	std::map<std::wstring, std::wstring> mSetVideoDevice;
	std::map<std::wstring, std::set<std::wstring>> mVideoDeviceCapbility;
	std::map<std::wstring, std::wstring> mSetAudioDevice;
	std::map<std::wstring, std::set<std::wstring>> mAudioDeviceCapbility;
private:
	CComPtr<IBaseFilter> mpVCap = nullptr;
	CComPtr<IBaseFilter> mpACap = nullptr;
	CComPtr<IVideoWindow> mpVW = nullptr;
	CComPtr<IMediaControl> mpMC = nullptr;
	CComPtr<IMediaEventEx> mpME = nullptr;
	CComPtr<IGraphBuilder> mpGraph = nullptr;
	CComPtr<ICaptureGraphBuilder2> mpBuilder = nullptr;
	CComPtr<IBaseFilter> mpGrabberVideoFilter = nullptr;
	CComPtr<ISampleGrabber> mpGrabber = nullptr;
	CComPtr<IBaseFilter> mpGrabberAudioFilter = nullptr;
	CComPtr<ISampleGrabber> mpGrabberAudio = nullptr;
	CComPtr<IBaseFilter> mpVideoNullRender = nullptr;
	CComPtr<IBaseFilter> mpAudioNullRender = nullptr;
	UINT32 mStatus;

private:
	std::shared_ptr<CMediaType> mvVideoType;
	std::shared_ptr<CMediaType> mvAudioType;
	HWND mhPreviewWnd = nullptr; //Show wnd
	//Splitter
	CSplitter mSplitterVideo;
	CSplitter mSplitterAudio;

	//DirectShow sample callback 
	std::unique_ptr<CSampleGrabber<true>> mpCallbackVideo;
	std::unique_ptr<CSampleGrabber<false>> mpCallbackAudio;

	//Dealer
	std::unique_ptr<CAsfSourceDealer> mpAsfSoure;

private:
	//TearDown Graph
	HRESULT TearDownGraph();
	void RemoveDownstream(IBaseFilter *pf);

	//Init Filter
	HRESULT InitFilter();
	HRESULT UnInitFilter();

	//Device binding and configuration functions
	HRESULT BindVideoDevice(const std::wstring& nsVideoDeviceName);
	HRESULT BindAudioDevice(const std::wstring& nsAudioDeviceName);
	HRESULT ConfigVideoDevice(const std::wstring& nsVideoDevCap);
	HRESULT ConfigAudioDevice();

public:

	//Constructor
	CMediaStreamManager();
	~CMediaStreamManager();

	//enum device
	bool EnumDevice(const bool nbIsVedeo);

	//The main entrance function 
	HRESULT StartPreview(const std::wstring& nsVideoDeviceName,
		const std::wstring& nsVideoDevCap,
		const std::wstring& nsAudioDeviceName,
		const HWND hPreview);

	//Stop record
	HRESULT StopAll();

	//Show Devece Property
	HRESULT DeviceProperty();


	//Start Asf Monitor
	HRESULT StartAsfMonitor(const int nPort, 
		REFGUID nProfileID);
	
	//Stop asf monitor
	HRESULT StopAsfMonitor();
};

