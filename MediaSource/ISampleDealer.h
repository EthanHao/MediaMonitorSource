#pragma once
#include <atomic>
#include <thread>
#include <mutex>
#include "BufferQueue.h"
//Sample Dealer Base class
class  ISampleDealer
{
private:
	std::mutex    mMutexVideo;
	std::mutex    mMutexAudio;
	std::thread    mThread;
	std::unique_ptr<BufferQueue> mVideoBufferQueue ;
	std::unique_ptr<BufferQueue> mAudioBufferQueue;
	std::atomic<bool> mbExit = false;

protected:
    AM_MEDIA_TYPE* mVideoMediaType;
	AM_MEDIA_TYPE* mAudioMediaType;

public:
	ISampleDealer(const AM_MEDIA_TYPE*, const AM_MEDIA_TYPE*);

	virtual ~ISampleDealer();

	//not copyable and not copy-assignable
	ISampleDealer(const ISampleDealer&) = delete;
	ISampleDealer& operator = (const ISampleDealer&) = delete;

	bool PushSample(const bool nbVideo, 
		const double ndbTime, 
		const char * npBuffer,
		const long nBufferLen, 
		const AM_MEDIA_TYPE* npMedia);
    int FetchVideoSample(double& ndbTime, char* & npBuffer, int nLen) const;
	int FetchAudioSample(double& ndbTime, char* & npBuffer, int nLen) const;

private:
	bool IsExit();
	bool Stop();
	bool Start();
	void  SampleCallbakcFunction(ISampleDealer* lpParam);

	
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

