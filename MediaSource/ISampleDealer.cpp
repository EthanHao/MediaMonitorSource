#include "stdafx.h"
#include "ISampleDealer.h"
#define VIDEO_BUFFER_QUEUE_SIZE 10

void ISampleDealer::SampleCallbakcFunction(ISampleDealer* lpParam)
{
	ISampleDealer * lpDeal = lpParam;
    char* lpVideoBuffer = nullptr;
	char* lpAudioBuffer = nullptr;
	int lnLen = 0;
	int lnAudioLen = 0;
	double ldbTime = 0;
	//Call begin
	if (lpDeal) 
		lpDeal->DealerBeginningThread();

	//Call the function timerly
	while (lpDeal && !lpDeal->IsExit())
	{
		//Get video buffer from video buffer queue and then call abstract method of DealVideoSample
		if (!lpVideoBuffer )
		{
			lnLen = lpDeal->FetchVideoSample(ldbTime, lpVideoBuffer, 0);
			if (lnLen > 0)
				lpVideoBuffer = new (std::nothrow)char[lnLen];
		}

		if (lpVideoBuffer)
		{
			if (0 < lpDeal->FetchVideoSample(ldbTime, lpVideoBuffer, lnLen))
				lpDeal->DealWithVideoSample(ldbTime, lpVideoBuffer, lnLen);
		}


		//Get Audio buffer from Audio buffer queue and then call abstract method of DealAudioSample
		if (!lpAudioBuffer )
		{
			lnAudioLen = lpDeal->FetchAudioSample(ldbTime, lpVideoBuffer, 0);
			if (lnAudioLen > 0)
				lpAudioBuffer = new (std::nothrow)char[lnAudioLen];
		}

		if (lpAudioBuffer )
		{
			if (0 < lpDeal->FetchAudioSample(ldbTime, lpAudioBuffer, lnAudioLen))
				lpDeal->DealWithAudioSample(ldbTime,lpAudioBuffer, lnAudioLen);
		}

		Sleep(10);
	}

	//call the pure function to give a chance to call
	//Call End
	if (lpDeal) 
		lpDeal->DealerEndThread();

	//free memory
	if (lpVideoBuffer != nullptr)
		delete[] lpVideoBuffer;

	if (lpAudioBuffer != nullptr)
		delete[] lpAudioBuffer;

	return ;
}

bool ISampleDealer::Start()
{
	mbExit = false; 
	mThread = std::thread(&ISampleDealer::SampleCallbakcFunction, this);
	
	return mThread.joinable();
}

bool ISampleDealer::Stop()
{
	if (mThread.joinable())
	{
		mbExit = true;
		mThread.join();
		//mThread.detach();
		return true;
	}
	return false;
}


ISampleDealer::~ISampleDealer()
{
	//End thread
	Stop();
}


bool ISampleDealer::PushSample(const bool nbVideo,
	const  double ndbTime,
	const  char *npBuffer,
	const  long nBufferLen)
{
	//has quit
	if (mbExit != false)
		return false;

	//add this sample to the queue
	if (nbVideo)
	{
		std::lock_guard<std::mutex> lock(mMutexVideo);
		if (!mVideoBufferQueue)
		{
			//First call
			mVideoBufferQueue.reset(new BufferQueue(nBufferLen + sizeof(double), VIDEO_BUFFER_QUEUE_SIZE));
		}

		//Push to video buffer queue
		mVideoBufferQueue->Push(ndbTime, npBuffer, nBufferLen);
	}
	else
	{
		std::lock_guard<std::mutex> lock(mMutexAudio);
		if (!mAudioBufferQueue )
		{
			//First call
			mAudioBufferQueue.reset(new BufferQueue(nBufferLen + sizeof(double), VIDEO_BUFFER_QUEUE_SIZE));
		}

		//Push to video buffer queue
		mAudioBufferQueue->Push(ndbTime, npBuffer, nBufferLen);
	}
	return true;
}

//if nLen == 0 return the true buffer size
//return 0 == get nothing
//       >0 get something 
int ISampleDealer::FetchVideoSample(double& ndbTime,
	char* & npBuffer, 
	const int nLen) 
{
	std::lock_guard<std::mutex> lock(mMutexVideo);
	//if queue is null return 0
	if (!mVideoBufferQueue)
		return 0;

	//if nlen == 0 then return the true size
	if (nLen == 0)
		return mVideoBufferQueue->GetItemSize();

	//pop a frame
	return mVideoBufferQueue->Pop(ndbTime, npBuffer, nLen);

}
int ISampleDealer::FetchAudioSample(double& ndbTime, 
	char* & npBuffer,
	const int nLen) 
{
	std::lock_guard<std::mutex> lock(mMutexAudio);
	//if queue is null return 0
	if (!mAudioBufferQueue) 
		return 0;

	//if nlen == 0 then return the true size
	if (nLen == 0)
		return mAudioBufferQueue->GetItemSize();

	//pop a frame
	return mAudioBufferQueue->Pop(ndbTime, npBuffer, nLen);
}

bool ISampleDealer::IsExit()
{
	return mbExit;
}