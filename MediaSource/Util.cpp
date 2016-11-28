#include "stdafx.h"
#include "util.h"

HRESULT CopyAttribute(IMFAttributes *pSrc, IMFAttributes *pDest, const GUID& key)
{
	PROPVARIANT var;
	PropVariantInit(&var);
	HRESULT hr = S_OK;
	hr = pSrc->GetItem(key, &var);
	if (SUCCEEDED(hr))
	{
		hr = pDest->SetItem(key, var);
	}
	PropVariantClear(&var);
	return hr;
}



std::wstring Subtype2String(const GUID & nSubtype)
{
	if (IsEqualGUID(nSubtype, MEDIASUBTYPE_RGB32))
		return L"RGB32";
	else
		return L"UNSURPORT";
}


GUID String2Subtype(const std::wstring & nsString)
{
	if (nsString.compare(L"RGB32") == 0)
		return MEDIASUBTYPE_RGB32;
	else
		return MEDIASUBTYPE_None;
}