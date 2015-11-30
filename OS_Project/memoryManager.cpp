#include "memoryManager.h"


void mergeMemory(std::vector< std::vector<int> > &memBreaks)
{
	bool breakTest = false;

	sortMemBreaks(0, memBreaks.size() - 1, memBreaks); //sort to get everything sequential

	int start = 0;
	while (true)
	{
		if (start == memBreaks.size() - 1)
		{
			break;
		}

		if (memBreaks.at(start).at(0) + memBreaks.at(start).at(1) == memBreaks.at(start + 1).at(0)) //If current spot is a distance equal to it's memory away from the next spot (IE, sequential empty spots)
		{
			memBreaks.at(start).at(1) += memBreaks.at(start + 1).at(1); //add together the memory space they both have
			memBreaks.erase(memBreaks.begin() + start + 1); //delete the next in series entry since it's no longer needed

		}

		else //Can't merge current position with next in line? Time to restart, but check if the next in line can be merged with it's sequential next in line
		{
			start++;
		}
	}

}

/*	Sorts the vectors in memBreaks using quick sort by position in memory, sequentially
*/
void sortMemBreaks(int inLeft, int inRight, std::vector< std::vector<int> > &memBreaks)
{
	int left = inLeft, right = inRight;
	int pivot = memBreaks.at((left + right) / 2).at(0);

	std::vector< std::vector<int> > temp;

	/* partition */

	while (left <= right)
	{	//Sort with the intention of putting smallest (highest priority) job on the end to be popped. Insert takes longer with vectors
		while (memBreaks.at(left).at(0) < pivot)
			left++;
		while (memBreaks.at(right).at(0) > pivot)
			right--;

		if (left <= right)
		{
			temp.push_back(memBreaks.at(left));
			memBreaks.at(left) = memBreaks.at(right);
			memBreaks.at(right) = temp.back();
			temp.clear();

			left++;
			right--;
		}
	};

	/* recursion */

	if (inLeft < right)
		sortMemBreaks(inLeft, right, memBreaks);
	if (left < inRight)
		sortMemBreaks(left, inRight, memBreaks);

}


/*	Sorts the vectors in readyJobs using quick sort
Jobs with highest priority (which is priority 1) are placed at the back to be easily popped off vector
*/
void sortReadyJobs(int inLeft, int inRight, std::vector< std::vector<int> > &readyJobs)
{
	int left = inLeft, right = inRight;
	int pivot = readyJobs.at((left + right) / 2).at(0);

	std::vector< std::vector<int> > temp;

	/* partition */

	while (left <= right)
	{	//Sort with the intention of putting smallest (highest priority) job on the end to be popped. Insert takes longer with vectors
		while (readyJobs.at(left).at(0) > pivot)
			left++;
		while (readyJobs.at(right).at(0) < pivot)
			right--;

		if (left <= right)
		{
			temp.push_back(readyJobs.at(left));
			readyJobs.at(left) = readyJobs.at(right);
			readyJobs.at(right) = temp.back();
			temp.clear();

			left++;
			right--;
		}
	};

	/* recursion */

	if (inLeft < right)
		sortReadyJobs(inLeft, right, readyJobs);
	if (left < inRight)
		sortReadyJobs(left, inRight, readyJobs);

}

/*	Sorts the vectors in storedJobs using quick sort
Jobs with highest priority (which is priority 1) are placed at the back to be easily popped off vector
*/
void sortStoredJobs(int inLeft, int inRight, std::vector< std::vector<int> > &storedJobs)
{
	int left = inLeft, right = inRight;
	int pivot = storedJobs.at((left + right) / 2).at(1);

	std::vector< std::vector<int> > temp;

	/* partition */

	while (left <= right)
	{	//Sort with the intention of putting smallest (highest priority) job on the end to be popped. Insert takes longer with vectors
		while (storedJobs.at(left).at(1) > pivot)
			left++;
		while (storedJobs.at(right).at(1) < pivot)
			right--;

		if (left <= right)
		{
			temp.push_back(storedJobs.at(left));
			storedJobs.at(left) = storedJobs.at(right);
			storedJobs.at(right) = temp.back();
			temp.clear();

			left++;
			right--;
		}
	};

	/* recursion */

	if (inLeft < right)
		sortStoredJobs(inLeft, right, storedJobs);
	if (left < inRight)
		sortStoredJobs(left, inRight, storedJobs);

}

/*
swapper: Takes addresses of global variabels used in mainDriver
	If there is availible space, adds job to the FIRST AVAILIBLE space in the drum and keeps track of it by adding it to the readyJobs vector
	If there is no space, slaps it on the storedJobs vector (which can be sorted by priority)

	ISSUES: Very much FIFS, nothing to swap out lower priority objects when memory is full
			No means of memory removal currently
*/
void swapper(int p[], std::vector< std::vector<int> > &memBreaks, std::vector< std::vector<int> > &readyJobs, std::vector< std::vector<int> > &storedJobs)
{
	mergeMemory(memBreaks); //Merges the memory fragments to make searching for avalible memory more successful. May be slow to call each time

	//Check for availible memory slot (Right now, just first availible spot is chosen)
	bool foundSpot = false;
	for (int i = 0; i < memBreaks.size(); i++)
	{
		if (p[3] <= memBreaks.at(i).at(1))
		{
			std::vector<int> input = { p[2], memBreaks.at(i).at(0), p[4] }; // Converts input array into vector. Can't store arrays in anything
			readyJobs.push_back(input); // Add the job description to the main vector

			siodrum(p[1], p[3], memBreaks.at(i).at(0), 0); //Moves job into memory at whatever free location was found
			memBreaks.at(i).at(0) += p[3]; //Move starting pos up by 10. Since it fit, no chance of overriding next in sequence
			memBreaks.at(i).at(1) -= p[3]; //Reduce availible memory by amount used

			if (memBreaks.at(i).at(1) == 0 && memBreaks.size() != 1) //If the availible memory in the spot is now 0, just get rid of the entry. If for some reason a job takes up the entire memory from 0-99, it keeps an entry around to prevent crashing
			{
				memBreaks.erase(memBreaks.begin() + i);
			}

			foundSpot = true; //found a spot, no need to place it on storedJobs
		}
	}

	if (foundSpot == false) //No room? Store for now
	{
		std::vector<int> input; // Converts input array into vector. Can't store arrays in anything
		for (int i = 5; i > 0; i--)
		{
			input.push_back(p[i]);
		}
		storedJobs.push_back(input); // Add the job description to the main vector
	}

}
