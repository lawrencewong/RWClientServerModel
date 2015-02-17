typedef struct {
	int clientID;
	char requestType;
	char * filename;
} packet;

// Information for each thread, including the thread's lock and all of the reader's locks
typedef struct {
	int thread_id;
	int iterations;
	int writers;
	int readers;
} thread_data;

// initializeFile function sets up the binary file based on how many writer threads there will be.
void* initializeFile(int num_writers, char * filename);