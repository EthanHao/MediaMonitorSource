#include "stdafx.h"
#include  "MediaStreamManager.h"

#pragma comment(lib,"Strmiids.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "wmvcore.lib")
#pragma warning(disable:4995)
#pragma warning(disable:4244)
#pragma warning(disable:4996)



CMediaStreamManager::CMediaStreamManager()
{
	mhPreviewWnd = NULL;
	mStatus = eStatus::eNothing;

	//DirectShow sample callback 
	mpCallbackVideo = std::move(std::unique_ptr<CSampleGrabber<true>>(new CSampleGrabber<true>(mSplitterVideo)));
	mpCallbackAudio = std::move(std::unique_ptr<CSampleGrabber<false>>(new CSampleGrabber<false>(mSplitterAudio)));

}
CMediaStreamManager::~CMediaStreamManager()
{
	StopAll();
}


HRESULT CMediaStreamManager::InitFilter()
{
	HRESULT hr;

	// Create the filter graph
	hr = mpGraph.CoCreateInstance(CLSID_FilterGraph);
	if (FAILED(hr))
		return hr;

	// Create the capture graph builder
	hr = mpBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
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
	hr = mpGrabberVideoFilter.CoCreateInstance(CLSID_SampleGrabber);
	if (FAILED(hr))
		return hr;

	hr = mpGrabberVideoFilter->QueryInterface(IID_ISampleGrabber, (void**)(&mpGrabber));
	if (FAILED(hr))
		return hr;

	// Create the capture Grabber
	hr = mpGrabberAudioFilter.CoCreateInstance(CLSID_SampleGrabber);
	if (FAILED(hr))
		return hr;

	hr = mpGrabberAudioFilter->QueryInterface(IID_ISampleGrabber, (void**)(&mpGrabberAudio));
	if (FAILED(hr))
		return hr;


	hr = mpAudioNullRender.CoCreateInstance(CLSID_NullRenderer);
	if (FAILED(hr))
		return hr;

	return hr;
}

HRESULT CMediaStreamManager::UnInitFilter()
{

	mpAudioNullRender.Release();
	mpGrabberVideoFilter.Release();
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
	mpGrabberVideoFilter.Detach();
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

	return S_OK;
}



// Tear down everything downstream of a given filter
void CMediaStreamManager::RemoveDownstream(IBaseFilter *pf)
{
	IPin *pP = 0, *pTo = 0;
	ULONG u;
	IEnumPins *pins = NULL;
	PIN_INFO pininfo;

	if (!pf)
		return;

	HRESULT hr = pf->EnumPins(&pins);
	pins->Reset();

	while (hr == NOERROR)
	{
		hr = pins->Next(1, &pP, &u);
		if (hr == S_OK && pP)
		{
			pP->ConnectedTo(&pTo);
			if (pTo)
			{
				hr = pTo->QueryPinInfo(&pininfo);
				if (hr == NOERROR)
				{
					if (pininfo.dir == PINDIR_INPUT)
					{
						RemoveDownstream(pininfo.pFilter);
						if (!mpGraph == false)
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

	if (pins)
		pins->Release();
}


HRESULT CMediaStreamManager::TearDownGraph()
// Tear down everything downstream of the capture filters, so we can build
// a different capture graph.  Notice that we never destroy the capture filters
// and WDM filters upstream of them, because then all the capture settings
// we've set would be lost.
//
{

	if (!mpVW == false)
	{
		// stop drawing in our window, or we may get wierd repaint effects
		mpVW->put_Owner(NULL);
		mpVW->put_Visible(OAFALSE);
	}

	// destroy the graph downstream of our capture filters
	if (!mpVCap == false)
		RemoveDownstream(mpVCap);
	if (!mpACap == false)
		RemoveDownstream(mpACap);


	//All Detach
	return S_OK;

}

HRESULT CMediaStreamManager::BindVideoDevice(const std::wstring& nsVideoDeviceName)
{
	CComPtr<IMoniker> lpMoniker;
	CComPtr<IBindCtx> pBindCtx;
	HRESULT hr = S_FALSE;
	do
	{
		BREAK_HR(hr = CreateBindCtx(0, &pBindCtx));
		ULONG chEaten = 0;
		std::wstring lDisName;
		if (mSetVideoDevice.find(nsVideoDeviceName.c_str()) == mSetVideoDevice.end())
			lDisName = mSetVideoDevice.begin()->second;
		else
			lDisName = mSetVideoDevice[nsVideoDeviceName.c_str()];
		BREAK_HR(hr = MkParseDisplayName(pBindCtx, lDisName.c_str(), &chEaten, &lpMoniker));
		BREAK_HR(hr = lpMoniker->BindToObject(0, 0, IID_PPV_ARGS(&mpVCap)));
	} while (false);
	return hr;
}

HRESULT CMediaStreamManager::BindAudioDevice(const std::wstring& nsAudioDeviceName)
{
	CComPtr<IMoniker> lpMoniker;
	CComPtr<IBindCtx> pBindCtx;
	HRESULT hr = S_FALSE;
	do
	{
		BREAK_HR(hr = CreateBindCtx(0, &pBindCtx));
		ULONG chEaten = 0;
		std::wstring lDisName;
		if (mSetAudioDevice.find(nsAudioDeviceName.c_str()) == mSetAudioDevice.end())
			lDisName = mSetAudioDevice.begin()->second;
		else
			lDisName = mSetAudioDevice[nsAudioDeviceName.c_str()];
		BREAK_HR(hr = MkParseDisplayName(pBindCtx, lDisName.c_str(), &chEaten, &lpMoniker));
		BREAK_HR(hr = lpMoniker->BindToObject(0, 0, IID_PPV_ARGS(&mpACap)));
	} while (false);
	return hr;
}

/*Using RGB32 colorspace and 640*480 as the default video device capability
*/
HRESULT CMediaStreamManager::ConfigVideoDevice(const std::wstring& nsVideoDevCap)
{
	bool lbVideoTypeSet = false;
	CComPtr<IAMStreamConfig> lpVsc;
	HRESULT hr = S_FALSE;
	if (nsVideoDevCap.empty())
		return hr;

	do
	{
		BREAK_HR(hr = mpBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Video,
			mpVCap,
			IID_PPV_ARGS(&lpVsc)));

		//Get Device Capability
		int lnX = 640;
		int lnY = 480;
		GUID lSubtypeGuid = MEDIASUBTYPE_RGB24;
		if (!nsVideoDevCap.empty())
		{
			WCHAR lsSubType[50] = { 0 };
			swscanf(nsVideoDevCap.c_str(), L"%s %d %d", lsSubType, &lnX, &lnY);
			lSubtypeGuid = String2Subtype(lsSubType);
		}

		int iFmt, iSize, i;
		BYTE          *bbuf = NULL;
		hr = lpVsc->GetNumberOfCapabilities(&iFmt, &iSize);
		if (SUCCEEDED(hr))
		{
			//malloc memory
			bbuf = (BYTE *)malloc(iSize);

			//check
			for (i = 0; i < iFmt; i++)
			{
				AM_MEDIA_TYPE *pmt;
				hr = lpVsc->GetStreamCaps(i, &pmt, bbuf);
				if (SUCCEEDED(hr))
				{
					VIDEO_STREAM_CONFIG_CAPS * lpSCC = (VIDEO_STREAM_CONFIG_CAPS*)bbuf;
					if (pmt->formattype == FORMAT_VideoInfo &&
						//IsEqualGUID(pmt->subtype, lSubtypeGuid) &&
						lpSCC->InputSize.cx == lnX &&
						lpSCC->InputSize.cy == lnY)
					{
						hr = lpVsc->SetFormat(pmt);
						mvVideoType = std::move(std::shared_ptr<CMediaType>(new CMediaType(pmt)));
						CMediaType::DeleteMediaType(pmt);
						lbVideoTypeSet = true;
						break;
					}
					else
						CMediaType::DeleteMediaType(pmt);
				}
			}

			//free 
			if (bbuf)
				free(bbuf);
		}

	} while (false);

	return lbVideoTypeSet ? hr : S_FALSE;
}

/**Using PCM format as a default
*/
HRESULT CMediaStreamManager::ConfigAudioDevice()
{
	HRESULT hr = S_FALSE;
	bool lbAudioTypeSet = false;

	do
	{
		//Choose Audio Stream Config
		CComPtr<IAMStreamConfig> lpAsc;
		BREAK_HR(hr = mpBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Audio,
			mpACap,
			IID_PPV_ARGS(&lpAsc)));

		int iFmt, iSize, i;
		BYTE          *bbuf = NULL;
		hr = lpAsc->GetNumberOfCapabilities(&iFmt, &iSize);
		if (SUCCEEDED(hr))
		{
			bbuf = (BYTE *)malloc(iSize);
			for (i = 0; i < iFmt; i++)
			{
				AM_MEDIA_TYPE *pmt;
				hr = lpAsc->GetStreamCaps(i, &pmt, bbuf);
				if (SUCCEEDED(hr))
				{
					if (/*(pmt->formattype == FORMAT_AudioInfo) && */(IsEqualGUID(pmt->subtype, MEDIASUBTYPE_PCM)))
					{
						hr = lpAsc->SetFormat(pmt);
						mvAudioType = std::move(std::shared_ptr<CMediaType>(new CMediaType(pmt)));
						CMediaType::DeleteMediaType(pmt);
						lbAudioTypeSet = true;
						break;
					}
					CMediaType::DeleteMediaType(pmt);
				}
			}

			//free buffer
			if (bbuf)
				free(bbuf);
		}

	} while (false);

	return lbAudioTypeSet ? hr : S_FALSE;
}
//start record
HRESULT CMediaStreamManager::StartPreview(const std::wstring& nsVideoDeviceName,
	const std::wstring& nsVideoDevCap,
	const std::wstring& nsAudioDeviceName,
	const HWND hPreview)
{
	if (mSetVideoDevice.empty() || mSetAudioDevice.empty())
		return S_FALSE;

	if ((mStatus & eStatus::ePreviewing) != 0)
		StopAll();


	HRESULT hr;
	//AM_MEDIA_TYPE lpMediaType;
	//AM_MEDIA_TYPE lpAudioType ;
	//Choose Video Device --> IMoniker
	//Choose Audio Device --> IMoniker
	//Build Graph
	//Run
	do
	{

		//Find Video Device Object
		BREAK_HR(hr = BindVideoDevice(nsVideoDeviceName));

		//Find Audio Device Object
		BREAK_HR(hr = BindAudioDevice(nsAudioDeviceName));

		BREAK_HR(hr = InitFilter());

		BREAK_HR(hr = mpBuilder->SetFiltergraph(mpGraph));

		//Add Video source Filter
		BREAK_HR(hr = mpGraph->AddFilter(mpVCap, NULL));
		//Add audio source filter
		BREAK_HR(hr = mpGraph->AddFilter(mpACap, NULL));


		// we use this interface to set the frame rate and image format
		BREAK_HR(hr == ConfigVideoDevice(nsVideoDevCap));
		
		// we use this interface to set the frame rate and image format
		BREAK_HR(hr == ConfigAudioDevice());
		

		//add video Grabber
		{
			BREAK_HR(hr = mpGraph->AddFilter(mpGrabberVideoFilter, L"Video Sample Grabber"));
			BREAK_HR(hr = mpGrabber->SetMediaType(mvVideoType->GetAMediaType()));
			BREAK_HR(hr = mpGrabber->SetCallback(mpCallbackVideo.get(), 1));
			BREAK_HR(hr = mpGrabber->SetBufferSamples(FALSE));

		}
		//add Audio Grabber
		{
			BREAK_HR(hr = mpGraph->AddFilter(mpGrabberAudioFilter, L"Audio Sample Grabber"));
			BREAK_HR(hr = mpGrabberAudio->SetMediaType(mvAudioType->GetAMediaType()));
			BREAK_HR(hr = mpGrabberAudio->SetCallback(mpCallbackAudio.get(), 1));
			BREAK_HR(hr = mpGrabberAudio->SetBufferSamples(FALSE));

		}

		//add video render
		//BREAK_HR(hr = mpGraph->AddFilter(mpVideoNullRender, L"Video Renderer"));
		//add Audio render
		BREAK_HR(hr = mpGraph->AddFilter(mpAudioNullRender, L"Audio Renderer"));

		//Render Video Stream
		BREAK_HR(hr = mpBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
			NULL,
			mpVCap,
			mpGrabberVideoFilter,
			nullptr));

		BREAK_HR(hr = SetupVideoWindow(hPreview));

		//Render Audio Stream
		BREAK_HR(hr = mpBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
			NULL,
			mpACap,
			mpGrabberAudioFilter,
			mpAudioNullRender));

		// run the graph
		if (!mpMC == false)
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
		
	} while (false);

	if ((mStatus & eStatus::ePreviewing) == 0)
	{
		//Cl
		UnInitFilter();
	}
	
	return hr;
}


HRESULT CMediaStreamManager::SetupVideoWindow(HWND hWnd)
{
	HRESULT hr;

	// Set the video window to be a child of the main window
	hr = mpVW->put_Owner((OAHWND)hWnd);
	if (FAILED(hr))
		return hr;

	// Set video window style
	hr = mpVW->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
	if (FAILED(hr))
		return hr;


	hr = ResizeVideoWindow(hWnd);
	if (FAILED(hr))
		return hr;

	// Make the video window visible, now that it is properly positioned
	hr = mpVW->put_Visible(OATRUE);
	if (FAILED(hr))
		return hr;

	//store the preview wnd
	mhPreviewWnd = hWnd;

	return hr;
}
HRESULT CMediaStreamManager::ResizeVideoWindow(HWND hWnd)
{
	if (!mpMC)
		return S_FALSE;

	RECT rc;
	// Make the preview video fill our window
	GetClientRect(hWnd, &rc);
	return mpVW->SetWindowPosition(0, 0, rc.right, rc.bottom);
	 
}

//Stop record
HRESULT CMediaStreamManager::StopAll()
{


	//Stop Network Monitor
	StopAsfMonitor();
	//Stop Direct show
	if (!mpMC == false)
		mpMC->StopWhenReady();

	// Stop receiving events
	if (!mpME == false)
		mpME->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);

	// Relinquish ownership (IMPORTANT!) of the video window.
	// Failing to call put_Owner can lead to assert failures within
	// the video renderer, as it still assumes that it has a valid
	// parent window.
	if (!mpVW == false)
	{
		mpVW->put_Visible(OAFALSE);
		mpVW->put_Owner(NULL);
	}

	TearDownGraph();
	UnInitFilter();
	mStatus = eStatus::eNothing;



	return S_OK;
}

//Show Devece Property
HRESULT CMediaStreamManager::DeviceProperty()
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
bool CMediaStreamManager::EnumDevice(const bool nbIsVedeo)
{
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = pCreateDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
	if (hr != NOERROR)
	{
		//ErrorMessage(TEXT("Error Creating Device Enumerator"));
		return false;
	}

	CComPtr<IEnumMoniker> pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(nbIsVedeo ? CLSID_VideoInputDeviceCategory : CLSID_AudioInputDeviceCategory,
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
	while (pEm->Next(1, &pM, 0) == S_OK)
	{
		CComPtr<IPropertyBag> pBag;
		std::wstring ls;
		if (S_OK == pM->BindToStorage(0, 0, IID_PPV_ARGS(&pBag)))
		{
			VARIANT varName;
			VariantInit(&varName);
			pBag->Read(L"FriendlyName", &varName, 0);
			ls = varName.bstrVal;

			VariantClear(&varName);

			LPOLESTR lsDisName;
			pM->GetDisplayName(0, 0, &lsDisName);
			std::wstring lsDisstring = OLE2CW(lsDisName);
			CoTaskMemFree(lsDisName);

			if (nbIsVedeo != false)
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
					nbIsVedeo ? &MEDIATYPE_Video : &MEDIATYPE_Audio,
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
							CMediaType::DeleteMediaType(pmt);
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
		} while (true);

		pM.Release();
		pM.Detach();
	}

	return true;
}

//Start Asf Monitor
HRESULT CMediaStreamManager::StartAsfMonitor(const int nPort, REFGUID nProfileID)
{
	//ConfigVideo
	//ConfigAudio
	//Start Work Thread
	if ((mStatus & eStatus::ePreviewing) == 0)
		return S_FALSE;

	if ((mStatus&eStatus::eMonitoringNetwork) != 0)
		return S_FALSE;


	mpAsfSoure = std::move( std::unique_ptr<CAsfSourceDealer> (new CAsfSourceDealer(mvVideoType, 
		mvAudioType,
		nPort,
		nProfileID)));
	//New failed
	if (mpAsfSoure == nullptr)
		return S_FALSE;


	HRESULT hr = mpAsfSoure->Start();
	if (!SUCCEEDED(hr))
	{
		mSplitterVideo.RemoveDealer(mpAsfSoure.get());
		mSplitterAudio.RemoveDealer(mpAsfSoure.get());
	}
	else
	{
		mStatus |= eStatus::eMonitoringNetwork;
		mSplitterVideo.RegisterDealer(mpAsfSoure.get());
		mSplitterAudio.RegisterDealer(mpAsfSoure.get());
	}
	return  hr;
}

//Stop Asf Monitor
HRESULT CMediaStreamManager::StopAsfMonitor()
{
	//QuitWorkThread
	//Flush to Disk
	HRESULT hr = S_OK;
	if (mpAsfSoure  && (mStatus&eStatus::eMonitoringNetwork) != 0)
	{
		hr = mpAsfSoure->Stop();
		if (SUCCEEDED(hr))
		{
			mSplitterVideo.RemoveDealer(mpAsfSoure.get());
			mSplitterAudio.RemoveDealer(mpAsfSoure.get());
			mpAsfSoure = nullptr;
			mStatus &= ~eStatus::eMonitoringNetwork;
		}
	}
	return S_OK;
}


