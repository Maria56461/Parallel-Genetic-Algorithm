#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "genetic_algorithm.h"

int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count,  int *numberThreads, int argc, char *argv[]) {

	FILE *fp;

	if (argc < 3) {
		// Numar incorect de argumente!
		fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		printf("Eroare la deschiderea fisierului de intrare!\n");
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
		fclose(fp);
		printf("Eroare la citirea primei linii din fisier (numarul de obiecte + capacitatea rucsacului)!\n");
		return 0;
	}

	if (*object_count % 10) {
		fclose(fp);
		printf("Numarul de obiecte nu este multiplu de 10!\n");
		return 0;
	}

	sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));
	// vector de structuri de tipul "greutate + profit"

	for (int i = 0; i < *object_count; ++i) {
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
			printf("Eroare la citirea vectorului de obiecte din fisierul de intrare!\n");
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int) strtol(argv[2], NULL, 10);
	
	if (*generations_count == 0) {
		printf("Eroare: numarul de generatii trebuie sa fie nenul!\n");
		free(tmp_objects);

		return 0;
	}

	*numberThreads = (int) strtol(argv[3], NULL, 10);
	*objects = tmp_objects;
	// objects = vector de pointeri la structuri 

	return 1;
}

void print_best_fitness(const individual *generation) {

	printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(const sack_object *objects, individual *generation, int object_count, int sack_capacity) {
	// calculeaza profitul pentru fiecare individ (o posibilitate de rucsac cu obiecte) din generatie 
	// intoarce 0 daca greutatea obiectelor puse in ghiozdan depaseste cpapacitatea ghiozdanului 
	int weight;
	int profit;

	for (int i = 0; i < object_count; ++i) {
		weight = 0;
		profit = 0;

		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			if (generation[i].chromosomes[j]) {
				weight += objects[j].weight;
				profit += objects[j].profit;
			}
		}

		generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
	}
}

int cmpfunc(const void *a, const void *b) {
	// compara 2 indivizi (functie pentru qsort)
	individual *first = (individual *) a;
	individual *second = (individual *) b;

	int res = second->fitness - first->fitness; // decreasing by fitness
	if (res == 0) {
		// daca valorile de fitness sunt egale, se face sortare crescatoare dupa numarul de obiecte din rucsac 
		res = first->nrObjects - second->nrObjects; // increasing by number of objects in the sack
		if (res == 0) {
			return second->index - first->index;
		}
	}

	return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index) {

	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0) {
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	} else {
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step) {
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
	}
}

void crossover(individual *parent1, individual *child1, int generation_index) {

	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(individual *generation, int threadID, int numberThreads) {

	int i;
	int start = threadID * (double)generation->chromosome_length / numberThreads;
	int end;
	if (generation->chromosome_length < (threadID + 1) * (double)generation->chromosome_length / numberThreads) {
		end = generation->chromosome_length;
	}
	else {
		end = (threadID + 1) * (double)generation->chromosome_length / numberThreads;
	}

	for (i = start; i < end; ++i) {
		free(generation[i].chromosomes);
		generation[i].chromosomes = NULL;
		generation[i].fitness = 0;
	}

}

void *run_genetic_algorithm(void* arg) {

	argument argumente = *(argument* ) arg;
	int object_count = argumente.object_count; 
	int generations_count = argumente.generations_count;
	int sack_capacity = argumente.sack_capacity;
	int numberThreads = argumente.numberThreads;
	int threadID = argumente.threadID;
	// toate generatiile vor avea acelasi numar de indivizi 
	individual *tmp = NULL;
	int start = threadID * (double)object_count / numberThreads;
	int end;
	if (object_count < (threadID + 1) * (double)object_count / numberThreads) {
		end = object_count;
	}
	else {
		end = (threadID + 1) * (double)object_count / numberThreads;
	}
	
	// set initial generation (composed of object_count individuals with a single item in the sack)
	for (int i = start; i < end; ++i) {
		// initial, generatia curenta are toate valorile de fitness setate pe 0
		argumente.currGen[i].fitness = 0;
		argumente.currGen[i].chromosomes = (int*) calloc(object_count, sizeof(int));
		argumente.currGen[i].chromosomes[i] = 1;
		argumente.currGen[i].index = i;
		argumente.currGen[i].chromosome_length = object_count;

		argumente.nextGen[i].fitness = 0;
		argumente.nextGen[i].chromosomes = (int*) calloc(object_count, sizeof(int));
		argumente.nextGen[i].index = i;
		argumente.nextGen[i].chromosome_length = object_count;
	}

	pthread_barrier_wait(argumente.bariera);

	// iterate for each generation
	for (int k = 0; k < generations_count; ++k) {
		for (int l = start; l < end; l++) {
			int aux = 0;
			for (int m = 0; m < object_count; m++) {
				aux = aux + argumente.currGen[l].chromosomes[m];
			}
			argumente.currGen[l].nrObjects = aux;
		}
		if (threadID == 0) {
			// compute fitness and sort by it
			compute_fitness_function(argumente.objects, argumente.currGen, object_count, sack_capacity);
			qsort(argumente.currGen, object_count, sizeof(individual), cmpfunc);
			// keep first 30% children (elite children selection)
			int start1 = threadID * (double)(object_count * 3 / 10) / numberThreads;
			int end1;
			if ((object_count * 3 / 10) < ((threadID + 1) * (double)(object_count * 3 / 10) / numberThreads)) {
				end1 = object_count * 3 / 10;
			}
			else {
				end1 = (threadID + 1) * (double)(object_count * 3 / 10) / numberThreads;
			}
			for (int i = start1; i < end1; ++i) {
				copy_individual(argumente.currGen + i, argumente.nextGen + i);
				// primii 30% indivizi din noua generatie vor fi elita generatiei vechi 
			}
			// mutate first 20% children with the first version of bit string mutation
			int start2 = threadID * (double)(object_count / 5) / numberThreads;
			int end2;
			if ((object_count / 5) < ((threadID + 1) * (double)(object_count / 5) / numberThreads)) {
				end2 = object_count / 5;
			}
			else {
				end2 = (threadID + 1) * (double)(object_count / 5) / numberThreads;
			}
			for (int i = start2; i < end2; ++i) {
				copy_individual(argumente.currGen + i, argumente.nextGen + object_count * 3 / 10 + i);
				mutate_bit_string_1(argumente.nextGen + object_count * 3 / 10 + i, k);
			// urmatorii 20% indivizi din noua generatie vor fi cei obtinuti prin primul tip de mutatie 
			}
			// mutate next 20% children with the second version of bit string mutation
			for (int i = start2; i < end2; ++i) {
				copy_individual(argumente.currGen + i + object_count / 5, argumente.nextGen + object_count / 2 + i);
				mutate_bit_string_2(argumente.nextGen + object_count / 2 + i, k);
				// urmatorii 20% indivizi din noua generatie vor fi cei obtinuti prin al doilea tip de mutatie 
			}
			// crossover first 30% parents with one-point crossover
			// (if there is an odd number of parents, the last one is kept as such)
			int start3 = threadID * (double)(object_count * 3 / 10 - 1) / numberThreads;
			int end3;
			if ((object_count * 3 / 10 - 1) < ((threadID + 1) * (double)(object_count * 3 / 10 - 1) / numberThreads)) {
				end3 = object_count * 3 / 10 - 1;
			}
			else {
				end3 = (threadID + 1) * (double)(object_count * 3 / 10 - 1) / numberThreads;
			}
			if ((object_count * 3 / 10) % 2 == 1) {
				// daca ramane un parinte fara pereche, acesta se pastreaza in noua generatie 
				copy_individual(argumente.currGen + object_count - 1, argumente.nextGen + object_count - 1);
				for (int i = start3; i < end3; i += 2) {
					crossover(argumente.currGen + i, argumente.nextGen + object_count * 7 / 10 + i, k);
					// ultimii 30% indivizi ai noii generatii se obtin prin crossover 
				}
			}
			else {
				for (int i = start1; i < end1; i += 2) {
					crossover(argumente.currGen + i, argumente.nextGen + object_count * 7 / 10 + i, k);
				}
			}
			// switch to new generation
			tmp = argumente.currGen;
			argumente.currGen = argumente.nextGen;
			argumente.nextGen = tmp;
			for (int i = 0; i < object_count; ++i) {
				argumente.currGen[i].index = i;
			}
			if (k % 5 == 0) {
				print_best_fitness(argumente.currGen);
				// k = numarul de generatii obtinute 
			}
		}
	}

	pthread_barrier_wait(argumente.bariera);

	if (threadID == 0) {
	compute_fitness_function(argumente.objects, argumente.currGen, object_count, sack_capacity);
	qsort(argumente.currGen, object_count, sizeof(individual), cmpfunc);
	print_best_fitness(argumente.currGen);
	}
	

	pthread_exit(NULL);
}