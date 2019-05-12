#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <ctype.h>
#include "lista.h"
#define MAX 37
Trie aloca(){
	Trie aux=(Trie)malloc(sizeof(TNod));

	if(!aux)
		return 0;
	int i;
	aux->isDir = 0;
	aux->isFile = 0;
		for(i=0; i< MAX; i++)
	{
		aux->copii[i]=NULL;
	}

	return aux;
}
void InsertDirector(Trie *var, char *string){
	Trie aux= *var;
	int contor;
	while(*string)
	{
		contor=*string-'a';
		if(aux->copii[contor]== NULL){
			aux->copii[contor] = aloca();
			if(aux->copii[contor] ==  NULL )
				return;
		}
		aux=aux->copii[contor];
		string++;
	}
	aux->isDir=1;
}
void InsertFile(Trie *var, char *string){
	Trie aux= *var;
	int contor;
	while(*string)
	{
		contor=*string-'a';
		if(aux->copii[contor]== NULL){
			aux->copii[contor] = aloca();
			if(aux->copii[contor] ==  NULL )
				return;
		}
		aux=aux->copii[contor];
		string++;
	}
	aux->isFile=1;
}
int SearchDirector(Trie *var, char *string){
	int contor;
	if(var == NULL)
		return 0;
	Trie aux = *var;
	while(*string)
	{
		contor=*string-'a';
		aux=aux->copii[contor];
		if(aux == NULL )
			return 0;
		string++;
	}
	return aux->isDir;
}
int SearchFile(Trie *var, char *string){
	int contor;
	if(var == NULL)
		return 0;
	Trie aux = *var;
	while(*string)
	{
		contor=*string-'a';
		aux=aux->copii[contor];
		if(aux == NULL )
			return 0;
		string++;
	}
	return aux->isFile;
}
int verificare(char *str){
	while(*str){
		printf("%d\n",*str );
		str++;
	}
	return 0;
}

void pwd(Trie var, char *string){
	int i;
	if( var == NULL)
		return;
		//printf("/\n" );
	for(i=0; i<MAX; i++){
		if (var->copii[i] != NULL)
		{
			printf("%c\n",i+'a' );
		}
		pwd(var->copii[i],string);
	}
}
int verifica_alfa(char string[10]){
	if(string[0] == '/' )
		return 1;
	else return 0;
   }

int cd(Trie *var, char *string, FILE *out1, char *cale){
	if(var == NULL)
		return 0;
	Trie aux = *var;
	char *a="..";
	char *s="/";
	int contor;
	if(strcmp(string,a) == 0){
		if(strcmp(cale,s) == 0){
			fprintf(out1,"%s: No such file or directory\n", string);
		}
	}
	else
	{
		if(SearchDirector(var,string) == 0){
			fprintf(out1,"%s: No such file or directory\n", string);
		}
		else
		{
			if(aux == NULL)
				return 0;
			while(*string){
				contor=*string-'a';
				aux=aux->copii[contor];
				if(aux == NULL )
					return 0;
				string++;
			}
			*var=aux;

		}
		
	}
	return 0;
}
/*int cauta(Trie *var,char string, char *cale){

	int contor;
	Trie *aux=var;
		if(var == NULL)
			return 0;
		Trie aux = *var;
		while(*string)
		{
			contor=*string-'a';
			aux=aux->copii[contor];
			if(aux == NULL )
				return 0;
			string++;
		}
		return aux->isDir;
}*/
void FreeAll(Trie var){
	if(!var)
		return;
	int i;
	for(i=0;i<MAX;i++)
		if(var->copii[i] != NULL)
			FreeAll(var->copii[i]);
		
	free(var);
}