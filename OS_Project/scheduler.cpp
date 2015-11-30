#include "scheduler.h"

/*
Scheduler for jobs, needs expanding on
*/
void scheduler(int *p, std::vector< std::vector<int> > &memBreaks, std::vector< std::vector<int> > &readyJobs, std::vector< std::vector<int> > &storedJobs)
{
	//FIFS, sorted by priority before getting here. All it does it take last element on ready list and send it in to be computed. Pops it off the vector
	p[2] = readyJobs.back().at(1);
	p[4] = readyJobs.back().at(2);
	readyJobs.pop_back();

}
