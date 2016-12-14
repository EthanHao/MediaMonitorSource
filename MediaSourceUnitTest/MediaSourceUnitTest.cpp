// MediaSourceUnitTest.cpp : Defines the entry point for the console application.
//


#include <gtest\gtest.h>
#include "..\MediaSource\BufferQueue.h"

TEST(BufferQueue, PushWrongParameters)
{
	BufferQueue lQueue(100, 10);
	double ldbTime = 0.001;
	char * pBuffer = new char[100];
	int lRet = lQueue.Push(ldbTime, pBuffer, 100);
	EXPECT_EQ(0, lRet);


	delete[] pBuffer;
}

TEST(BufferQueue, PopWrongParameters)
{
	BufferQueue lQueue(100, 10);
	double ldbTime = 0.001;
	char * pBuffer = new char[100];
	int lRet = lQueue.Push(ldbTime, pBuffer, 100 - sizeof(double));
	EXPECT_EQ(lRet, 100 - sizeof(double));

	double ldbPopTime = 0.0f;
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100);
	EXPECT_EQ(lRet,0);

	delete[] pBuffer;
}

TEST(BufferQueue, PushSuccessWithOneElementQueue)
{
	BufferQueue lQueue(100, 1);
	double ldbTime = 0.1;
	char * pBuffer = new char[100];
	int lRet = lQueue.Push(ldbTime, pBuffer, 100-sizeof(double));
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//pop one
	double ldbPopTime = 0.0f;
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 - sizeof(double));
	EXPECT_DOUBLE_EQ(ldbTime, ldbPopTime);
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//empty queue, can not pop anything
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 - sizeof(double));
	EXPECT_DOUBLE_EQ(lRet, 0);

	//pop the wrong size
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 );
	EXPECT_DOUBLE_EQ(lRet, 0);

	//push again
	lRet = lQueue.Push(ldbTime, pBuffer, 100 - sizeof(double));
	EXPECT_EQ(100 - sizeof(double), lRet);

	//push another one ,shoule replaced the old one
	ldbTime = 0.2;
	lRet = lQueue.Push(ldbTime, pBuffer, 100 - sizeof(double));
	EXPECT_EQ(100 - sizeof(double), lRet);

	//pop
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 - sizeof(double));
	EXPECT_DOUBLE_EQ(ldbTime, ldbPopTime);
	EXPECT_EQ(lRet, 100 - sizeof(double));

	delete[] pBuffer;
}


TEST(BufferQueue, PushSuccessWithTwoElementQueue)
{
	BufferQueue lQueue(100, 2);
	
	char * pBuffer = new char[100];
	int lRet = lQueue.Push(0.1f, pBuffer, 100 - sizeof(double));
	EXPECT_EQ(lRet, 100 - sizeof(double));

	double ldbPopTime = 0.0f;
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 - sizeof(double));
	EXPECT_DOUBLE_EQ(0.1f, ldbPopTime);
	EXPECT_EQ(lRet, 100 - sizeof(double));
	
	
	//push 0.1
	lRet = lQueue.Push(0.1f, pBuffer, 100 - sizeof(double));
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//push 0.2
	lRet = lQueue.Push(0.2f, pBuffer, 100 - sizeof(double));
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//pop one
	ldbPopTime = 0.0f;
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 - sizeof(double));
	EXPECT_DOUBLE_EQ(0.1f, ldbPopTime);
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//pop second one
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 - sizeof(double));
	EXPECT_DOUBLE_EQ(0.2f, ldbPopTime);
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//pop failed
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 - sizeof(double));
	EXPECT_DOUBLE_EQ(lRet, 0);

	//push 0.1
	lRet = lQueue.Push(0.1f, pBuffer, 100 - sizeof(double));
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//push 0.2
	lRet = lQueue.Push(0.2f, pBuffer, 100 - sizeof(double));
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//push 0.3 //0.1 must be replaced
	lRet = lQueue.Push(0.3f, pBuffer, 100 - sizeof(double));
	EXPECT_EQ(lRet, 100 - sizeof(double));

	ldbPopTime = 0.0f;
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 - sizeof(double));
	EXPECT_DOUBLE_EQ(0.2f, ldbPopTime);
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//pop second one
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 - sizeof(double));
	EXPECT_DOUBLE_EQ(0.3f, ldbPopTime);
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//pop failed
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 - sizeof(double));
	EXPECT_DOUBLE_EQ(lRet, 0);


	//push 0.1
	lRet = lQueue.Push(0.1f, pBuffer, 100 - sizeof(double));
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//push 0.2
	lRet = lQueue.Push(0.2f, pBuffer, 100 - sizeof(double));
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//push 0.3 //0.1 must be replaced
	lRet = lQueue.Push(0.3f, pBuffer, 100 - sizeof(double));
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//push 0.4 //0.2 must be replaced
	lRet = lQueue.Push(0.4f, pBuffer, 100 - sizeof(double));
	EXPECT_EQ(lRet, 100 - sizeof(double));



	ldbPopTime = 0.0f;
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 - sizeof(double));
	EXPECT_DOUBLE_EQ(0.3f, ldbPopTime);
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//pop second one
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 - sizeof(double));
	EXPECT_DOUBLE_EQ(0.4f, ldbPopTime);
	EXPECT_EQ(lRet, 100 - sizeof(double));

	//pop failed
	lRet = lQueue.Pop(ldbPopTime, pBuffer, 100 - sizeof(double));
	EXPECT_DOUBLE_EQ(lRet, 0);

	delete[] pBuffer;
}


int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc,argv);
	return RUN_ALL_TESTS();
  
}

