#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <sys/queue.h>

///Round-Robin scheduling, sequential
///preemptive algorithm

void Round_Robin(int limit, int *sum, int *cnt, int *x, int q, int *wt, int *tat, int *at, int *bt, int *temp, int *i, int *CPU_util) {
	printf("\nProcess ID\tBurst Time\t Turnaround Time\t Waiting Time\n");
	for((*sum) = at[0], (*i) = 0; (*x) != 0; ) {  
		if(temp[(*i)] <= q && temp[(*i)] > 0) {  
    		(*sum) += temp[(*i)];  
    		temp[(*i)] = 0;  
    		(*cnt) = 1;  
    	}     
    	
    	else if(temp[(*i)] > 0) {  
        	temp[(*i)] = temp[(*i)] - q;  
       	 	(*sum) += q;    
       	 	
    	}  
    	if(temp[(*i)]==0 && (*cnt) == 1) {  
        	(*x)--; //decrement the process  
        	printf("\nProcess [%d]\t%d\t\t %d\t\t\t %d", (*i) + 1, bt[(*i)], (*sum) - at[(*i)], (*sum) - at[(*i)] - bt[(*i)]);  
        	(*wt) += (*sum) - at[(*i)] - bt[(*i)];  
        	(*tat) += (*sum) - at[(*i)];  
        	(*cnt) = 0;     
    	}  
    	
    	if((*i) == limit - 1)
        	(*i)=0;  
        	
    	else if(at[(*i)+1]<= (*sum))
        	(*i)++;    
        else if(at[(*i)+1]  > (*sum) && limit - (*x) == (*i) + 1){
        	(*sum) = at[(*i)+1];
        	(*i)++;
        }
    	else 
        	(*i)=0; 
	}
	(*CPU_util) += ((*sum) - at[0]);  
}	

///cunoscut drept FIFO - stack
///non-preemptive algorithm

void FCFS(int *burst_time, int *arrival_time, int *wait_time, int *turnaround_time,int *CPU_util, int limit) {
	
	int *wt_time;
	wt_time = (int*) calloc(limit+1, sizeof(int));
	wt_time[0] = 0;
	int completion_time = arrival_time[0];
	
	///practic completion_time-ul va fi suma burst_time-urilor prcedente	
    for(int i = 0; i < limit; i++) {
    	if(completion_time >= arrival_time[i]){
    		completion_time += burst_time[i];
   		wt_time[i] = completion_time - arrival_time[i] - burst_time[i];
   	}
   	else{
   		completion_time = arrival_time[i]+ burst_time[i];
   		wt_time[i] = 0;
   	}
    }
    
    printf("\nProcess ID\tBurst Time\t Turnaround Time\t Waiting Time\n");
    for(int i = 0; i < limit; i++) {
   		(*wait_time) += wt_time[i];
    	(*turnaround_time) += (burst_time[i] + wt_time[i]);
    	printf("\nProcess[%d]\t%d\t\t %d\t\t\t %d", i + 1, burst_time[i], burst_time[i] + wt_time[i], wt_time[i]);
   	}
   	
   	(*CPU_util) += (arrival_time[limit-1] + burst_time[limit-1] + wt_time[limit-1] - arrival_time[0]);
   	free(wt_time);
}

///shortest job next
///procesele ajung in ordine!
struct proc{
    TAILQ_ENTRY(proc) tailq;
    int bt,at,id;
};
struct proc proc_int(int id, int arrival_time, int burst_time){
    struct proc aux;
    aux.bt = burst_time;
    aux.at = arrival_time;
    aux.id = id;
    return aux;
}
void SJN(int *burst_time, int *arrival_time, int *wait_time, int *turnaround_time,int *CPU_util, int limit) {
	int *wt_time;
	struct proc *procese;
	procese = calloc(limit+1, sizeof(struct proc));
	wt_time = (int*) calloc(limit+1, sizeof(int));
      	wt_time[0] = 0;
      	int clock = arrival_time[0];
	    
	TAILQ_HEAD(procq, proc);
	    
	for(int i = 0; i < limit; i++){
	    	procese[i] = proc_int(i,arrival_time[i],burst_time[i]);
	}
	
	(*CPU_util) -= clock;
	struct procq q;
	TAILQ_INIT(&q);
	int i = 0;
	while(!TAILQ_EMPTY(&q) || i < limit){
		while(procese[i].at <= clock && i < limit){
			struct proc *p;
			int gasit = 0;
			TAILQ_FOREACH(p, &q, tailq)
				if(p->bt > procese[i].bt){
					gasit = 1;
					TAILQ_INSERT_BEFORE(p, &procese[i], tailq);
					break;
				}
			if(gasit == 0) TAILQ_INSERT_TAIL(&q, &procese[i], tailq);
			i++;
		}
		if(!TAILQ_EMPTY(&q)){
			struct proc *aux = TAILQ_FIRST(&q);
			wt_time[aux->id] = clock - aux->at;
			clock = clock + aux->bt;
			TAILQ_REMOVE(&q, aux, tailq);
		}
		else clock += 1;
	}
	 
	printf("\nProcess ID\tBurst Time\t Turnaround Time\t Waiting Time\n");
	for(int i = 0; i < limit; i++) {
	    (*wait_time) += wt_time[i];
	    (*turnaround_time) += (burst_time[i] + wt_time[i]);
	    printf("\nProcess[%d]\t%d\t\t %d\t\t\t %d", i + 1, burst_time[i], burst_time[i] + wt_time[i], wt_time[i]);
	}
	(*CPU_util) += clock;
	free(wt_time);
	free(procese);
}

//shortest remaining time
void SRT(int *burst_time, int *arrival_time,int *wait_time, int *turnaround_time, int *CPU_util, int limit) {
	int *wt_time, *bt_time;
	struct proc *procese;
	procese = calloc(limit+1, sizeof(struct proc));
	wt_time = (int*) calloc(limit+1, sizeof(int));
      	bt_time = (int*) calloc(limit+1, sizeof(int));
      	for(int i = 0; i < limit; i++) {
		wt_time[i] = 0;
		bt_time[i] = burst_time[i];
	}
	
      	int clock = arrival_time[0];
	(*CPU_util) -= clock;
	TAILQ_HEAD(procq, proc);
	    
	for(int i = 0; i < limit; i++){
	    	procese[i] = proc_int(i,arrival_time[i],burst_time[i]);
	}
	
	struct procq q;
	TAILQ_INIT(&q);
	
	int i = 0;
	while(!TAILQ_EMPTY(&q) || i < limit){
		while(procese[i].at <= clock && i < limit){
			struct proc *p;
			int gasit = 0;
			TAILQ_FOREACH(p, &q, tailq)
				if(p->bt > procese[i].bt){
					gasit = 1;
					TAILQ_INSERT_BEFORE(p, &procese[i], tailq);
					break;
				}
			if(gasit == 0) TAILQ_INSERT_TAIL(&q, &procese[i], tailq);
			i++;
		}
		if(!TAILQ_EMPTY(&q)){
			struct proc *aux = TAILQ_FIRST(&q);
			wt_time[aux->id] = wt_time[aux->id] + clock - aux->at;
			bt_time[aux->id] -= 1;
			clock += 1;
			procese[aux->id].at = clock;
			if(bt_time[aux->id] == 0) TAILQ_REMOVE(&q, aux, tailq);
		}
		else clock += 1;
	}
	 
	printf("\nProcess ID\tBurst Time\t Turnaround Time\t Waiting Time\n");
	for(int i = 0; i < limit; i++) {
	    (*wait_time) += wt_time[i];
	    (*turnaround_time) += (burst_time[i] + wt_time[i]);
	    printf("\nProcess[%d]\t%d\t\t %d\t\t\t %d", i + 1, burst_time[i], burst_time[i] + wt_time[i], wt_time[i]);
	}
	(*CPU_util) += clock;
	free(wt_time);
	free(bt_time);
	free(procese);
}


///din input.txt si priority.txt vom alege datele de intrare
int main(int argc, char** argv) {   
	if(argc < 2){
		printf("Please give a file with inputs!\n");
		return 0;
	} 
    int limit, x, alg_option;
	int wait_time = 0, turnaround_time = 0, CPU_util = 0, burst_total = 0, *arrival_time, *burst_time, *temp;
	float average_wait_time, average_turnaround_time;
	
	int fisier = open(argv[1], O_RDONLY);
	char input[1000];
	read(fisier, input, 1000);
	close(fisier);
	char *token = strtok(input," \n");
	limit = atoi(token);
	x = limit;
	
	arrival_time = (int*) calloc(limit+1, sizeof(int));
	burst_time = (int*) calloc(limit+1, sizeof(int));
	temp = (int*) calloc(limit+1, sizeof(int));
	
	for(int i = 0 ; i < limit; i++){
		token = strtok(NULL, " \n");
		arrival_time[i] = atoi(token);
		token = strtok(NULL, " \n");
		burst_time[i] = atoi(token);
		burst_total += burst_time[i];
		
	}
	
	printf("The list of algorithms:\n1.FCFS\n2.Round-Robin\n3.SRT\n4.SJN\n");
    	char continuare[1] = "y";
    	while(continuare[0] == 'y'){
    	     wait_time = 0;
    	     turnaround_time = 0;
    	     CPU_util = 0;
	     printf("\nEnter the algorithm option: ");
	     scanf("%d", &alg_option);
	     if(alg_option == 1)
		 	FCFS(burst_time, arrival_time, &wait_time, &turnaround_time, &CPU_util, limit);
		 	
	     if(alg_option == 2) {
	     	int i = 0, sum = 0, cnt = 0, q = 0, x = limit;
	     	for(int j = 0; j < limit; j++){
	     		temp[j] = burst_time[j];
	     	}
	     	printf("Enter the Time Quantum for the process:");  
			scanf("%d", &q);  
	     	Round_Robin(limit, &sum, &cnt, &x, q, &wait_time, &turnaround_time, arrival_time, burst_time, temp, &i, &CPU_util);
	     }
	     if(alg_option == 3)
			SRT(burst_time, arrival_time, &wait_time, &turnaround_time, &CPU_util, limit);
	     if(alg_option == 4)
			SJN(burst_time, arrival_time, &wait_time, &turnaround_time, &CPU_util, limit);
			
	     average_wait_time = wait_time * 1.0 / limit;
	     average_turnaround_time = turnaround_time * 1.0 / limit;
	       
	     printf("\n\nAverage Waiting Time: %f", average_wait_time);
	     printf("\nAvg Turnaround Time: %f\n", average_turnaround_time);
	     printf("CPU Utilization: %f%% \n\n", burst_total * 100.0 / CPU_util);
	     printf("Continue with another algorithm?(y/n)");
	     scanf("%s",continuare);
   	}
  	free(arrival_time);
    free(burst_time);
   	free(temp);
	return 0;
}
