#include "stdafx.h"
#include "util.h"
#include "AsfSourceDealer.h"


//Callback class ,can recevie the event
class CMonitorAsfNetworkSinkStatusCallBack : public IWMStatusCallback
{
public:
	HRESULT STDMETHODCALLTYPE OnStatus(
		WMT_STATUS        Status,
		HRESULT           hr,
		WMT_ATTR_DATATYPE dwType,
		BYTE              *pValue,
		void              *pvContext
		)
	{
		if (WMT_CLIENT_CONNECT == Status)
		{
			//there is a client connected the server
			WM_CLIENT_PROPERTIES  * lpClient = (WM_CLIENT_PROPERTIES  *)pValue;
			//if the ip existed in our authenticated list,then nothing to do ,else close this 
		}
		return S_OK;
	}
};

//
//
//void Trace(const char* format, ...)
//{
//	char lBuf[MAX_PATH] = { 0 };
//	va_list args;
//	va_start(args, format);
//	vsprintf_s(lBuf, format, args);
//	va_end(args);
//	OutputDebugStringA(lBuf);
//}

bool CAsfSourceDealer::DealWithVideoSample(const double ndbTime,
	const char *pBuffer,
	const long nBufferLen)
{
	if (pBuffer == NULL || nBufferLen == 0 || VideoType() == nullptr|| !mpWriter )
		return false;

	
	VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)VideoType()->pbFormat;
	//Push each video frame to the ip:port
	HRESULT hr = S_OK;
	CComPtr<INSSBuffer> mpWmVideoSample;
	if (!mpWmVideoSample)
	{
		//First time allocate the buffer
		hr = mpWriter->AllocateSample(nBufferLen, &mpWmVideoSample);
	}

	//Allocate memory failed
	if (!mpWmVideoSample)
		return false;

	//Copy memory
	PBYTE pDest;
	mpWmVideoSample->GetBuffer(&pDest);
	memcpy(pDest, pBuffer, nBufferLen);
	mpWmVideoSample->SetLength(nBufferLen);
	
	//Write memory
	hr = mpWriter->WriteSample(mnVideoStreamNO, mdbVideoFrameTime, 0, mpWmVideoSample);
	mdbVideoFrameTime += pVih->AvgTimePerFrame;
	mpWmVideoSample.Release();
	return SUCCEEDED(hr);
}


bool CAsfSourceDealer::DealWithAudioSample(const double ndbTime,
	const char *pBuffer,
	const long nBufferLen)
{
	if (pBuffer == NULL || nBufferLen == 0 || AudioType() == NULL || !mpWriter )
		return false;
	WAVEFORMATEX *pVih = (WAVEFORMATEX*)AudioType()->pbFormat;
	LONGLONG lnAuduiDuration = (LONGLONG)10000000 / (pVih->nAvgBytesPerSec / nBufferLen);
	//Push each Audio frame to the ip:port
	HRESULT hr = S_OK;
	CComPtr<INSSBuffer> mpWmAudioSample;
	if (!mpWmAudioSample)
		hr = mpWriter->AllocateSample(nBufferLen, &mpWmAudioSample);

	//Allocate memory failed
	if (!mpWmAudioSample)
		return false;

	//Copy memory
	PBYTE pDest;
	mpWmAudioSample->GetBuffer(&pDest);
	memcpy(pDest, pBuffer, nBufferLen);
	mpWmAudioSample->SetLength(nBufferLen);

	//Write memory
	hr = mpWriter->WriteSample(mnAudioStreamNO, mdbAudioFrameTime, 0, mpWmAudioSample);
	mdbAudioFrameTime += lnAuduiDuration;
	mpWmAudioSample.Release();
	/*Trace("video%lld -audio%lld -differ %lld", (long long)mdbVideoFrameTime,
		(long long)mdbAudioFrameTime,
		(long long)mdbVideoFrameTime - (long long)mdbAudioFrameTime);*/
	return SUCCEEDED(hr);
}

//Init the writer 
HRESULT CAsfSourceDealer::InitWriter()
{
	HRESULT hr = S_OK;


	CComPtr<IWMRegisterCallback> pNetSinkCallback;
	//CMonitorAsfNetworkSinkStatusCallBack lCallback;
	SUCCESS_HR(hr = WMCreateWriterNetworkSink(&mpNetSink));

	SUCCESS_HR(hr = mpNetSink->Open(&mdwPort));

	SUCCESS_HR(hr = mpWriter->QueryInterface(IID_PPV_ARGS(&mpWriterAdvanced)));
	SUCCESS_HR(hr = mpWriterAdvanced->AddSink(mpNetSink));
	SUCCESS_HR(hr = mpNetSink->QueryInterface(IID_PPV_ARGS(&pNetSinkCallback)));
	SUCCESS_HR(hr = mpWriterAdvanced->SetLiveSource(TRUE));
	SUCCESS_HR(hr = mpWriterAdvanced->SetSyncTolerance(3000));
	//SUCCESS_HR(pNetSinkCallback->Advise(&lCallback,(void*)pNetSink));
	SUCCESS_HR(hr = ConfigureProfile());
	mdbVideoFrameTime = 0;
	mdbAudioFrameTime = 0;
	return hr;
}


//Config the writer 
HRESULT CAsfSourceDealer::ConfigureProfile()
{
	//Set input
	HRESULT hr = S_OK;
	CComPtr<IWMInputMediaProps> pInputProps = NULL;
	//CComPtr<IWMInputMediaProps> pAudioProps = NULL;
	//CComPtr<IWMInputMediaProps> pVideoProps = NULL;
	// pWriter->SetProfileByID(WMProfile_V80_256Video);

	CComPtr<IWMProfileManager> pManager;
	CComPtr<IWMProfile> pProfile;
	CComPtr<IWMStreamConfig> pConfig;
	SUCCESS_HR(hr = WMCreateProfileManager(&pManager));
	SUCCESS_HR(hr = pManager->LoadProfileByID(mnProfileID,&pProfile));
	SUCCESS_HR(hr = pProfile->GetStream(0, &pConfig));
	SUCCESS_HR(hr = pConfig->SetBufferWindow(2000));
	SUCCESS_HR(hr = pProfile->ReconfigStream(pConfig));
	SUCCESS_HR(hr = mpWriter->SetProfile(pProfile));
	DWORD   cInputs;

	hr = mpWriter->GetInputCount(&cInputs);

	GUID                guidInputType;
	DWORD dwAudioInput = 0, dwVideoInput = 0;

	for (DWORD i = 0; i < cInputs; i++)
	{

		hr = mpWriter->GetInputProps(i, &pInputProps);
		hr = pInputProps->GetType(&guidInputType);

		if (guidInputType == WMMEDIATYPE_Audio && AudioType())
		{
			// pAudioProps = pInputProps;
			mnAudioStreamNO = i;
			pInputProps->SetMediaType((WM_MEDIA_TYPE*)AudioType());
			hr = mpWriter->SetInputProps(i, pInputProps);
		}

		else if (guidInputType == WMMEDIATYPE_Video && VideoType())
		{
			// pVideoProps = pInputProps;
			mnVideoStreamNO = i;

			pInputProps->SetMediaType((WM_MEDIA_TYPE*)VideoType());
			hr = mpWriter->SetInputProps(i, pInputProps);
		}

		pInputProps.Release();
	}

	return hr;
}


bool CAsfSourceDealer::DealerBeginningThread()
{
	
	//Init Writer
	HRESULT hr = InitWriter();
	
	//Begin Writting
	SUCCESS_HR(hr = mpWriter->BeginWriting());
	//PostMessage(mhEventWnd, WM_CPATURENOTIFY, WPARAM_CAPTURE_STARTED, SUCCEEDED(hr) ? 0 : 1);
	return SUCCEEDED(hr);
}
bool  CAsfSourceDealer::DealerEndThread()
{
	if (!mpWriter || !mpWriterAdvanced || !mpNetSink)
		return false;

	//Try to close the port
	HRESULT hr = S_OK;
	SUCCESS_HR(hr = mpWriter->EndWriting());
	SUCCESS_HR(hr = mpWriterAdvanced->RemoveSink(mpNetSink));
	SUCCESS_HR(hr = mpNetSink->Close());
	//PostMessage(mhEventWnd, WM_CPATURENOTIFY, WPARAM_CAPTUREND, SUCCEEDED(hr) ? 0 : hr);
	return SUCCEEDED(hr);

}
