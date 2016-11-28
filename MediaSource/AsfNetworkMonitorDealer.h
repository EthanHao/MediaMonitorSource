#pragma once
#include "ISampleDealer.h"



//Network Stream Monitor
//
class  CAsfNetworkMonitorDealer : public ISampleDealer
{
public:
	virtual bool DealVideoSample(const double ndbTime, const BYTE *pBuffer, const long nBufferLen);
	virtual bool DealAudioSample(const double ndbTime, const BYTE *pBuffer, const long nBufferLen);
	virtual HRESULT DealBeginningThread();
	virtual HRESULT DealEndThread(); //Stop can be call in other thread , might cause deadlock
public:
	CAsfNetworkMonitorDealer(const AM_MEDIA_TYPE* nVideoType, const AM_MEDIA_TYPE* nAudioType, DWORD ndwPort, REFGUID nProfileID) :
		ISampleDealer(nVideoType, nAudioType)
	{
		mdwPort = ndwPort;
		mnAudioStreamNO = 0;
		mnVideoStreamNO = 1;
		mnProfileID = nProfileID;
		WMCreateWriter(NULL, &mpWriter);
		mdbVideoFrameTime = 0;
		mdbVideoFrameTime = 0;
		
	}
	~CAsfNetworkMonitorDealer()
	{
		if (mpWriter != NULL)
			mpWriter.Release();
	}
private:
	DWORD mdwPort;  //Port
	CComPtr<IWMWriter> mpWriter; //writer 
	CComPtr<IWMWriterNetworkSink> mpNetSink;
	CComPtr<IWMWriterAdvanced> mpWriterAdvanced;
	GUID mnProfileID;  //Profile ID, decide the quality of the stream
	DWORD mnAudioStreamNO;
	DWORD mnVideoStreamNO;
	//CComPtr<INSSBuffer> mpWmVideoSample;
	//CComPtr<INSSBuffer> mpWmAudioSample;
	QWORD mdbVideoFrameTime;
	QWORD mdbAudioFrameTime;
	
private:
	HRESULT ConfigureProfile();
	HRESULT InitWriter();
};
