#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include "functii.c"

int main(){

	FILE *in;
	in = fopen("input.in", "r");
	if (in == NULL) {
		fprintf(stderr, "ERROR: Can't open file %s\n", "input.in");
		return -1;
	}


	char output_filename[] = "output.out";
	char output_filename1[] = "er.err";
	FILE *out;
	FILE *out1;
	out = fopen(output_filename, "w");
	out1 = fopen(output_filename1, "w");
	int nr_comenzi;
	char comanda[20];
	Trie var=aloca();
	char *token;
	char nume_director[10];
	char *s="..";
	fscanf(in,"%d",&nr_comenzi);
	char cale[100];
	strcpy(cale,"/");
	printf("nr comenzi   %d\n",nr_comenzi );
	while ((fscanf(in, "%s", comanda) != EOF)) {
		if(strcmp(comanda,"mkdir") == 0){

			fscanf(in,"%[^\n]",nume_director);
			token = strtok(nume_director, "/ ");
			while(token != NULL){
				if(verifica_alfa(nume_director) == 1)
					InsertDirector(&var, token+1);
				else
					InsertDirector(&var,token);
      			token = strtok(NULL, s);
			}
		}
		if(strcmp(comanda,"cd") == 0)
		{
			fscanf(in,"%s",nume_director);
			if(strcmp(nume_director,s) !=0 )
			{
				if(verifica_alfa(nume_director) == 1){
					if(SearchDirector(&var,nume_director+1) == 1){
						strcat(cale,nume_director);
						//strcat(cale,"/");
					}
					cd(&var,nume_director+1,out1,cale);
				}
				else{
					if(SearchDirector(&var,nume_director) == 1){
						strcat(cale,nume_director);
						strcat(cale,"/");
					}
					cd(&var,nume_director,out1,cale);
				}
			}
			else{
				printf("miruna   %s\n",cale );
				cd(&var,nume_director,out1,cale);
				//cauta(&var,nume_director,&cale);
					strcpy(cale,"/");
			}
			
			if(strcmp(comanda,"ls")){
				fscanf(in,"%s",nume_director);
				if(strcmp(nume_director,cale) == 0){
					
				}
			}	
		}
	}
	FreeAll(var);
	fclose(in);
	fclose(out);
	return 0;
	
}