#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <Windows.h>
#include <psapi.h>
#include <process.h>
#include <Dbghelp.h>
#include "MemoryStatistics.h"



using namespace std;

const int CirculateTimes = 10;
const int HintInterval = 1;

const int SingleTranscationMemory = 1000;

const int MaxMemoryStep = 1000;
const int AllocateTimes = 100;

const int WaitBeforeDelete = 0;

int main( void )
{
	int i;
	PROCESS_MEMORY_COUNTERS pmc;
	HANDLE TargetProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId() );
	MemoryStats* p = NULL;

	for (i=1; i<=CirculateTimes; i++) {
		GetProcessMemoryInfo( TargetProc, &pmc, sizeof(pmc) );
		cout<<"Before: "<<pmc.WorkingSetSize<<endl;
		p = new MemoryStats( 0, true, 10 );
		Sleep(100);
		GetProcessMemoryInfo( TargetProc, &pmc, sizeof(pmc) );
		cout<<"After : "<<pmc.WorkingSetSize<<endl;
		delete p;
		Sleep(100);
		GetProcessMemoryInfo( TargetProc, &pmc, sizeof(pmc) );
		cout<<"Delete: "<<pmc.WorkingSetSize<<endl;
		cout<<endl;
	}

	cout<<"Press any key to return 0......";
	system("pause>nul");
	return 0;
}

class SampleClass
{
	int* p;
public:
	SampleClass() {
		p = new int[SingleTranscationMemory];
		for(int*q=p, i=0; i<SingleTranscationMemory; i++,q++)
			*q=1;
	}
	~SampleClass() {
		delete p;
	}
};

int main2( void )
{
	//Variable definations
	int i, j;
	int r;
	SampleClass* obj[AllocateTimes];
	MemoryStats* MyCounter;

	//Initialize random generator
	srand((unsigned)time(0));

	cout<<"Started.\n\n\n";
	MyCounter = new MemoryStats( 0, false );

	//Circulate begin
	for ( i=0; i<CirculateTimes; i++ )
	{
		MyCounter->StartSession();
		for ( j=0; j<AllocateTimes; j++ )
		{
			
			//Generate a random quantity for allocating memory
			r=rand() % MaxMemoryStep;
			obj[j] = new SampleClass[r];
			
		}
		MyCounter->GetMax();
		
		if ( WaitBeforeDelete )
		{
			//if ( !(i%HintInterval) ) cout<<"Wait"<<WaitBeforeDelete<<"ms Before Delete.\n";
			if ( !(i%HintInterval) ) Sleep( WaitBeforeDelete );
		}
		MyCounter->StartSession();
		for ( j=0; j<AllocateTimes; j++ )
		{
			delete[] obj[j];
		}
		MyCounter->GetMax();
	}
	MyCounter->GetMax();
	delete MyCounter;
	cout<<"\n\nFinished.\n";
	cout<<"Press any key to return 0......";
	system("pause>nul");
	return 0;
}