#pragma once
#include "dshow.h"
#include "qedit.h"
#include <list>
#include "Sample.h"
#include "Splitter.h"

typedef bool (*VideoLogoCallback)(PVOID ,PBYTE &); 

template <bool VideoType,int MonitorSample>
class SampleGrabberCallback : public ISampleGrabberCB
{
	
private:
	  CRITICAL_SECTION   m_critsec;	
	  DWORD mLen;	
	  std::list<sSample*>  mvecSmaple;
	  double mbPreSampleTime ; //sec
	  double mbPreMonitorSampleTime ; //sec
	 
	  LONGLONG mnCount;
	  CRITICAL_SECTION   m_critsecMoniter;
	  std::list<sSample*>  mvecMoniterSmaple;

	  CRITICAL_SECTION   m_critsecRec;
	  bool mbRecord;
	  
	  VideoLogoCallback mpFun;
	  PVOID mpContex;
public:
	void SetContextAndCallBack(VideoLogoCallback npFun,PVOID npContext)
	{
		mpContex = npContext;
		mpFun = npFun;
	}
	void Remove()
	{
		EnterCriticalSection(&m_critsec);		
		std::list<sSample*>::iterator lIte = mvecSmaple.begin();
		for(; lIte != mvecSmaple.end(); lIte++)
		{
			//delete (*lIte);
			(*lIte)->Release();			
		}
		mvecSmaple.clear();
		LeaveCriticalSection(&m_critsec);
		mbPreSampleTime = 0;
		mLen = 0;
	}
	SampleGrabberCallback()
	{
		InitializeCriticalSection(&m_critsec);	
		InitializeCriticalSection(&m_critsecMoniter);
		InitializeCriticalSection(&m_critsecRec);
		mLen=0;
		mbPreSampleTime = 0;
		mbRecord = false;
		mnCount = 0;
	
	}
	~SampleGrabberCallback()
	{
		DeleteCriticalSection(&m_critsec);
		DeleteCriticalSection(&m_critsecMoniter);
		DeleteCriticalSection(&m_critsecRec);
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
        OutputDebugString(L"SampleCB");
		int n = pSample->GetSize();
		return S_OK;
    }

    STDMETHODIMP BufferCB(double Time, BYTE *pBuffer, long BufferLen)
    {      
		//OutputDebugString(L"BufferVidoCB");
		//
	    //
	
		if (VideoType != false )
		{
			if(mpFun)
				mpFun(mpContex,pBuffer);
			////Write image to moniter list
			
			//if (mnCount % 20 == 0)
			{			
				// 1s to 100 nano second  1 = 10e7
				sSample *ls = new sSample(pBuffer, BufferLen, Time,mbPreMonitorSampleTime == 0? 0: abs(Time-mbPreMonitorSampleTime)*10000000);
				mbPreMonitorSampleTime = Time;
				EnterCriticalSection(&m_critsecMoniter);
				if(mvecMoniterSmaple.size() == MonitorSample)
				{
					sSample *lTemp = *mvecMoniterSmaple.begin();
					mvecMoniterSmaple.pop_front();
					if(lTemp)
						lTemp->Release();
				}
				//ls->AddRef();
				mvecMoniterSmaple.push_back(ls);
				LeaveCriticalSection(&m_critsecMoniter);
			}
			mnCount++;
			
			if (mbRecord == false)
				return S_OK;

			sSample *lsRecord = new sSample(pBuffer,BufferLen,Time,mbPreSampleTime == 0? 0: abs(Time-mbPreSampleTime)*10000000);	
			mbPreSampleTime = Time;
			EnterCriticalSection(&m_critsec);			
			mvecSmaple.push_back(lsRecord);	
			LeaveCriticalSection(&m_critsec);
		}
		else
		{
			//audio 1s 2frames
			sSample *ls = new sSample(pBuffer, BufferLen, Time, mbPreMonitorSampleTime == 0? 0: abs(Time-mbPreMonitorSampleTime)*10000000);
			mbPreMonitorSampleTime = Time;
			EnterCriticalSection(&m_critsecMoniter);
			if(mvecMoniterSmaple.size() == MonitorSample)
			{
					sSample *lTemp = *mvecMoniterSmaple.begin();
					mvecMoniterSmaple.pop_front();
					if(lTemp)
						lTemp->Release();
			}		
			mvecMoniterSmaple.push_back(ls);
			LeaveCriticalSection(&m_critsecMoniter);

			if (mbRecord == false)
				return S_OK;

			sSample *lsRecord = new sSample(pBuffer, BufferLen, mbPreSampleTime == 0? 0: abs(Time-mbPreSampleTime)*10000000, 0);
			mbPreSampleTime = Time;
			EnterCriticalSection(&m_critsec);			
			mvecSmaple.push_back(lsRecord);	
			LeaveCriticalSection(&m_critsec);
		}

		
		return S_OK;
    }

	
	bool GetSampleBuffer(sSample* & ls)
	{
		bool bRet = false;		
		
		EnterCriticalSection(&m_critsec);		
		if(mvecSmaple.size() > 0)	
		{
			/*WCHAR lBuf[20] = {0};
			wsprintf(lBuf,L"sample %d",mvecSmaple.size());
			OutputDebugString(lBuf);
			std::list<sSample*>::iterator lIte = mvecSmaple.begin();*/
			ls = *mvecSmaple.begin();
			mvecSmaple.pop_front();
		
			bRet = true;
		}
		LeaveCriticalSection(&m_critsec);
		return bRet;
	}
	bool GetMonitorSampleBuffer(sSample* & ls)
	{
		bool bRet = false;

		EnterCriticalSection(&m_critsecMoniter);
		if (mvecMoniterSmaple.size() > 0)
		{
			
			ls = *mvecMoniterSmaple.begin();
			mvecMoniterSmaple.pop_front();

			bRet = true;
		}
		LeaveCriticalSection(&m_critsecMoniter);
		return bRet;
	}
	bool GetMonitorAllSampleBuffer( std::list<sSample*> & nvecAllSample)
	{
		bool bRet = false;
		nvecAllSample.clear();

		EnterCriticalSection(&m_critsecMoniter);
		if (mvecMoniterSmaple.size() == MonitorSample)
		{			
			nvecAllSample.insert(nvecAllSample.begin(),mvecMoniterSmaple.begin(),mvecMoniterSmaple.end());
			mvecMoniterSmaple.clear();
			bRet = true;
		}
		LeaveCriticalSection(&m_critsecMoniter);
		return bRet;
	}
	void StartRecord()
	{
		EnterCriticalSection(&m_critsec);	
		mbRecord = true;
		mbPreSampleTime = 0;
		LeaveCriticalSection(&m_critsec);	
	}
	void StopRecord()
	{
		EnterCriticalSection(&m_critsec);	
		mbRecord = false;
		mbPreSampleTime = 0;
		LeaveCriticalSection(&m_critsec);	
	}
};

