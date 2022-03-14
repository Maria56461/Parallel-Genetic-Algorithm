#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

// structure for an individual in the genetic algorithm; the chromosomes are an array corresponding to each sack
// object, in order, where 1 means that the object is in the sack, 0 that it is not

typedef struct _individual {
	int nrObjects;
	int fitness;
	// valoarea de fitness, calitatea individului 
	int *chromosomes;
	// sirul de cromozomi 
    int chromosome_length;
	// numarul de obiecte existente 
	int index;
} individual;

#endif