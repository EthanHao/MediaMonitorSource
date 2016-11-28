#pragma once



//Buffer Queue
class BufferQueue
{
private:
	int mnStart; //Start Point
	int mnEnd;   //End Point
	char* mpBuffer = nullptr;
	int mnElementSize;
	int mnElementCount;
	int mnCount = 0;
private:
	int CircualAdd(int n)
	{
		//Pop
		if (n == mnElementCount - 1)
			return 0;
		else
			return ++n;
	}
public:
	BufferQueue(int nEleSize, int nEleCount)
	{
		mnStart = 0;
		mnEnd = 0;
		mnElementSize = nEleSize;
		mnElementCount = nEleCount;
		mpBuffer = (char*)malloc(nEleSize*nEleCount);
	}
	~BufferQueue()
	{
		if (mpBuffer != NULL)
			free(mpBuffer);
	}
	int Push(double ndbTime, const char* npBuffer, int nLen)
	{
		//check the buffer len
		if (nLen + sizeof(double) != mnElementSize)
			return 0;

		//Push 
		memcpy(mpBuffer + mnEnd*mnElementSize, &ndbTime, sizeof(double));
		memcpy(mpBuffer + mnEnd*mnElementSize + sizeof(double), npBuffer, nLen);
		mnEnd = CircualAdd(mnEnd);
		//check if the start equal mnstart that mean we lost a single item
		if (mnEnd == mnStart)
			mnStart = CircualAdd(mnStart);
		mnCount++;
		return nLen;
	}
	int Pop(double& ndbTime,  char* & npBuffer, int nLen)
	{
		//check if the queue is empty
		if (mnStart == mnEnd)
			return 0;

		//check the buffer len
		if (nLen + sizeof(double) != mnElementSize)
			return 0;

		//Pop
		memcpy(&ndbTime, mpBuffer + mnStart*mnElementSize, sizeof(double));
		memcpy(npBuffer, mpBuffer + mnStart*mnElementSize + sizeof(double), mnElementSize - sizeof(double));
		mnStart = CircualAdd(mnStart);
		mnCount--;
		return nLen;
	}

	int GetItemSize()
	{
		return mnElementSize - sizeof(double);
	}
	bool IsFull()
	{
		return mnCount == mnElementCount;
	}
};
