#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "genetic_algorithm.h"


int main(int argc, char *argv[]) {
	
	// array with all the objects that can be placed in the sack
	sack_object *objects = NULL;

	// number of objects
	int object_count = 0;

	// maximum weight that can be carried in the sack
	int sack_capacity = 0;

	// number of generations
	int generations_count = 0;

	// numarul total de thread-uri 
	int numberThreads = 0; 
	
	pthread_barrier_t bariera;

	if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, &numberThreads, argc, argv)) {
		return 0;
	}

	pthread_t threads[numberThreads];
	individual* currGen = (individual*) calloc(object_count, sizeof(individual));
	individual* nextGen = (individual*) calloc(object_count, sizeof(individual));
	int i;
	argument *argumente = (argument*) calloc(numberThreads, sizeof(argument));;
	pthread_barrier_init(&bariera, NULL, numberThreads);

	// se creeaza thread-urile
	for (i = 0; i < numberThreads; i++) {
		argumente[i].threadID = i;
		argumente[i].objects = objects;
		argumente[i].object_count = object_count;
		argumente[i].sack_capacity = sack_capacity;
		argumente[i].generations_count = generations_count;
		argumente[i].bariera = &bariera; 
		argumente[i].numberThreads = numberThreads;
		argumente[i].currGen = currGen;
		argumente[i].nextGen = nextGen;
		pthread_create(&threads[i], NULL, run_genetic_algorithm, &argumente[i]);
	}
	
	// se asteapta thread-urile
	for (i = 0; i < numberThreads; i++) {
		pthread_join(threads[i], NULL);
		free_generation(currGen, i, numberThreads);
		free_generation(nextGen, i, numberThreads);
	}

	free(objects);
	// free resources
	free(currGen);
	free(nextGen);

	return 0;
}
