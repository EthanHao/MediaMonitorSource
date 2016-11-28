#pragma once
#include "dshow.h"
#include "qedit.h"
#include <list>
#include "Sample.h"
#include "Splitter.h"

typedef bool(*VideoWaterMarkCallback)(PVOID, PBYTE &, int);

template <bool VideoType>
class CSampleGrabber : public ISampleGrabberCB
{

private:
	AM_MEDIA_TYPE mStreamType; //The meta information for stream
	CSplitter& mSplitter;
	double mbPreSampleTime; //sec
	PVOID mpContex;
public:
	void SetContextAndCallBack(VideoWaterMarkCallback npFun, PVOID npContext)
	{
		mpContex = npContext;
	
	}
	
	CSampleGrabber(CSplitter & nSplitter):
		mSplitter(nSplitter)
	{
		mpWaterMarkFun = NULL;
		mpContex = NULL;
		mbPreSampleTime = 0;
		
	}
	
	// Fake referance counting.
	STDMETHODIMP_(ULONG) AddRef() { return 1; }
	STDMETHODIMP_(ULONG) Release() { return 2; }

	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
	{
		if (NULL == ppvObject) return E_POINTER;
		if (riid == __uuidof(IUnknown))
		{
			*ppvObject = static_cast<IUnknown*>(this);
			return S_OK;
		}
		if (riid == IID_ISampleGrabberCB)
		{
			*ppvObject = static_cast<ISampleGrabberCB*>(this);
			return S_OK;
		}
		return E_NOTIMPL;
	}

	STDMETHODIMP SampleCB(double Time, IMediaSample *pSample)
	{
		return S_OK;
	}

	STDMETHODIMP BufferCB(double ndbTime, BYTE *pBuffer, long nBufferLen)
	{
		mSplitter.OnSample(VideoType,ndbTime, pBuffer, nBufferLen, &mStreamType);
		return S_OK;
	}
};

