#pragma once

class CMediaType
{
private:
	AM_MEDIA_TYPE * mpMediaType;
public:


	// general purpose function to delete a heap allocated AM_MEDIA_TYPE structure
	// which is useful when calling IEnumMediaTypes::Next as the interface
	// implementation allocates the structures which you must later delete
	// the format block may also be a pointer to an interface to release

	static void WINAPI DeleteMediaType(__inout_opt AM_MEDIA_TYPE *pmt)
	{
		// allow NULL pointers for coding simplicity

		if (pmt == NULL) {
			return;
		}

		FreeMediaType(*pmt);
		CoTaskMemFree((PVOID)pmt);
	}


	// this also comes in useful when using the IEnumMediaTypes interface so
	// that you can copy a media type, you can do nearly the same by creating
	// a CMediaType object but as soon as it goes out of scope the destructor
	// will delete the memory it allocated (this takes a copy of the memory)

	static AM_MEDIA_TYPE * WINAPI CreateMediaType(AM_MEDIA_TYPE const *pSrc)
	{
		//ASSERT(pSrc);
		if (pSrc == NULL)
			return NULL;

		// Allocate a block of memory for the media type

		AM_MEDIA_TYPE *pMediaType =
			(AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));

		if (pMediaType == NULL) {
			return NULL;
		}
		// Copy the variable length format block

		HRESULT hr = CopyMediaType(pMediaType, pSrc);
		if (FAILED(hr)) {
			CoTaskMemFree((PVOID)pMediaType);
			return NULL;
		}

		return pMediaType;
	}


	//  Copy 1 media type to another

	static HRESULT WINAPI CopyMediaType(__out AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource)
	{
		//  We'll leak if we copy onto one that already exists - there's one
		//  case we can check like that - copying to itself.
		if (pmtSource == pmtTarget || pmtSource->pbFormat == NULL)
			return S_FALSE;
		*pmtTarget = *pmtSource;
		if (pmtSource->cbFormat != 0) {
			//ASSERT(pmtSource->pbFormat != NULL);
			pmtTarget->pbFormat = (PBYTE)CoTaskMemAlloc(pmtSource->cbFormat);
			if (pmtTarget->pbFormat == NULL) {
				pmtTarget->cbFormat = 0;
				return E_OUTOFMEMORY;
			}
			else {
				CopyMemory((PVOID)pmtTarget->pbFormat, (PVOID)pmtSource->pbFormat,
					pmtTarget->cbFormat);
			}
		}
		if (pmtTarget->pUnk != NULL) {
			pmtTarget->pUnk->AddRef();
		}

		return S_OK;
	}

	//  Free an existing media type (ie free resources it holds)

	static void WINAPI FreeMediaType(__inout AM_MEDIA_TYPE& mt)
	{
		if (mt.cbFormat != 0) {
			CoTaskMemFree((PVOID)mt.pbFormat);

			// Strictly unnecessary but tidier
			mt.cbFormat = 0;
			mt.pbFormat = NULL;
		}
		if (mt.pUnk != NULL) {
			mt.pUnk->Release();
			mt.pUnk = NULL;
		}
	}

public:
	CMediaType(const AM_MEDIA_TYPE * npType)
	{
		//mpMediaType = nullptr;
		mpMediaType = CreateMediaType(npType);
	}
	~CMediaType()
	{
		if (mpMediaType)
			DeleteMediaType(mpMediaType);
	}

	AM_MEDIA_TYPE *GetAMediaType()
	{
		return mpMediaType;
	}


	//not copyable and not copy-assignable
	CMediaType(const CMediaType&) = delete;
	CMediaType& operator = (const CMediaType&) = delete;

	//moveable and move-assignable is default
	CMediaType(CMediaType&&) = default;
	CMediaType& operator = (CMediaType&&) = default;
};
