#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <vector>

void swapper(int p[], std::vector< std::vector<int> > &memBreaks, std::vector< std::vector<int> > &readyJobs, std::vector< std::vector<int> > &storedJobs);
void mergeMemory(std::vector< std::vector<int> > &memBreaks);

void sortReadyJobs(int inLeft, int inRight, std::vector< std::vector<int> > &readyJobs);
void sortStoredJobs(int inLeft, int inRight, std::vector< std::vector<int> > &storedJobs);
void sortMemBreaks(int inLeft, int inRight, std::vector< std::vector<int> > &memBreaks);

void siodrum(int jobnum, int jobsize, int coreaddress, int direction); //Function defined in SOS, not memoryManager.cpp, here for references

#endif