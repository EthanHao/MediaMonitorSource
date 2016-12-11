#pragma once
#include <atomic>
#include <thread>
#include <mutex>
#include "BufferQueue.h"
#include "MediaType.h"
//Sample Dealer Base class
class  ISampleDealer
{
private:
	std::mutex    mMutexVideo;
	std::mutex    mMutexAudio;
	std::thread   mThread;
	std::unique_ptr<BufferQueue> mVideoBufferQueue = nullptr;
	std::unique_ptr<BufferQueue> mAudioBufferQueue = nullptr;
	std::atomic<bool> mbExit = false;

protected:
	std::shared_ptr<CMediaType>  mVideoMediaType;
	std::shared_ptr<CMediaType>  mAudioMediaType;

public:
	ISampleDealer(const std::shared_ptr<CMediaType> & npVideoType,
		const std::shared_ptr<CMediaType> &npAudioType) :
		mVideoMediaType(npVideoType),
		mAudioMediaType(npAudioType)
	{
	}

	virtual ~ISampleDealer();

	//not copyable and not copy-assignable
	ISampleDealer(const ISampleDealer&) = delete;
	ISampleDealer& operator = (const ISampleDealer&) = delete;

	bool PushSample(const bool nbVideo,
		const double ndbTime,
		const char * npBuffer,
		const long nBufferLen);

	int FetchVideoSample(double& ndbTime, char* & npBuffer, const int nLen);

	int FetchAudioSample(double& ndbTime, char* & npBuffer, const int nLen);

	AM_MEDIA_TYPE * VideoType()
	{
		if (mVideoMediaType)
			return mVideoMediaType->GetAMediaType();
		return nullptr;
	}
	AM_MEDIA_TYPE * AudioType()
	{
		if (mAudioMediaType)
			return mAudioMediaType->GetAMediaType();
		return nullptr;
	}

	bool Stop();

	bool Start();
private:
	bool IsExit();
	
	static void  SampleCallbakcFunction(ISampleDealer* lpParam);

public:
	virtual bool DealWithVideoSample(const double ndbTime,
		const char * npBuffer,
		const long nBufferLen) = 0;

	virtual bool DealWithAudioSample(const double ndbTime,
		const char * npBuffer,
		const long nBufferLen) = 0;

	virtual bool DealerBeginningThread() = 0;

	virtual bool DealerEndThread() = 0;
};

