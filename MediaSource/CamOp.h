#pragma once




#include "ISampleDealer.h"
#include "SampleGrabber.h"

enum eStatus
{
	eNothing = 0,
	ePreviewing = 1,
	eMonitoringNetwork = 2,
	eRendering = 4,
};


class CCamera
{
public:
	std::map<std::wstring,std::wstring> mSetVideoDevice;
	std::map<std::wstring, std::set<std::wstring>> mVideoDeviceCapbility;
	std::map<std::wstring,std::wstring> mSetAudioDevice;
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
	CComPtr<IBaseFilter> mVideoNullRender = nullptr;
	CComPtr<IBaseFilter> mpAudioNullRender = nullptr;
	UINT32 mStatus;

private:
	std::unique_ptr<AM_MEDIA_TYPE> mvVideoType;
	std::unique_ptr<AM_MEDIA_TYPE> mvAudioType;
	HWND mhPreviewWnd  = nullptr; //Show wnd
	
	//Splitter
	CSplitter mSplitterVideo;
	CSplitter mSplitterAudio;

	//DirectShow sample callback 
	CSampleGrabber<true>* mCallbackVideo;
	CSampleGrabber<false>* mCallbackAudio;

private:
	//TearDown Graph
	HRESULT TearDownGraph();
	void RemoveDownstream(IBaseFilter *pf);

   //Init Filter
	HRESULT InitFilter();
	HRESULT CCamera::UnInitFilter();



public:

	//Constructor
	CCamera();
	~CCamera();

	//enum device
	bool EnumDevice(const bool nbIsVedeo);

	//The main entrance function 
	HRESULT StartPreview(const std::wstring& nsVideoDeviceName,
		const std::wstring& nsVideoDevCap,
		const std::wstring& nsAudioDeviceName,
		const int& nPort,
		REFGUID nProfileID,
		const HWND hPreview);

	//Stop record
	HRESULT StopAll();

	//Show Devece Property
	HRESULT DeviceProperty();




	

};
