#pragma once
#include "ISampleDealer.h"

//splitter class , we can split the stream into some numbers of stream
//each stream can deal sample separately
class  CSplitter
{
public:
	bool RegisterDealer(const ISampleDealer * npDeal);
	bool RemoveDealer(const ISampleDealer * npDeal);
	bool RemoveAll();
	bool OnSample(const bool nbVideo, 
		const double ndbTime, 
		const char *npBuffer, 
		const long nBufferLen);
	
	
private:
	std::mutex mMutex;
	std::unique_ptr<std::list<const ISampleDealer*>> mListDealer = nullptr;
};
