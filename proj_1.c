#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int lines=0;
char** lines_content;
char* search_str;
int fd;

void rec_search(int start, int end);
void count_Line();
void count_Elem();
void insert_Elem();

int main(int argc, char* argv[]) {

/*Controllo errore inserimento dal terminale*/	
	
	search_str=argv[1];
	if(argc!=3){
		exit(-1);
	}

	fd=open(argv[2],O_RDONLY);
	if(fd==-1){
		printf("Error in opening the file\n");
		exit(-1);
	}
	
	count_Line();
	lseek(fd,0,SEEK_SET);
	count_Elem();
	lseek(fd,0,SEEK_SET);
	insert_Elem();
	
}

void count_Line() {

	char c;
	while(read(fd,&c,1)!=0){
		if(c=='\n')
			lines++;
	}

	printf("Lines: %d\n",lines);

	lines_content=(char**)malloc(lines*sizeof(char*));

}

void count_Elem() {
	char c;
	int line_len=0,i=0;
	while(read(fd,&c,1)!=0){
		line_len++;
		if(c=='\n'){
			lines_content[i++]=(char*)malloc((line_len)*sizeof(char));
			//printf("%d\n",line_len);
			line_len=0;
		}
	}
	/*if(i!=lines){
		lines_content[i]=(char*)malloc((line_len)*sizeof(char));
	}*/

}

void insert_Elem() {
	char c;
	int i=0, j=0;
	while(read(fd,&c,1)!=0){
		if(c=='\n'){
			lines_content[i][j]='\x00';
			j=0;
			printf("%s\n",lines_content[i]);
			i++;
		}
		else
			lines_content[i][j++]=c;
	}
	if(i!=lines){
		lines_content[i][j]='\x00';
		printf("%s\n",lines_content[i]);
	}


//Inizio ricerca carattere
	int start=0,end=lines-1;
	printf("Go search!\n");
	rec_search(start,end);

}

//algoritmo di ricerca
void rec_search(int start, int end){
	int half=(start+end)/2;//Divisione logica dell'array
	int pid=fork();// Creazione primo figlio
	if(pid==0){//Se è processo figlio
		if(start==half){
			printf("%d-%s\n",start,lines_content[start] );
//Verifica se valore trovato
			if(strcmp(search_str,lines_content[start]) == 0)
				printf("FOUND!\n");
			exit(0);
		}
		else{
			rec_search(start,half);
		}
	}
	else{
		int pid2=fork();//Creazione secondo figlio
		if(pid2==0){
			if(half+1==end){
				printf("%d-%s\n",end,lines_content[end] );
				if(strcmp(search_str,lines_content[end])==0)
					printf("FOUND!\n");
				exit(0);
			}
			else{
				rec_search(half+1,end);
			}
		}
		else{
			int ret=0;
			waitpid(pid,&ret,0);//Attesa del 1° figlio
			waitpid(pid2,&ret,0);//Attesa del 2° figlio
		}
	}
}
