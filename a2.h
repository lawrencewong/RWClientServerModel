// Information for each thread, including the thread's lock and all of the reader's locks
typedef struct thread_data{
	int thread_id;
	int iterations;
	int writers;
	int readers;
	char * filename;
	char * dest;
	struct sockaddr_in servaddr;
} thread_data;

typedef struct ticketNode{
	int pid;
	char requestType;
	int socketFD;
	int thread_id;
	int iteration;
	struct sockaddr_in cliaddr;
	struct ticketNode * head;
	struct ticketNode * next;
} ticketNode;

typedef struct clientGroupInfo{
	int pid;
	int numActiveReaders;
	int activeWriter;
} clientGroupInfo;

// initializeFile function sets up the binary file based on how many writer threads there will be.
void* initializeFile(int num_writers, char * filename);
// increment function that is used for the writer threads. The thread will run for the amount of iterations. Before reading and writing writer thread will lock all readers and the main writer lock.
void* increment(void* parameter);
// readNumber function that is used for reader threads. The thread will run for the amount of iterations. Before reading it will use it's own lock.
void* readNumber(void* parameter);
//
void startClientQueue(int pid, char requestType, int index, int socketFD, int thread_id, int iteration, struct sockaddr_in cliaddr);
//
void addToClientQueue(int pid, char requestType, int index, int socketFD, int thread_id, int iteration, struct sockaddr_in cliaddr);
//
void releaseClientQueue(int index, int pid, char requestType, char release);
//
void runProcess(int index, ticketNode * ticketToRun);