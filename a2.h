// Information for each thread
typedef struct thread_data{
	int thread_id;
	int iterations;
	int writers;
	int readers;
	char * filename;
	struct sockaddr_in servaddr;
} thread_data;

// Queue nodes
typedef struct ticketNode{
	int pid;
	char requestType;
	int socketFD;
	int thread_id;
	int iteration;
	struct sockaddr_in cliaddr;
	struct ticketNode * next;
} ticketNode;

// Struct to house flags to keep writers and readers permission
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
// Starts a queue for the client given a ticket
void startClientQueue(int pid, char requestType, int index, int socketFD, int thread_id, int iteration, struct sockaddr_in cliaddr);
// Adds to the queue for a given ticket
void addToClientQueue(int pid, char requestType, int index, int socketFD, int thread_id, int iteration, struct sockaddr_in cliaddr);
// Accepts release from client determines how the next element of queue will be ran
void releaseClientQueue(int index, int pid, char requestType, char release);
// Dequeues a ticket if it has permissions to run
void runProcess(int index);