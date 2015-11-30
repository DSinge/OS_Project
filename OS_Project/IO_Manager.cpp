#include "IO_Manager.h"

std::vector< std::vector<int> > storedIORequests;
bool diskBusy;

//Just to set anything to default values at startup
void IO_ManagerInitialization()
{
	diskBusy = false;
}

/*
Function manages jobs that require I/O service
*/
void diskQueueManager(int *p)
{
	//if the disk is busy, queue it for later and return. If its not busy, add what wanted I/O time onto the queue. Queue not sorted right now
	if (diskBusy == true)
	{
		std::vector<int> input;
		for (int i = 5; i > 0; i--)
		{
			input.push_back(p[i]);
		}
		storedIORequests.push_back(input);
	}

	else //just throw the job onto the I/O handler
	{
		siodisk(p[1]);
	}
}