#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include <vector>

void IO_ManagerInitialization();
void diskQueueManager(int *p);

void siodisk(int jobnum); //Function not defined in IO_Manager.cpp, part of SOS

#endif