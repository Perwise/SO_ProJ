#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int lines=0;
char** lines_content;
char* search_str;
char final[256];
int pfd1[2];

void rec_search(int , int , int);

int main(int argc, char* argv[]) {

	void print_usage() {
    	printf("Usage: file_eseguibile  -v valore  -i input.txt  -o output.txt");
    	
    	exit(0);
    	
    	} 
	
	
	search_str=argv[1];
	if(argc!=3){
		exit(-1);
	}

	int fd=open(argv[2],O_RDONLY);
	if(fd==-1){
		printf("Error in opening the file\n");
		exit(-1);
	}
	char c;
	while(read(fd,&c,1)!=0){
		if(c=='\n')
			lines++;
	}

	printf("Lines: %d\n",lines);

	lines_content=(char**)malloc(lines*sizeof(char*));

	lseek(fd,0,SEEK_SET);

	int line_len=0,i=0;
	while(read(fd,&c,1)!=0){
		line_len++;
		if(c=='\n'){
			lines_content[i++]=(char*)malloc((line_len)*sizeof(char));
			//printf("%d\n",line_len);
			line_len=0;
		}
	}
	if(i!=lines){
		lines_content[i]=(char*)malloc((line_len)*sizeof(char));
	}

	lseek(fd,0,SEEK_SET);

	i=0;
	int j=0;
	while(read(fd,&c,1)!=0){
		if(c=='\n'){
			lines_content[i][j]='\x00';
			j=0;
			printf("%d:%s\n",i,lines_content[i]);
			i++;
		}
		else
			lines_content[i][j++]=c;
	}
	if(i!=lines){
		lines_content[i][j]='\x00';
		printf("%d:%s\n",i,lines_content[i]);
	}

	printf("\n\nGo search!\n");
	int start=0,end=lines-1;
	rec_search(start,end,0);

}

void rec_search(int start, int end, int fd_){


	int pfd1[2], pfd2[2];
	
	int half=(start+end)/2;
	if (pipe(pfd1) < 0){
	
	perror("pipe");
	 exit(0);
	 }
	 if (pipe(pfd2) < 0){
	
	perror("pipe");
	 exit(0);
	 } 
	int pid=fork();
	
	if(start==half)
		printf("INDEX LEFT:%d-%d\n",start,end);
	//if(end==half+1)
	//	printf("INDEX RIGHT:%d-%d\n",start,end);
	
	
	if(pid==0){
	close(pfd1[0]);
		if(start==half){
			if(strcmp(search_str,lines_content[start])==0){
			
				int new=1;
				write(pfd1[1],&new,sizeof(new));//# di match
				write(pfd1[1],&start,sizeof(start));//valore
			}
			exit(0);
		}
		else{
			rec_search(start,half,pfd1[1]);
		}
	}
	else{
		int pid2=fork();
		if(pid2==0){
			close(pfd2[0]);
			if(half+1==end){
				if(strcmp(search_str,lines_content[end])==0){
					
					int new=1;
					write(pfd2[1],&new,sizeof(new));
					write(pfd2[1],&end,sizeof(end));
				}
				exit(0);
			}
			else{
				rec_search(half+1,end,pfd2[1]);
			}
		}

		else{
			close(pfd1[1]);
			close(pfd2[1]);
			
			int ret=0;
			int n=0,n_=0;
			
			//aspetto figli...
			waitpid(pid,&ret,0);
			waitpid(pid2,&ret,0);
			
			int tot=0;
			start=0;end=0;
			n=read(pfd1[0], &start, sizeof(start));//read how many results from child-1
			n_=read(pfd2[0], &end, sizeof(end));	//read how many results from child-2
			if(n>0)	tot+=start;
			if(n_>0) tot+=end;
			int vet[tot],i;	//allocate space for results
			if(n>0){
				read(pfd1[0],vet,sizeof(int)*start);	//read first child results
			}
			
			if(n_>0){
				read(pfd2[0],vet+start,sizeof(int)*end);	//read second child results
			}
			
			//print for debug (only root father)
			if(!fd_){	for(i=0;i<tot;i++)	printf("%d ",vet[i]+1);
			printf("\n");}
			
			//if not root father, send through pipe to father
			if(fd_ && tot>0){
				int size=tot;
				int n=write(fd_,&size,sizeof(int));
				if(n==sizeof(int)){
						write(fd_,vet,sizeof(int)*tot);
				}
			}
			
		}
	}
}
