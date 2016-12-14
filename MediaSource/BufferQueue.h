#pragma once



//Buffer Queue
class BufferQueue
{
private:
	int mnFront; //Start Point
	int mnRear;   //End Point
	char* mpBuffer = nullptr;
	int mnElementSize;
	int mnElementCount;

private:
	int CircularlyAdd(int n)
	{
		return (n + 1) % mnElementCount;
	}
public:
	BufferQueue(int nEleSize, int nEleCount)
	{
		mnFront = -1;
		mnRear = -1;
		mnElementSize = nEleSize;
		mnElementCount = nEleCount;
		mpBuffer = (char*)malloc(nEleSize*nEleCount);
	}
	~BufferQueue()
	{
		if (mpBuffer != nullptr)
			free(mpBuffer);
	}
	bool IsEmpty()
	{
		return mnFront == -1 && mnRear == -1;
	}
	bool IsFull()
	{
		return (mnRear + 1) % mnElementCount == mnFront;
	}
	int Push(double ndbTime, const char* npBuffer, int nLen)
	{
		//check the buffer len
		if (nLen + sizeof(double) != mnElementSize)
			return 0;

		if (IsEmpty())
		{
			mnFront = mnRear = 0;
		}
		else if (IsFull())
		{
			mnFront = CircularlyAdd(mnFront);
			mnRear  = CircularlyAdd(mnRear);
		}
		else
		{
			mnRear = CircularlyAdd(mnRear);
		}
		//Push 
		memcpy(mpBuffer + mnRear*mnElementSize, &ndbTime, sizeof(double));
		memcpy(mpBuffer + mnRear*mnElementSize + sizeof(double), npBuffer, nLen);

		return nLen;
	}
	int Pop(double& ndbTime, char* & npBuffer, int nLen)
	{
		//check if the queue is empty
		if (IsEmpty())
			return 0;

		//check the buffer len
		if (nLen + sizeof(double) != mnElementSize)
			return 0;


		//Pop
		memcpy(&ndbTime, mpBuffer + mnFront*mnElementSize, sizeof(double));
		memcpy(npBuffer, mpBuffer + mnFront*mnElementSize + sizeof(double), mnElementSize - sizeof(double));

		//Only one element
		if (mnRear == mnFront)
		{
			mnRear = mnFront = -1;
		}
		else
			mnFront = CircularlyAdd(mnFront);

		return nLen;
	}

	int GetItemSize()
	{
		return mnElementSize - sizeof(double);
	}

};
