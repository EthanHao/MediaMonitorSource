#pragma once
#include "windows.h"



class CBufferSample
{
private:
	BYTE * mpBuffer;
	DWORD  mLen;
	double mTime;
	double mDuration;

public:

	CBufferSample(const BYTE * nBuffer, DWORD nLen, double nTime, double nDuration)
	{
		mpBuffer = (BYTE*)malloc(nLen);
		memcpy(mpBuffer, nBuffer, nLen);
		mLen = nLen;
		mTime = nTime;
		mDuration = nDuration;
	}
	CBufferSample(const CBufferSample& other)
	{
		if (this == &other)
			return;

		mpBuffer = (BYTE*)malloc(other.mLen);
		memcpy(mpBuffer, other.mpBuffer, other.mLen);
		mLen = other.mLen;
		mTime = other.mTime;
		mDuration = other.mDuration;
	}
	CBufferSample& CBufferSample::operator=(const CBufferSample& other)
	{
		if (this == &other)
			return *this;

		mpBuffer = (BYTE*)malloc(other.mLen);
		memcpy(mpBuffer, other.mpBuffer, other.mLen);
		mLen = other.mLen;
		mTime = other.mTime;
		mDuration = other.mDuration;
		return *this;
	}

	//Let the complier generate the default move assignment function
	CBufferSample& CBufferSample::operator=(CBufferSample&& other) = default;

	//let the complier generate the default move consturct function
	CBufferSample(CBufferSample&& other) = default;

	~CBufferSample()
	{
		if (mpBuffer != NULL)
		{
			free(mpBuffer);
			mpBuffer = NULL;
		}
	}

	double Time() { return mTime; }
	DWORD Len() { return mLen; }
	BYTE* Buf() { return mpBuffer; }
	double Duration() { return mDuration; }

	
};
