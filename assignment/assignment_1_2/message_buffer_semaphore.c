#include "message_buffer_semaphore.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>

int shmid;
void *memory_segment=NULL;

int semid;
union semun sem_union;

void init_sem() {
    /*---------------------------------------*/
    /* TODO 1 : init semaphore               */

    if((semid = semget(SEM_KEY, 1, IPC_CREAT|IPC_EXCL|0666)) == -1){
	if((semid = semget(SEM_KEY, 0, 0)) == -1) {
		printf("semaphore init fail!\n\n");
		return;
	}
    }
    sem_union.val = 1;
    semctl(semid, 0, SETVAL, sem_union);

    /* TODO 1 : END                          */
    /*---------------------------------------*/
    printf("init semid : %d\n", semid);
}

void destroy_sem() {
    /*---------------------------------------*/
    /* TODO 2 : destroy semaphore            */

    if(semctl(semid, 0, IPC_RMID) == -1){
	printf("semctl error!\n\n");
    	return;	
    }

    printf("destroy semaphore\n\n");

    /* TODO 2 : END                          */
    /*---------------------------------------*/
}

void s_wait() {
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = -1;
    buf.sem_flg = SEM_UNDO;

    if (semop(semid, &buf, 1) == -1) {
        printf("<s_wait> semop error!\n");
        return ;
    }
}

void s_quit() {
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = 1;
    buf.sem_flg = SEM_UNDO;

    if (semop(semid, &buf, 1) == -1) {
        printf("<s_quit> semop error!\n");
        return ;
    }
}

/*---------------------------------------------*/
/* TODO 3 : use s_quit() and s_wait()          */
/* (1) clone from assignment 1-1 !!!           */
/* (2) use semaphore                           */
/* (3) report explanation (ex. 2020020000.pdf) */


int init_buffer(MessageBuffer **buffer) {
    /*---------------------------------------*/
    /* TODO 1 : init buffer                  */
    int i;
    if((shmid = shmget(SHM_KEY, sizeof(MessageBuffer), IPC_CREAT|0666)) == -1) return -1;
    if((memory_segment = shmat(shmid, NULL, 0)) == (void*)-1) return -1;
    *buffer = (MessageBuffer*)memory_segment;
    printf("share_memory in %p\n\n", memory_segment);  

    (*buffer)->in = (*buffer)->out = 0;
    for (i = 0; i < BUFFER_SIZE; ++i){
	(*buffer)->messages[i].sender_id = 0;
        (*buffer)->messages[i].data[0] = '\0';
    }    
    /* TODO 1 : END                          */
    /*---------------------------------------*/

    printf("init buffer\n");
    return 0;
}

int attach_buffer(MessageBuffer **buffer) {
    /*---------------------------------------*/
    /* TODO 2 : attach buffer                */
    /* do not consider "no buffer situation" */
    if((shmid = shmget(SHM_KEY, sizeof(MessageBuffer), IPC_CREAT|0666)) == -1) return -1;
    if((memory_segment = shmat(shmid, NULL, 0)) == (void*)-1) return -1;
    printf("shared_memory in %p\n\n", memory_segment);  
    *buffer = (MessageBuffer*)memory_segment;
    
    /* TODO 2 : END                          */
    /*---------------------------------------*/

    printf("attach buffer\n");
    printf("\n");
    return 0;
}

int detach_buffer() {
    if (shmdt(memory_segment) == -1) {
        printf("shmdt error!\n\n");
        return -1;
    }

    printf("detach buffer\n\n");
    return 0;
}


int destroy_buffer() {
    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        printf("shmctl error!\n\n");
        return -1;
    }

    printf("destroy shared_memory\n\n");
    return 0;
}

int produce(MessageBuffer **buffer, int sender_id, char *data) {
    if (is_full(**buffer)) {
        printf("full!\n\n");
        return -1;
    }

    if (strlen(data) > 100) {
        printf("len(data) > 100\n\n");
        return -1;
    }

    /*---------------------------------------*/
    /* TODO 3 : produce message              */

    s_wait(semid);
    (*buffer)->messages[(*buffer)->in].sender_id = sender_id;
    strcpy((*buffer)->messages[(*buffer)->in].data , data);
    (*buffer)->in = ((*buffer)->in + 1) % BUFFER_SIZE;
    s_quit(semid);

    /* TODO 3 : END                          */
    /*---------------------------------------*/

    printf("produce message\n");
    return 0;
}

int consume(MessageBuffer **buffer, Message **message) {
    if (is_empty(**buffer)) {
        return -1;
    }

    /*---------------------------------------*/
    /* TODO 4 : consume message              */

    s_wait(semid);
    *message = (Message*)&((*buffer)->messages[(*buffer)->out]); 
    (*buffer)->out = ((*buffer)->out + 1) % BUFFER_SIZE;
    s_quit(semid);

    /* TODO 4 : END                          */
    /*---------------------------------------*/
    return 0;
}


int is_empty(MessageBuffer buffer) {
    /*---------------------------------------*/
    /* TODO 5 : is empty?                    */
    
    return buffer.in == buffer.out;

    /* TODO 5 : END                          */
    /*---------------------------------------*/
}

int is_full(MessageBuffer buffer) {
    /*---------------------------------------*/
    /* TODO 6 : is full?                     */
    
    return (buffer.in+1)%BUFFER_SIZE == buffer.out;

    /* TODO 6 : END                          */
    /*---------------------------------------*/
}

/* TODO 4 : END                                */
/*---------------------------------------------*/
