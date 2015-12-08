#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <queue>
#include <iterator>
using namespace std;

void siodisk(int jobnum);
void siodrum(int jobnum, int jobsize, int coreaddress, int direction);
void ontrace(); // called without arguments
void offtrace(); // called without arguments

// Daniel
// Holds any relevant info about a job
struct PCB {
    int jobNumber;
    int priority;
    int jobSize;
    int maxCpuTime;
    int arrivalTime;
    int cpuTimeUsed;
	int memoryPos;

    bool inCore;
    
    bool onIOQueue;
    bool isDoingIO;

};
// Stores all jobs
// Index is a job number and value is a PCB
static map<int, PCB> jobTable;

// Daniel
// Stores all the free spaces in memory
// in the form <Starting address, Size of free space>
// New nodes are created as needed
static list<pair<int, int> > freeSpaceTable;

// Stores pointers to PCBs in jobTable
// Using pointers to easily test if the job is in memory
static queue<PCB*> IOQueue;

//Stores the current time, taken from interrupts
static int time;

bool jobInCPU;

void allocMem(int startAddress, int jobSize);
int findMemLoc(int jobSize);

//Finds the highest priority job stored in jobTable and returns the job (key) number
unsigned int findJob();

bool scheduler(int &a, int p[]);


/* Function: startup
 * -----------------
 * Allows initialization of (static) system variables declared above.
 * Called once at start of the simulation.
 */

void startup(){
    
    //ontrace();
    // First node - All of memory is free
    freeSpaceTable.push_back(pair<int, int>(0, 99));
    time = 0;
	jobInCPU = false;
}



// INTERRUPT HANDLERS
// The following 5 functions are the interrupt handlers. The arguments
// passed from the environment are detailed with each function below.
// See RUNNING A JOB, below, for additional information

/* Function: Crint
 * ---------------
 * Indicates the arrival of a new job on the drum.
 * At call : p [1] = job number
 * p [2] = priority
 * p [3] = job size, K bytes
 * p [4] = max CPU time allowed for job
 * p [5] = current time
 */
void Crint(int &a, int p[]){
	printf("Crint called\n");

    // Create a PCB and store it in the job table
    // Maybe this should be a constructor?
    // I think it's clearer this way, but that could be argued
    PCB pcb;
    pcb.jobNumber = p[1];
    pcb.priority = p[2];
    pcb.jobSize = p[3];
    pcb.maxCpuTime = p[4];
    pcb.arrivalTime = p[5];
    pcb.cpuTimeUsed = 0;
    pcb.inCore = false; // Will be set in Drmint
    pcb.onIOQueue = false;
    pcb.isDoingIO = false;
	pcb.memoryPos = NULL;

    int memLoc = findMemLoc(p[3]);

    if(memLoc >= 0){
        allocMem(memLoc, p[3]);
		pcb.memoryPos = memLoc;
        siodrum(p[1], p[3], memLoc, 0);
    }
    else{
        // Swap a job out
        exit(0); // Temporary
    }

	jobTable.insert(pair<int, PCB>(p[1], pcb));

    time = p[5];
	printf("Time = %i\n\n", time);
}



/* Function: Dskint
 * ----------------
 * Disk interrupt.
 * At call: p[5] = current time
 */
void Dskint(int &a, int p[]){
	printf("Dskint called\n");

    printf(" CPU time used: %d\n", jobTable[1].cpuTimeUsed);
    IOQueue.pop();
	jobTable[p[1]].isDoingIO = false;
    
    if(!IOQueue.empty()){
        printf("IOQueue not empty");
    }

	if (jobInCPU == false)
		scheduler(a, p);

    time = p[5];
	printf("Time = %i\n\n", time);

}



/* Function: Drmint
 * ----------------
 * Drum interrupt. When drum is finished moving something in or out of memory, this is called
 * At call : p [5] = current time
 */
void Drmint(int &a, int p[]){
	cout << "Drmint Called\n";
	jobTable[p[1]].inCore = true;

	//Only scheduling here if there's absolutely nothing in the CPU
	if (jobInCPU == false)
		scheduler(a, p);

  //  printf("p[4]: %d\n", p[4]);

    time = p[5];    
	printf("Time = %i\n\n", time);
}



/* Function: Svc
 * -------------
 * Supervisor call from user program.
 * At call : p [5] = current time
 * a = 5 => job has terminated
 * a = 6 => job requests disk i/o
 * a = 7 => job wants to be blocked until all its pending
 * I/O requests are completed
 */
void Svc(int &a, int p[]){
	printf("Svc called\n");

	jobTable[p[1]].cpuTimeUsed += p[5] - time;
	printf(" Job %i CPU time used: %d\n", p[1], jobTable[p[1]].cpuTimeUsed);
    
    
    switch(a){
        case 5:    // Job has terminated
        {
			printf(" Job %i has terminated\n", p[1]);
			jobTable.erase(p[1]);
			if (scheduler(a, p) == false) //Scheduler will run, but if it returns 0, that means there is nothing that can be scheduled, so it will stall the CPU
			{	
				printf(" No jobs could be run, CPU will stall.\n");
				jobInCPU = false;
				a = 1;
			}
			time = p[5];
        } break;
        
        case 6:    // Job requests disk I/O
        {//Only handling first item
			printf(" Job %i requested I/O\n", p[1]);

            IOQueue.push(&jobTable[p[1]]);
            jobTable[p[1]].onIOQueue = true;
            //cout << IOQueue.front()->jobNumber << endl;

            siodisk(IOQueue.front()->jobNumber);
            jobTable[IOQueue.front()->jobNumber].isDoingIO = true;

            a = 2;
            p[2] = 0;
            p[3] = 18;
            p[4] = jobTable[1].maxCpuTime - jobTable[1].cpuTimeUsed;
            time = p[5];
        } break;
        
        case 7:    // Job wants to be blocked
        {
			printf(" Job %i wants to be blocked\n", p[1]);
			if (scheduler(a, p) == false) //Scheduler will run, but if it returns 0, that means there is nothing that can be scheduled, so it will stall the CPU
			{
				jobInCPU = false;
				a = 1;
			}
			time = p[5];
        } break;

        default:
        {
            printf("a has invalid value %d\n", a);
        }
    }
	printf("Time = %i\n\n", time);
}

/* Function: Tro
* -------------
* Timer-Run-Out.
* At call : p [5] = current time
*/
void Tro(int &a, int p[]){

	printf("Tro called\n");

	time = p[5];
	printf("Time = %i\n\n", time);
}


//End of Interrupts
//===================
//Start of Functions



/* Function: allocMem
* ------------------
* Updates the Free Space Table when a job is placed in memory
*/
void allocMem(int startAddress, int jobSize){

	list<pair<int, int> >::iterator freeSpaceIter;

	for (freeSpaceIter = freeSpaceTable.begin();
		freeSpaceIter != freeSpaceTable.end();
		freeSpaceIter++)
	{
		if (freeSpaceIter->first == startAddress){
			freeSpaceIter->first += jobSize;
			freeSpaceIter->second -= jobSize;
			if (freeSpaceIter->second == 0){
				freeSpaceTable.erase(freeSpaceIter);
				return;
			}
		}
	}
}



/* Function: findMemLoc
 * --------------------
 * First-Fit Algorithm
 * Looks in the Free Space Table for the first space
 * that is >= jobSize
 *
 * returns: the address of a valid space (0-99)
 *          -1 if no space is >= jobSize
 */
int findMemLoc(int jobSize){

    list<pair<int, int> >::iterator freeSpaceIter;

    for(freeSpaceIter = freeSpaceTable.begin();
        freeSpaceIter != freeSpaceTable.end();
        freeSpaceIter++)
    {
        if(freeSpaceIter->second >= jobSize){
            return freeSpaceIter->first;
        }
    }
    return -1;
    
}



/* Function: findJob
* ---------------
* Sets up an iterator to check the jobTable map
* While the first element of the map is not in core, it will keep changing the starting values until it finds one that is
* Afterwards, it checks the remaining members of jobTable for any that might have higher priority
* Returns the position of a high priority item
* If there are no jobs that can be run, it should follow the map until the end of the list and return NULL
*/
unsigned int findJob()
{
	std::map<int, PCB>::iterator jobIt = jobTable.begin();
	if (jobTable.size() == 0) //Returns null if the job table is empty
		return NULL;

	unsigned int highPriJobPriority = jobIt->second.priority, highPriJobPos = jobIt->first; //Set the default choice values to first item

	for (jobIt = jobTable.begin(); jobIt != jobTable.end(); ++jobIt)
	{
		if (jobIt->second.priority < highPriJobPriority && jobIt->second.inCore == true && jobIt->second.isDoingIO == false) //Is the priority on the node it's looking at and is it in the core?
		{
			highPriJobPos = jobIt->first;
			highPriJobPriority = jobIt->second.priority;
		}
	}

	//Here to make sure the default isn't breaking the rules. If the only choice is one that's busy or not inCore, it's useless
	if (jobTable[highPriJobPos].isDoingIO == true || jobTable[highPriJobPos].inCore == false)
	{
		printf(" No suitable job could be found!\n");
		return NULL;
	}

	return highPriJobPos;
}



/* Function: scheduler
* --------------------
* Tells the CPU which job to process
* Highest Priority chosen using 
*  
*/
bool scheduler(int &a, int p[])
{	 
	int pos = findJob();
	if (pos == NULL)
		return false;

	printf(" Placing Job %i into the CPU\n", pos);

	p[1] = jobTable[pos].jobNumber;
	p[2] = jobTable[pos].memoryPos;
	p[3] = jobTable[pos].jobSize;
	p[4] = jobTable[pos].maxCpuTime - jobTable[pos].cpuTimeUsed;
	printf(" p[4]= %i\n", p[4]);

	a = 2;
	jobInCPU = true;

	return true;
}