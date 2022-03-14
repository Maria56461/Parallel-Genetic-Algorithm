#ifndef ARGUMENT_H
#define ARGUMENT_H

typedef struct _argument {
	sack_object* objects;
	int object_count;
    int sack_capacity;
    int generations_count;
    pthread_barrier_t *bariera;
    individual* currGen;
    individual* nextGen;
    int numberThreads;
    int threadID;
} argument;

#endif
