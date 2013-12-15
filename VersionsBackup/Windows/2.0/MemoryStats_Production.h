#ifndef _IOSTREAM_
#    include <iostream>
#endif

#ifndef _WINDOWS_
#    include <Windows.h>
#endif

#ifndef _PSAPI_H_
#    include <psapi.h>
#endif

#ifndef _INC_PROCESS
#    include <process.h>
#endif
 

DWORD WINAPI MemoryCheckThread(LPVOID);

/**
 * The MemoryStats class enables you to checkout
 * the maximum memory usage in its lifetime.
 * @author Jimmy Liu (JiaHuan Liu)
 * @version 2.0
 */
class MemoryStats
{

private:
	HANDLE Monitor;
	bool Is_Started;
	SIZE_T LastPeak;
	HANDLE StopEvent;
	bool IgnoreErrors;

protected:
	SIZE_T MaxCounter;
	int TargetPID;
	HANDLE TargetProc;
	int CheckInterval;
	SIZE_T& p_WorkingSet;
	SIZE_T& p_PeakWs;

	void RecvMem( SIZE_T& Mem, SIZE_T& Peak ) {
		MemRefresh();
		Mem = pmc.WorkingSetSize;
		Peak = pmc.PeakWorkingSetSize;
	}

	inline void MemRefresh( void ) {
		GetProcessMemoryInfo( TargetProc, &pmc, sizeof(pmc) );
	}

	HANDLE OpenTargetProcess( void )
	{
		TargetProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, TargetPID );
		return TargetProc;
	}

public:

	PROCESS_MEMORY_COUNTERS pmc;

	//Set default checking interval.
	static const int DefaultCheckInterval = 10;

	//Declare the monitoring thread-function as a friend.
	friend DWORD WINAPI MemoryCheckThread( LPVOID );

	/**
	 * Initialize the MemoryStats class on user's demand.
	 * @param PID (Required)
	 *         The ProcessID of your target process. If this value
	 *        was given 0, it will automatically obtain the current
	 *        process ID as its target.
	 * @param instant_start (Optional)
	 *         Start the statisting instantly right after the 
	 *        initialization if true. Or you can start manually
	 *        using the @See MemoryStats#StartSession if false.
	 *        The default value is true.
	 * @param Interval (Optional)
	 *         The monitoring interval of the monitoring thread.
	 *        Lower interval causes the increment of accuracy but
	 *        the CPU consumption as well. The default value is
	 *        "DefaultCheckInterval".
	 * @param NoErrors (Optional & Not-Used)
	 *         Ignore Errors during the statisting process.
	 */
	MemoryStats( int PID, bool instant_start = true, int Interval = DefaultCheckInterval, bool NoErrors = true )
		: TargetPID(PID), CheckInterval(Interval), IgnoreErrors(NoErrors), p_WorkingSet(pmc.WorkingSetSize), p_PeakWs(pmc.PeakWorkingSetSize) {
		
		if ( PID <= 0 )
			PID = TargetPID = GetCurrentProcessId();
		if ( Interval < 0 )
			Interval = CheckInterval = DefaultCheckInterval;

		Is_Started = false;
		ClearHandles();
		ClearCounters();

		if ( instant_start ) StartSession();
	}

	/**
	 * Clear all the counters manuly.
	 * Not recommanded, use StopSession() or GetMax( true ) instead.
	 */
	void ClearCounters( void ) {
		if ( ! Is_Started ) {
			LastPeak = 0;
			MaxCounter = 0;
		} else {
			LastPeak = GetCurrentPeakWorkingSet();
			MaxCounter = 0;
		}
	}

	void ClearHandles( void ) {
		if ( ! Is_Started ) {
			Monitor = NULL;
			TargetProc = NULL;
			StopEvent = NULL;
		}
	}

	// Start a Statisting Session.
	void StartSession( void ) {
		ClearHandles();
		ClearCounters();

		StopEvent = CreateEvent( NULL, true, false, NULL );
		
		OpenTargetProcess();
		LastPeak = GetCurrentPeakWorkingSet();

		Monitor = CreateThread( 0, 256, MemoryCheckThread, this, 0, NULL );

		Is_Started = true;
	}

	/**
	 * @return The current physical memory usage of your target.
	 */
	SIZE_T GetCurrentWorkingSet( void ) {
		if ( !Is_Started ) OpenTargetProcess();
		MemRefresh();
		return pmc.WorkingSetSize;
	}

	SIZE_T GetCurrentPeakWorkingSet() {
		if ( !Is_Started ) OpenTargetProcess();
		MemRefresh();
		return pmc.PeakWorkingSetSize;
	}

	/**
	 * @return The maximum memory during the session
	 * @param StopChecking (optional)
	 *         Stop the session before obtain the maximum if true.
	 *        Default value is true. Default value is Strongly recommanded.
	 */

	SIZE_T GetMax( bool StopChecking = true ) {
		if ( StopChecking )
			StopSession();
		return MaxCounter;
	}

	// Stop the Statisting Session.
	void StopSession( void ) {
		if ( Is_Started ) {
			SetEvent( StopEvent );
			WaitForSingleObject( Monitor, INFINITE );
			Is_Started = false;
			CloseHandle( TargetProc );
			CloseHandle( StopEvent );
			CloseHandle( Monitor );
		}
		ClearHandles();
	}

	~MemoryStats()
	{
		StopSession();
	}
		
};

//The monitoring thread-function.
DWORD WINAPI MemoryCheckThread( LPVOID arg )
{
	MemoryStats* Obj = (MemoryStats*)arg;
	bool RecordBroken = false;
	SIZE_T CurrentMemory = 0;
	SIZE_T CurrentPeak = 0;
	SIZE_T LastPeak = Obj->LastPeak;
	SIZE_T& Max = Obj->MaxCounter;
	while ( WAIT_TIMEOUT == WaitForSingleObject(Obj->StopEvent, Obj->CheckInterval) ) {
		Obj->RecvMem( CurrentMemory, CurrentPeak );
		if ( CurrentPeak > LastPeak ) {
			RecordBroken = true;
			Max = CurrentPeak;
			break;
		} else if ( CurrentMemory > Max ) {
			Max = CurrentMemory;
		}
	}
	if ( RecordBroken ) {
		WaitForSingleObject(Obj->StopEvent, INFINITE);
		Obj->RecvMem( CurrentMemory, CurrentPeak );
		Max = CurrentPeak;
	}
	return 10;
}