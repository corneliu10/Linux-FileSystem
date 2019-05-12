#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define MAX 37

typedef struct nod_trie {
	int isDir, isFile;
	struct nod_trie *copii[MAX]; 
} TNod, *Trie;