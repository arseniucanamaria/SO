#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "a2_helper.h"
#include <semaphore.h>

typedef struct
{
    int pn; //PROCESS NUMBER
    int tn; //THREAD NUMBER
} TH_STRUCT;

sem_t *s1, *s2, *s3, *s4,*s5; //five semaphores
void *functie(void *arg)
{
    TH_STRUCT *p = (TH_STRUCT *)arg;
    if ((p->pn == 6) && (p->tn == 4 || p->tn == 2))
    {
        if (p->tn == 4)
        {
            info(BEGIN, p->pn, p->tn);
            sem_post(s1); //perm+1
            sem_wait(s2);
            info(END, p->pn, p->tn);
        }
        if (p->tn == 2)
        {
            sem_wait(s1); 
            info(BEGIN, p->pn, p->tn);

            info(END, p->pn, p->tn);
            sem_post(s2);
        }
    }
    else
    {
        if ((p->pn == 3 && p->tn ==2) ||
        (p->pn == 3 && p->tn ==5) || (p->pn == 6 && p->tn ==1))
        {
            if(p->pn == 3 && p->tn ==2){
                info(BEGIN, p->pn, p->tn);
                info(END, p->pn, p->tn);
                sem_post(s3);
            }
            if(p->pn == 6 && p->tn ==1)
            {
                sem_wait(s3);
                info(BEGIN, p->pn, p->tn);
                info(END, p->pn, p->tn); 
                sem_post(s4);
            }
            if(p->pn == 3 && p->tn ==5)
            {
                sem_wait(s4);
                info(BEGIN, p->pn, p->tn);
                info(END, p->pn, p->tn); 
            }

        }
        else
        {
            if(p->pn==2)
            {
                sem_wait(s5);
                info(BEGIN, p->pn, p->tn);
                info(END, p->pn, p->tn);
                sem_post(s5);

            }
            else
            {
                info(BEGIN, p->pn, p->tn);
                info(END, p->pn, p->tn);
            }
            
            
        }
    }

    return NULL;
}
void creare_threaduri(int numar_process, int numar_threaduri)
{
    TH_STRUCT params[numar_threaduri];
    pthread_t tids[numar_threaduri];
    int i;

    for (i = 0; i < numar_threaduri; i++)
    {
        params[i].pn = numar_process;
        params[i].tn = i + 1;
        pthread_create(&tids[i], NULL, functie, &params[i]);
    }

    for (i = 0; i < numar_threaduri; i++)
    {
        pthread_join(tids[i], 0);
    }
    return;
}

int main()
{
//stergere semafoare in caz ca existau de dinainte
    unlink("s1");
    unlink("s2");
    unlink("s3");
    unlink("s4");
    unlink("s5");
    s1 = sem_open("s1", O_CREAT, 0644, 0);
    s2 = sem_open("s2", O_CREAT, 0644, 0);
    s3 = sem_open("s3", O_CREAT, 0644, 0);
    s4 = sem_open("s4", O_CREAT, 0644, 0);
    s5 = sem_open("s5", O_CREAT, 0644, 5);

    init();
    info(BEGIN, 1, 0);
    pid_t pid2 = -1, pid3 = -1, pid4 = -1, pid5 = -1, pid6 = -1, pid7 = -1;

    pid2 = fork();
    if (pid2 == 0)
    {
        info(BEGIN, 2, 0);
        creare_threaduri(2, 46);
        pid3 = fork();
        if (pid3 == 0)
        {
            info(BEGIN, 3, 0);
            creare_threaduri(3, 5);
            info(END, 3, 0);
        }
        else
        {
            pid4 = fork();
            if (pid4 == 0)
            {
                info(BEGIN, 4, 0);

                info(END, 4, 0);
            }

            else
            {
                pid6 = fork();
                if (pid6 == 0)
                {
                    info(BEGIN, 6, 0);
                    creare_threaduri(6, 5);
                    info(END, 6, 0);
                }
                else
                {
                    waitpid(pid3, 0, 0);
                    waitpid(pid4, 0, 0);
                    waitpid(pid6, 0, 0);
                    info(END, 2, 0);
                }
            }
        }
    }
    else
    {
        pid5 = fork();
        if (pid5 == 0)
        {
            info(BEGIN, 5, 0);
            pid7 = fork();
            if (pid7 == 0)
            {
                info(BEGIN, 7, 0);

                info(END, 7, 0);
            }
            else
            {
                waitpid(pid7, 0, 0);
                info(END, 5, 0);
            }
        }
        else
        {
            waitpid(pid2, 0, 0);
            waitpid(pid5, 0, 0);
            info(END, 1, 0);
        }
    }
    return 0;
}
