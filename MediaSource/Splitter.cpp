#include "stdafx.h"
#include "Splitter.h"


bool CSplitter::RegisterDealer( ISampleDealer * const npDeal)
{
	std::lock_guard<std::mutex> lock(mMutex);
	if (!mListDealer)
	{
		std::unique_ptr<std::list<ISampleDealer*>> lTemp(new std::list<ISampleDealer*>());
		mListDealer = std::move(lTemp);
	}
	mListDealer->push_back(npDeal);
	return true;
}

bool CSplitter::RemoveDealer( ISampleDealer * const npDeal)
{
	std::lock_guard<std::mutex> lock(mMutex);
	if(mListDealer)
		mListDealer->remove(npDeal);
	return true;
}

bool CSplitter::RemoveAll()
{
	std::lock_guard<std::mutex> lock(mMutex);
	if (mListDealer) 
		mListDealer->clear();
	return true;
}

bool CSplitter::OnSample(const bool nbVideo,
	const double ndbTime,
	const char *npBuffer,
	const long nBufferLen)
{
	std::lock_guard<std::mutex> lock(mMutex);
	if (mListDealer && mListDealer->size() > 0)
	{
		auto iter = mListDealer->begin();
		for (; iter != mListDealer->end(); iter++)
			(*iter)->PushSample(nbVideo, ndbTime, npBuffer, nBufferLen);
	}
	return true;
}