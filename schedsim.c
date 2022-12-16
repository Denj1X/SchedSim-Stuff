#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

///Round-Robin scheduling
///preemptive algorithm

void Round_Robin(int *burst_time, int *arrival_time, int *temp, int *wait_time, int *turnaround_time, int limit, int *x) {
	int time_quantum;
	printf("\nEnter Time Quantum: ");
  	scanf("%d", &time_quantum);
  	printf("\nProcess ID\tBurst Time\t Turnaround Time\t Waiting Time\n");
      
  	for(int total = 0, i = 0; (*x) != 0;) {
  		int counter = 0;
      	if(temp[i] <= time_quantum && temp[i] > 0) {
     		total = total + temp[i];
           	temp[i] = 0;
           	counter = 1;
    	}
            
  		else if(temp[i] > 0) {
      		temp[i] = temp[i] - time_quantum;
           	total = total + time_quantum;
     	}
            
      	if(temp[i] == 0 && counter == 1) {
       		(*x)--;
          	printf("\nProcess[%d]\t%d\t\t %d\t\t %d", i + 1, burst_time[i], total - arrival_time[i], total - arrival_time[i] - burst_time[i]);
        	(*wait_time) = (*wait_time) + total - arrival_time[i] - burst_time[i];
        	(*turnaround_time) = (*turnaround_time) + total - arrival_time[i];
          	counter = 0;
  		}
            
 		if(i == limit - 1) {
        	i = 0;
       	}
            
     	else if(arrival_time[i + 1] <= total) {
      		i++;
      	}
            
      	else {
    		i = 0;
   		}
  	}

}

///cunoscut drept FIFO - stack
///non-preemptive algorithm

void FCFS(int *burst_time, int *arrival_time, int *temp, int *wait_time, int *turnaround_time, int limit) {
	for(int i = 0; i < limit; i++) {
		arrival_time[i] = 0;
	}

	int *wt_time, *td_time;
	wt_time = (int*) calloc(limit+1, sizeof(int));
      td_time = (int*) calloc(limit+1, sizeof(int));
      wt_time[0] = 0;
    
      for(int i = 1; i < limit; i++) {
    	wt_time[i] = wt_time[i-1] + burst_time[i-1];
      }
    
      printf("\nProcess ID\tBurst Time\t Turnaround Time\t Waiting Time\n");
      for(int i = 0; i < limit; i++) {
    	      (*wait_time) += wt_time[i];
    	      (*turnaround_time) += (burst_time[i] + wt_time[i]);
    	      printf("\nProcess[%d]\t%d\t\t %d\t\t\t %d", i + 1, burst_time[i], burst_time[i] + wt_time[i], wt_time[i]);
      }
      free(wt_time);
      free(td_time);
}

void SRT() {


}

void SJN() {


}

///din input.txt si priority.txt vom alege datele de intrare
int main(int argc, char** argv) {    
    int limit, x, alg_option;
    int wait_time = 0, turnaround_time = 0, *arrival_time, *burst_time, *temp;
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
    }
    
    
       printf("Enter the algorithm option: ");
     scanf("%d", &alg_option);
     if(alg_option == 1) {
         FCFS(burst_time, arrival_time, temp, &wait_time, &turnaround_time, limit);
     }
     if(alg_option == 2) {
         Round_Robin(burst_time, arrival_time, temp, &wait_time, &turnaround_time, limit, &x);
    }
    average_wait_time = wait_time * 1.0 / limit;
       average_turnaround_time = turnaround_time * 1.0 / limit;
       
       printf("\n\nAverage Waiting Time: %f", average_wait_time);
      printf("\nAvg Turnaround Time: %f\n", average_turnaround_time);
      
      free(arrival_time);
      free(burst_time);
      free(temp);
       return 0;
}
