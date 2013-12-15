void PrintMemoryInfo( DWORD processID )
{
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;
    // Print the process identifier.
    cout<<"\nProcess ID: "<<processID<<"\n";
    // Print information about the memory usage of the process.
    hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
                                    PROCESS_VM_READ,
                                    FALSE, processID );
    if (NULL == hProcess)
        return;

	//InitializeProcessForWsWatch(hProcess);

    if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
    {
        //cout<<"\tPageFaultCount: "<<pmc.PageFaultCount<<endl;
        cout<<"\tPeakWorkingSetSize: "<<pmc.PeakWorkingSetSize<<endl;
        cout<<"\tWorkingSetSize: "<<pmc.WorkingSetSize<<endl;
        cout<<"\tQuotaPeakPagedPoolUsage: "<<pmc.QuotaPeakPagedPoolUsage<<endl;
        cout<<"\tQuotaPagedPoolUsage: "<<pmc.QuotaPagedPoolUsage<<endl;
        cout<<"\tQuotaPeakNonPagedPoolUsage: "<<pmc.QuotaPeakNonPagedPoolUsage<<endl;
        cout<<"\tQuotaNonPagedPoolUsage: "<<pmc.QuotaNonPagedPoolUsage<<endl;
        cout<<"\tPeakPagefileUsage: "<<pmc.PeakPagefileUsage<<endl;
        cout<<"\tPagefileUsage: "<<pmc.PagefileUsage<<endl; 
    }
    CloseHandle( hProcess );
}

int echo( void )
{
    // Get the list of process identifiers.
    DWORD aProcesses = _getpid();
    PrintMemoryInfo( aProcesses );
    return 0;
}