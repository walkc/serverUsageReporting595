

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

double percent_usage = 0;
int first_reading = 0;
int second_reading = 0;
double max_usage = 0;
double avgs_count = 0;
double runningTotal = 0;
double array_of_avgs[3600];
double running_avg = 0;
int index_of_avgs = 0;
int flag = 0;
pthread_mutex_t lock_max;
pthread_mutex_t lock_last;
pthread_mutex_t lock_avg;
pthread_mutex_t lock_print;
pthread_mutex_t lock_flag;
// extern int fd;

void* get_input(void* p){
    char* val = malloc(sizeof(char)*100);
   //start loop 
    while(1){
        //get input
        scanf("%s", val);
        //check if input is q return. if it is, quit
        if(strcmp(val, "q")==0){
//             printf("got the q\n");
            free(val);
            pthread_mutex_lock(&lock_flag);
            flag = 1;
            pthread_mutex_unlock(&lock_flag);
//             close(fd);
//             printf("done in get input\n");
            return NULL;
        }    
    }
    free(val);
    return NULL;
}

void* get_stats(void* p){
    FILE* proc;
//     int* ret=malloc(sizeof(int));
//     int readings=0;
    while(1){
        pthread_mutex_lock(&lock_flag);
       if(flag==1){
           pthread_mutex_unlock(&lock_flag);
//            printf("got stats broke\n");
           return NULL;
           break;
       } 
         pthread_mutex_unlock(&lock_flag);
//         char* usage = malloc(sizeof(int)*20);
        char* usage;
        char* cpu = malloc(sizeof(char)*100);
       
        proc = fopen("/proc/stat", "r");
        if(proc==NULL){
            printf("proc is null\n");
//             *ret = -1;
            return NULL;
        }
        fgets(cpu, 100, proc);
        pthread_mutex_lock(&lock_print);
        printf("%s", cpu);
        pthread_mutex_unlock(&lock_print);
        strtok(cpu, " ");
        for(int i= 0; i<4; i++){
            usage = strtok(NULL, " ");
        }
        pthread_mutex_lock(&lock_print);
        printf("%s\n", usage);
        pthread_mutex_unlock(&lock_print);

        
        if(avgs_count==0){
            first_reading=atof(usage);
        }
        else{
          int thisReading = atof(usage);
          int dif = thisReading-first_reading;
            pthread_mutex_lock(&lock_last);
          percent_usage = 100-(dif/4.0);
            pthread_mutex_unlock(&lock_last);
          first_reading=thisReading;
                if(percent_usage<0){
                    printf("0\n");
                }
                if(percent_usage>100){
                    printf("100\n");
                }
                else{
                    printf("%f\n", percent_usage);
                } 
            
            //update maxUsage
            pthread_mutex_lock(&lock_max);
            if(percent_usage>max_usage){
                max_usage=percent_usage;
            }
            printf("max_usage: %f\n", max_usage);
             pthread_mutex_unlock(&lock_max);
            
            //check the index of the array of averages
            //if the index is 3600, reset the index to 0
            if(index_of_avgs==3600){
                index_of_avgs=0;
            }
            
            //check the count of all averages to date
            //if the count is larger than 3600 (aka its been an hour or more), calculate avg 
            if(avgs_count>3600){
                //subtract old value, add new value to running total, divide by 3600
                runningTotal = runningTotal - array_of_avgs[index_of_avgs] + percent_usage;
                running_avg = runningTotal/3600;
                //add the new value to the array
                array_of_avgs[index_of_avgs]=percent_usage;
            }
            
            //if the count is less 3600 (it hasn't been an hour) 
            else{
                //add the value to the array
                array_of_avgs[index_of_avgs]=percent_usage;
                index_of_avgs++;
               //update the stats for overall average
                   runningTotal+=percent_usage;
                pthread_mutex_lock(&lock_avg);
                   running_avg = runningTotal/avgs_count;
                   printf("running average: %f\n", running_avg);
                pthread_mutex_unlock(&lock_avg);
            }
            
        }
        avgs_count++;
 
        
        fclose(proc);
        free(cpu);
      

        
        sleep(1);
    }
//         printf("done in get stats");
        return NULL;
}

