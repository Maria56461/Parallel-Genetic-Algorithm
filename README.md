# Parallel-Genetic-Algorithm
Here is an implementation solving The Knapsack Problem in C. The algorithm is parallelized using PThreads. 

## Maria-Alexandra Barbu, 335CA

Pentru a paraleliza programul am inclus biblioteca "pthread.h". Functia
pe care am paralelizat-o este "run_genetic_algorithm" din fisierul
"genetic_algorithm.c". Pentru ca functia sa primeasca un singur argument, am
creat o structura in fisierul "argument.h". Aceasta structura a primit ca
membri toate argumentele functiei din implementarea secventiala, plus un
pointer la bariera pe care am folosit-o, numarul de thread-uri, ID-ul
thread-ului curent si cei doi vectori de indivizi care vor reprezenta generatia
curenta si noua generatie si vor fi modificati la fiecare iteratie. A fost
nevoie ca acesti vectori sa fie declarati in "main" si trimisi ca argument in
structura pentru ca fiecare thread sa modifice aceleasi variabile de generatie
curenta si generatie noua. Daca ramaneau variabile locale, declarate in
functie, fiecare thread ar fi lucrat pe alte variabile si ar fi modificat parti
din fiecare, dar niciuna dintre generatiile noi obtinute nu ar fi fost complet
corecta. Mai intai am paralelizat "for-ul" de la inceputul programului folosind
formulele pentru "start" si "end" din laboratoare. Am avut grija sa nu eliberez
aceeasi memorie de mai multe ori- astfel, am mutat free-urile in functia "main"
ca sa nu se execute pe mai multe thread-uri. In main am creat thread-urile, o
singura data, apoi le-am dat join. Am observat ca sortarea se face destul de
ineficient- pentru fiecare 2 indivizi din vector care sunt comparati, se
calculeaza mereu suma cromozomilor sai in cazul in care valorile de fitness
sunt egale. Pentru ca acest calcul sa nu se mai realizeze in mod repetat, am 
creat un camp numit "nrObjects" in structutra unui individ si am calculat o
singura data suma cromozomilor sai de fiecare data cand am obtinut o noua
generatie. Am paralelizat apoi aceste calcule. Pentru ca restul calculelor sa
nu fie facute de mai multe ori de multiple thread-uri, le-am pus intr-un bloc
"if" si le-am conditionat sa fie executate doar de primul thread. Un bug
intalnit a fost legat de ID-urile thread-urilor. Pentru ca fiecare thread sa
aiba un ID diferit a fost nevoie ca fiecarui thread sa-i corespunda o alta
structura "argumente", de aceea in "main" am alocat memorie pentru
"numberThreads" astfel de structuri. Am paralelizat, in final, functia pentru
eliberare de memorie. Pentru o scalabilitate mai buna, as fi putut paraleliza
mai multe dintre for-urile din program. 
