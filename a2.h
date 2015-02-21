// Information for each thread, including the thread's lock and all of the reader's locks
typedef struct {
	int thread_id;
	int iterations;
	int writers;
	int readers;
	char * filename;
	char * dest;
} thread_data;

typedef struct{
	int pid;
	char * filename;
	char requestType;
	struct ticketNode * head;
	struct ticketNode * ahead;
} ticketNode;

// initializeFile function sets up the binary file based on how many writer threads there will be.
void* initializeFile(int num_writers, char * filename);
// increment function that is used for the writer threads. The thread will run for the amount of iterations. Before reading and writing writer thread will lock all readers and the main writer lock.
void* increment(void* parameter);
// readNumber function that is used for reader threads. The thread will run for the amount of iterations. Before reading it will use it's own lock.
void* readNumber(void* parameter);