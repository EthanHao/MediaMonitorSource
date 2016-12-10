#pragma once
#include "ISampleDealer.h"



//Network Stream Monitor
//
class  CAsfSourceDealer : public ISampleDealer
{
public:
	virtual bool DealWithVideoSample(const double ndbTime,
		const char * npBuffer,
		const long nBufferLen) ;

	virtual bool DealWithAudioSample(const double ndbTime,
		const char * npBuffer,
		const long nBufferLen) ;

	virtual bool DealerBeginningThread() ;

	virtual bool DealerEndThread() ;

public:
	CAsfSourceDealer(const std::shared_ptr<CMediaType>& nVideoType, 
		const std::shared_ptr<CMediaType>& nAudioType,
		DWORD ndwPort, 
		REFGUID nProfileID) :
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
	~CAsfSourceDealer()
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

	QWORD mdbVideoFrameTime;
	QWORD mdbAudioFrameTime;
	
private:
	HRESULT ConfigureProfile();
	HRESULT InitWriter();
};
