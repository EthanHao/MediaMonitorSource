#include "stdafx.h"
#include "Splitter.h"


bool CSplitter::RegisterDealer(const ISampleDealer * npDeal)
{
	std::lock_guard<std::mutex> lock(mMutex);
	if (!mListDealer)
	{
		std::unique_ptr<std::list<const ISampleDealer*>> lTemp(new std::list<const ISampleDealer*>());
		mListDealer = std::move(lTemp);
	}
	mListDealer->push_back(npDeal);
	return true;
}

bool CSplitter::RemoveDealer(const ISampleDealer * npDeal)
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
	const long nBufferLen,
	const AM_MEDIA_TYPE* npMedia)
{
	std::lock_guard<std::mutex> lock(mMutex);
	if(mListDealer)
		for(auto const & p : *mListDealer)
			p->PushSample(nbVideo,ndbTime, npBuffer,nBufferLen,npMedia);
	return true;
}