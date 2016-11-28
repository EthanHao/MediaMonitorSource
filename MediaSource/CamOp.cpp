#include "stdafx.h"
#include  "CamOp.h"
#include "objidl.h"
#include "dshow.h"
#include "mtype.h"
#include "Setting.h"
#include "util.h"
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "wmvcore.lib")
#pragma warning(disable:4995)
#pragma warning(disable:4244)
#pragma warning(disable:4996)

#ifdef _DEBUG
#pragma comment(lib, "strmbasd.lib")
#else
#pragma comment(lib, "strmbase.lib")
#endif

CCamera::CCamera()
{
	mvVideoType = NULL;
	mvAudioType = NULL;
	mhPreviewWnd = NULL;
	mStatus = eStatus::eNothing;

	//DirectShow sample callback 
	mCallbackVideo = new CSampleGrabber<true>(mSplitterVideo);
	mCallbackAudio = new CSampleGrabber<false>(mSplitterAudio);

	
}
CCamera::~CCamera()
{
	StopAll();
}


HRESULT CCamera::InitFilter()
{
	HRESULT hr;

	// Create the filter graph
	hr = mpGraph.CoCreateInstance (CLSID_FilterGraph);
	if (FAILED(hr))
		return hr;

	// Create the capture graph builder
	hr = mpBuilder.CoCreateInstance (CLSID_CaptureGraphBuilder2 );
	if (FAILED(hr))
		return hr;

	// Obtain interfaces for media control and Video Window
	hr = mpGraph->QueryInterface(IID_PPV_ARGS(&mpMC));
	if (FAILED(hr))
		return hr;

	hr = mpGraph->QueryInterface(IID_PPV_ARGS(&mpVW));
	if (FAILED(hr))
		return hr;

	hr = mpGraph->QueryInterface(IID_PPV_ARGS(&mpME));
	if (FAILED(hr))
		return hr;   

	// Create the capture Grabber
	hr = mpGrabberF.CoCreateInstance (CLSID_SampleGrabber );	
	if (FAILED(hr))
		return hr;

	hr = mpGrabberF->QueryInterface(IID_ISampleGrabber,(void**)(&mpGrabber));
	if (FAILED(hr))
		return hr;  

	// Create the capture Grabber
	hr = mpGrabberAudioFilter.CoCreateInstance (CLSID_SampleGrabber );	
	if (FAILED(hr))
		return hr;

	hr = mpGrabberAudioFilter->QueryInterface(IID_ISampleGrabber,(void**)(&mpGrabberAudio));
	if (FAILED(hr))
		return hr;  

	hr = mpRender.CoCreateInstance (CLSID_NullRenderer );
	if (FAILED(hr))
		return hr;

	hr = mpAudioNullRender.CoCreateInstance (CLSID_NullRenderer );
	if (FAILED(hr))
		return hr;

	return hr;
}

HRESULT CCamera::UnInitFilter()
{
	HRESULT hr = S_OK;

	mpAudioNullRender.Release();
	mpRender.Release();
	mpGrabberF.Release();
	mpGrabber.Release();
	mpME.Release();
	mpVW.Release();
	mpMC.Release();
	mpBuilder.Release();
	mpGraph.Release();
	mpGrabberAudioFilter.Release();
	mpGrabberAudio.Release();
	mpVCap.Release();
	mpACap.Release();

	mpAudioNullRender.Detach();
	mpRender.Detach();
	mpGrabberF.Detach();
	mpGrabber.Detach();
	mpME.Detach();
	mpVW.Detach();
	mpMC.Detach();
	mpBuilder.Detach();
	mpGraph.Detach();
	mpGrabberAudioFilter.Detach();
	mpGrabberAudio.Detach();
	mpVCap.Detach();
	mpACap.Detach();

	return hr;
}



// Tear down everything downstream of a given filter
void CCamera::RemoveDownstream(IBaseFilter *pf)
{
	IPin *pP=0, *pTo=0;
	ULONG u;
	IEnumPins *pins = NULL;
	PIN_INFO pininfo;

	if (!pf)
		return;

	HRESULT hr = pf->EnumPins(&pins);
	pins->Reset();

	while(hr == NOERROR)
	{
		hr = pins->Next(1, &pP, &u);
		if(hr == S_OK && pP)
		{
			pP->ConnectedTo(&pTo);
			if(pTo)
			{
				hr = pTo->QueryPinInfo(&pininfo);
				if(hr == NOERROR)
				{
					if(pininfo.dir == PINDIR_INPUT)
					{
						RemoveDownstream(pininfo.pFilter);
						if(!mpGraph == false)
						{
							mpGraph->Disconnect(pTo);
							mpGraph->Disconnect(pP);
							mpGraph->RemoveFilter(pininfo.pFilter);
						}
					}
					pininfo.pFilter->Release();
				}
				pTo->Release();
			}
			pP->Release();
		}
	}

	if(pins)
		pins->Release();
}


HRESULT CCamera::TearDownGraph()	
	// Tear down everything downstream of the capture filters, so we can build
	// a different capture graph.  Notice that we never destroy the capture filters
	// and WDM filters upstream of them, because then all the capture settings
	// we've set would be lost.
	//
{

	if(!mpVW == false)
	{
		// stop drawing in our window, or we may get wierd repaint effects
		mpVW->put_Owner(NULL);
		mpVW->put_Visible(OAFALSE);		
	}

	// destroy the graph downstream of our capture filters
	if(!mpVCap  == false)
		RemoveDownstream(mpVCap);
	if(!mpACap  == false)
		RemoveDownstream(mpACap);


	//All Detach
	return S_OK;

}
//start record
HRESULT CCamera::StartPreview(const std::wstring& nsVideoDeviceName,
	const std::wstring& nsVideoDevCap,
	const std::wstring& nsAudioDeviceName,
	const std::wstring& nsMonitorDir,
	const std::wstring& nsLogPng,
	const int& nPort,
	REFGUID nProfileID,
	const HWND hPrevice)
{
	if(mSetVideoDevice.empty() ||  mSetAudioDevice.empty())
		return S_FALSE;

	if ((mStatus & eStatus::ePreviewing) != 0)
		StopAll();

	//clear last time audio and video
	mCallbackVideo->SetContextAndCallBack(CCamera::VideoCallback,this);

    
	HRESULT hr;
	//AM_MEDIA_TYPE lpMediaType;
	//AM_MEDIA_TYPE lpAudioType ;
	//Choose Video Device --> IMoniker
	//Choose Audio Device --> IMoniker
	//Build Graph
	//Run
	do
	{
		
		{ //Find Video Device Object
			CComPtr<IMoniker> lpMoniker;
			CComPtr<IBindCtx> pBindCtx;
			hr=CreateBindCtx(0,&pBindCtx);
			ULONG chEaten=0;
			std::wstring lDisName;
			if(mSetVideoDevice.find(nsVideoDeviceName.c_str()) == mSetVideoDevice.end())
				lDisName = mSetVideoDevice.begin()->second;
			else
				lDisName = mSetVideoDevice[nsVideoDeviceName.c_str()];
			BREAK_HR(hr = MkParseDisplayName(pBindCtx, lDisName.c_str(), &chEaten, &lpMoniker));
			BREAK_HR(hr = lpMoniker->BindToObject(0, 0, IID_PPV_ARGS(&mpVCap)));
			
		}
		{ //Find Audio Device Object
 			CComPtr<IMoniker> lpMoniker;
			CComPtr<IBindCtx> pBindCtx;
			hr=CreateBindCtx(0,&pBindCtx);
			ULONG chEaten=0;
			std::wstring lDisName;
			if(mSetAudioDevice.find(nsAudioDeviceName.c_str()) == mSetAudioDevice.end())
				lDisName = mSetAudioDevice.begin()->second;
			else
				lDisName = mSetAudioDevice[nsAudioDeviceName.c_str()];
			BREAK_HR(hr = MkParseDisplayName(pBindCtx, lDisName.c_str(), &chEaten, &lpMoniker));
			BREAK_HR(hr = lpMoniker->BindToObject(0, 0, IID_PPV_ARGS(&mpACap)));
			
		}

		BREAK_HR(hr = InitFilter());
		BREAK_HR(hr = mpBuilder->SetFiltergraph(mpGraph));
		//Add Video source Filter
		BREAK_HR(hr = mpGraph->AddFilter(mpVCap, NULL));
		//Add audio source filter
		BREAK_HR(hr = mpGraph->AddFilter(mpACap, NULL));
		

		// we use this interface to set the frame rate and image format
		{
			bool lbVideoTypeSet = false;
			CComPtr<IAMStreamConfig> lpVsc;
			hr = mpBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, mpVCap,
				IID_PPV_ARGS(&lpVsc));

			if(hr != S_OK)
				break;

			//Get Device Capability
			int lnX = 640;
			int lnY = 480;
			GUID lSubtypeGuid = MEDIASUBTYPE_RGB32;
			if(!nsVideoDevCap.empty())
			{
				WCHAR lsSubType[50] = {0};
				swscanf(nsVideoDevCap.c_str(),L"%s %d %d",lsSubType,&lnX,&lnY);
				lSubtypeGuid = String2Subtype(lsSubType);
			}

			int iFmt,iSize,i;
			BYTE          *bbuf = NULL;
			hr = lpVsc->GetNumberOfCapabilities(&iFmt, &iSize);
			if (SUCCEEDED(hr))
			{
				bbuf = (BYTE *)malloc(iSize);
				for(i = 0; i < iFmt; i++)
				{
					AM_MEDIA_TYPE *pmt;
					hr = lpVsc->GetStreamCaps(i, &pmt, bbuf);
					if (SUCCEEDED(hr))
					{
						VIDEO_STREAM_CONFIG_CAPS * lpSCC = (VIDEO_STREAM_CONFIG_CAPS*)bbuf;
						if (pmt->formattype == FORMAT_VideoInfo && 
							IsEqualGUID(pmt->subtype, lSubtypeGuid) &&
							lpSCC->InputSize.cx == lnX && lpSCC->InputSize.cy == lnY)
						{    
							hr = lpVsc->SetFormat(pmt);	
							mvVideoType = CreateMediaType(pmt);
							DeleteMediaType(pmt);
							lbVideoTypeSet = true;
							break;
						}
						DeleteMediaType(pmt);
					}
				}
				if (bbuf)
					free(bbuf);
			}

			if(lbVideoTypeSet == false)
				break;
		}

		{//Choose Audio Stream Config
			bool lbAudioTypeSet = false;
			CComPtr<IAMStreamConfig> lpAsc;
			hr = mpBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Audio, mpACap,
				IID_PPV_ARGS(&lpAsc));

			if(hr != S_OK)
				break;
			int iFmt,iSize,i;
			BYTE          *bbuf = NULL;
			hr = lpAsc->GetNumberOfCapabilities(&iFmt, &iSize);
			if (SUCCEEDED(hr))
			{
				bbuf = (BYTE *)malloc(iSize);
				for(i = 0; i < iFmt; i++)
				{
					AM_MEDIA_TYPE *pmt;
					hr = lpAsc->GetStreamCaps(i, &pmt, bbuf);
					if (SUCCEEDED(hr))
					{
						if (/*(pmt->formattype == FORMAT_AudioInfo) && */(IsEqualGUID(pmt->subtype, MEDIASUBTYPE_PCM))) 
						{    
							hr = lpAsc->SetFormat(pmt);	
							mvAudioType = CreateMediaType(pmt);
							DeleteMediaType(pmt);
							lbAudioTypeSet = true;
							break;
						}
						DeleteMediaType(pmt);
					}
				}
				if (bbuf)
					free(bbuf);
			}
	
			if(lbAudioTypeSet == false)
				break;
		}


		///Create Water mark deal
		mpWaterMaker = new CWaterMarkFilter((const AM_MEDIA_TYPE*)mvVideoType, nsLogPng);
		

		//add video Grabber
		{
			BREAK_HR(hr = mpGraph->AddFilter(mpGrabberF, L"Video Sample Grabber"));
			BREAK_HR(hr = mpGrabber->SetMediaType(mvVideoType));
			BREAK_HR(hr = mpGrabber->SetCallback(mCallbackVideo, 1));
			BREAK_HR(hr = mpGrabber->SetBufferSamples(FALSE));
		
		}
		//add Audio Grabber
		{
			BREAK_HR(hr = mpGraph->AddFilter(mpGrabberAudioFilter, L"Audio Sample Grabber"));
			BREAK_HR(hr = mpGrabberAudio->SetMediaType(mvAudioType));
			BREAK_HR(hr = mpGrabberAudio->SetCallback(mCallbackAudio, 1));
			BREAK_HR(hr = mpGrabberAudio->SetBufferSamples(FALSE));
			
		}

		//add video render
		BREAK_HR(hr = mpGraph->AddFilter(mpRender, L"Renderer"));
		//add Audio render
		BREAK_HR(hr = mpGraph->AddFilter(mpAudioNullRender, L"Audio Renderer"));
		
		//Render Video Stream
		BREAK_HR(hr = mpBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
			NULL,
			mpVCap,
			mpGrabberF,
			mpRender));
		

		//Render Audio Stream
		BREAK_HR(hr = mpBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
			NULL,
			mpACap,
			mpGrabberAudioFilter,
			mpAudioNullRender));
			
		// run the graph
		if (!mpMC  == false)
		{
			hr = mpMC->Run();
			if (FAILED(hr))
			{
				// stop parts that ran
				mpMC->Stop();
			}
			else
			{				
				mStatus |= eStatus::ePreviewing;
			}
		} 
		break;
	}while(1);

	if ((mStatus & eStatus::ePreviewing)  == 0)
	{
		//Cl
		UnInitFilter();
	}
	else
	{
		//Start Render
		StartRender();

		//start monitor asf and local
		StartAsfMonitor(nPort, nProfileID);
	}
	
	
	return hr;
}

//Stop record
HRESULT CCamera::StopAll()
{

	//Stop Render
	StopRender();
		

	//Stop Network Monitor
	StopAsfMonitor();


	//Stop Direct show
	if(!mpMC == false)
		mpMC->StopWhenReady();

	// Stop receiving events
	if (!mpME == false)
		mpME->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);

	// Relinquish ownership (IMPORTANT!) of the video window.
	// Failing to call put_Owner can lead to assert failures within
	// the video renderer, as it still assumes that it has a valid
	// parent window.
	if(!mpVW == false)
	{
		mpVW->put_Visible(OAFALSE);
		mpVW->put_Owner(NULL);
	}

	TearDownGraph();
	UnInitFilter();
	mStatus  = eStatus::eNothing;
	if (mvVideoType)
	{
		FreeMediaType(*mvVideoType);
		mvVideoType = NULL;
	}
	if (mvAudioType)
	{
		FreeMediaType(*mvAudioType);
		mvAudioType = NULL;
	}


	return S_OK;
}

//Show Devece Property
HRESULT CCamera::DeviceProperty()
{
  CAUUID caGUID;
  CComPtr<ISpecifyPropertyPages> pISPP;
  HRESULT    hr;

  // We MUST have a device to display property pages for
  if (!mpVCap != false)
    return S_FALSE;

  //StopVideo();

  // get the ISpecifyPropertyPages interface
  hr = mpVCap->QueryInterface(IID_PPV_ARGS(&pISPP));
  if (SUCCEEDED(hr))
  {
	FILTER_INFO FilterInfo;
    hr = mpVCap->QueryFilterInfo(&FilterInfo); 
    CComPtr<IUnknown> pFilterUnk;
    mpVCap->QueryInterface(IID_PPV_ARGS(&pFilterUnk));

    // get the list of page Id's
    hr = pISPP->GetPages(&caGUID);   
    if (SUCCEEDED(hr))
    {
      OleCreatePropertyFrame(
        mhPreviewWnd,          // Parent window
        0, 0,                   // (Reserved)
        FilterInfo.achName,             // Caption for the dialog box
        1,                      // Number of objects (just the filter)
        &pFilterUnk,   // Array of object pointers. 
        caGUID.cElems,          // Number of property pages
        caGUID.pElems,          // Array of property page CLSIDs
        0,                      // Locale identifier
        0, NULL);               // Reserved
      CoTaskMemFree(caGUID.pElems);
    }
  }
  
	return S_OK;
}

//enum device
bool CCamera::EnumDevice(const bool nbIsVedeo)
{
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = pCreateDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
	if (hr != NOERROR)
	{
		//ErrorMessage(TEXT("Error Creating Device Enumerator"));
		return false;
	}

	CComPtr<IEnumMoniker> pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(nbIsVedeo ? CLSID_VideoInputDeviceCategory:CLSID_AudioInputDeviceCategory,
		&pEm,
		0);
	if (hr != NOERROR)
	{
		//ErrorMessage(TEXT("Sorry, you have no video capture hardware"));
		return false;
	}

	nbIsVedeo ? mSetVideoDevice.clear(), mVideoDeviceCapbility.clear() 
		: mSetAudioDevice.clear(), mAudioDeviceCapbility.clear();

	pEm->Reset();
	CComPtr<IMoniker> pM;
	while(pEm->Next(1, &pM, 0) == S_OK)
	{
		CComPtr<IPropertyBag> pBag;
		std::wstring ls;
		if(S_OK == pM->BindToStorage(0,0,IID_PPV_ARGS(&pBag)))
		{
			VARIANT varName;
			VariantInit(&varName);
			pBag->Read(L"FriendlyName",&varName,0);
			ls = varName.bstrVal;
		
			VariantClear(&varName);

			LPOLESTR lsDisName;
			pM->GetDisplayName(0,0,&lsDisName);
			std::wstring lsDisstring = OLE2CW(lsDisName);
			CoTaskMemFree(lsDisName);

			if(nbIsVedeo != false)
				mSetVideoDevice[ls] = lsDisstring;
			else
				mSetAudioDevice[ls] = lsDisstring;

		
		}	
		do
		{
			CComPtr<IBaseFilter> lpFilter;
			if (S_OK == pM->BindToObject(0, 0, IID_PPV_ARGS(&lpFilter)))
			{
				CComPtr<IAMStreamConfig> lpVsc;
				CComPtr<ICaptureGraphBuilder2> lpBuilder;

				// Create the capture graph builder
				hr = lpBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
				if (FAILED(hr))
					break;
				hr = lpBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
					nbIsVedeo?&MEDIATYPE_Video:&MEDIATYPE_Audio,
					lpFilter,
					IID_PPV_ARGS(&lpVsc));
				if (FAILED(hr))
					break;

				//Get Device Capability

				int iFmt, iSize, i;
				BYTE          *bbuf = NULL;
				std::set<std::wstring> lset;
				hr = lpVsc->GetNumberOfCapabilities(&iFmt, &iSize);
				if (SUCCEEDED(hr))
				{
					bbuf = (BYTE *)malloc(iSize);
					for (i = 0; i < iFmt; i++)
					{
						AM_MEDIA_TYPE *pmt;
						hr = lpVsc->GetStreamCaps(i, &pmt, bbuf);
						if (SUCCEEDED(hr))
						{
							if (nbIsVedeo)
							{
								VIDEO_STREAM_CONFIG_CAPS * lpSCC = (VIDEO_STREAM_CONFIG_CAPS*)bbuf;
								std::wstring lsTemp = Subtype2String(pmt->subtype);
								WCHAR buff[200];
								wsprintf(buff, L"%s(%d*%d)", lsTemp.c_str(), lpSCC->InputSize.cx, lpSCC->InputSize.cy);
								//std::string buffAsStdStr = buff;
								////lsTemp = lsTemp+L"(" + lpSCC->MinOutputSize + L"*" + lpSCC->MaxOutputSize + L")";
								
								lset.insert(buff);
							}
							else
							{
								AUDIO_STREAM_CONFIG_CAPS * lpSCC = (AUDIO_STREAM_CONFIG_CAPS*)bbuf;
								std::wstring lsTemp = Subtype2String(pmt->subtype);
							}
							DeleteMediaType(pmt);
						}
					}
					if (bbuf)
						free(bbuf);
				}

				if (nbIsVedeo != false)
					mVideoDeviceCapbility[ls] = lset;
				else
					mAudioDeviceCapbility[ls] = lset;
			}
			break;
		}while (true);

		pM.Release();
		pM.Detach();
	} 

	return true;
}



//Start Asf Monitor
HRESULT CCamera::StartAsfMonitor(const int nPort, REFGUID nProfileID)
{
	//ConfigVideo
	//ConfigAudio
	//Start Work Thread
	if ((mStatus & eStatus::ePreviewing) == 0)
		return S_FALSE;

	if ((mStatus&eStatus::eMonitoringNetwork) != 0)
		return S_FALSE;

	if (mpMp4NetworkDeal != NULL)
		return S_FALSE;

	mpMp4NetworkDeal = new CAsfNetworkMonitorDeal(mvVideoType, mvAudioType, nPort, nProfileID);
	//New failed
	if (mpMp4NetworkDeal == NULL)
		return S_FALSE;


	HRESULT hr = mpMp4NetworkDeal->Start();
	if(!SUCCEEDED(hr))
	{
		mSplitterVideo.RemoveDeal(mpMp4NetworkDeal);
		mSplitterAudio.RemoveDeal(mpMp4NetworkDeal);
		delete mpMp4NetworkDeal;
	}
	else
	{
		mStatus |= eStatus::eMonitoringNetwork;
		mSplitterVideo.RegisterDeal(mpMp4NetworkDeal);
		mSplitterAudio.RegisterDeal(mpMp4NetworkDeal);
	}
	return  hr;
}

//Stop Asf Monitor
HRESULT CCamera::StopAsfMonitor()
{
	//QuitWorkThread
	//Flush to Disk
	HRESULT hr = S_OK;
	if (mpMp4NetworkDeal != NULL && (mStatus&eStatus::eMonitoringNetwork) != 0)
	{
		hr = mpMp4NetworkDeal->Stop();
		if (SUCCEEDED(hr))
		{
			mSplitterVideo.RemoveDeal(mpMp4NetworkDeal);
			mSplitterAudio.RemoveDeal(mpMp4NetworkDeal);
			delete mpMp4NetworkDeal;
			mpMp4NetworkDeal = NULL;
			mStatus &= ~eStatus::eMonitoringNetwork;
		}
	}
	return S_OK;
}


