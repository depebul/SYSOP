#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>

#define MINTIME_PATIENT 2
#define MAXTIME_PATIENT 5
#define MINTIME_PHARMACIST 5
#define MAXTIME_PHARMACIST 15

#define MINTIME_VISIT 2
#define MAXTIME_VISIT 4

#define MINTIME_PHARMACIST_VISIT 1
#define MAXTIME_PHARMACIST_VISIT 3

#define MAX_PATIENTS 3
#define MAX_MEDS 6

int patients_in_hospital_num = 0;
int patient_in_hospital[3] = {0};
pthread_mutex_t patient_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t patient_cond = PTHREAD_COND_INITIALIZER;

int meds = 0;
pthread_mutex_t med_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t med_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t pharmacist_mut = PTHREAD_MUTEX_INITIALIZER;

int is_visit = 0;
int patients_in_visit = 0;
pthread_mutex_t visit_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t visit_cond = PTHREAD_COND_INITIALIZER;

int initial_time;

int rand_int(int min, int max)
{
    return min + (rand() % (max + 1 - min));
}

void patient_msg(int id, char *message)
{

    printf("[%ld] - %s(%d): %s\n", time(NULL), "Pacjent", id, message);
}

void pharmacist_msg(int id, char *message)
{

    printf("[%ld] - %s(%d): %s\n", time(NULL), "Farmaceuta", id, message);
}

void doctor_msg(char *message)
{
    printf("[%ld] - %s: %s\n", time(NULL), "Doktor", message);
}

void *patient(void *arg)
{
    int id = *(int *)arg;
    char msg[64];
    int wait = rand_int(MINTIME_PATIENT, MAXTIME_PATIENT);
    sprintf(msg, "ide do szpitala, bede za %d s", wait);
    patient_msg(id, msg);
    sleep(wait);
    while (1)
    {
        pthread_mutex_lock(&patient_mut);
        if (patients_in_hospital_num < MAX_PATIENTS)
        {
            patient_in_hospital[patients_in_hospital_num] = id;
            patients_in_hospital_num++;
            break;
        }
        int wait_time = rand_int(MINTIME_PATIENT, MAXTIME_PATIENT);
        sprintf(msg, "za dużo pacjentów, wracam później za %d s", wait_time);
        // patient_msg(id, msg);
        pthread_mutex_unlock(&patient_mut);
        sleep(wait_time);
    }
    sprintf(msg, "czeka %d pacjentów na lekarza", patients_in_hospital_num);
    patient_msg(id, msg);
    if (patients_in_hospital_num == MAX_PATIENTS)
    {
        pthread_mutex_unlock(&patient_mut);
        pthread_mutex_lock(&med_mut);
        while (meds < MAX_PATIENTS)
        {
            pthread_cond_wait(&med_cond, &med_mut);
        }
        pthread_mutex_unlock(&med_mut);
        pthread_mutex_lock(&visit_mut);
        while (is_visit || patients_in_visit != 0)
        {
            pthread_cond_wait(&visit_cond, &visit_mut);
        }
        patient_msg(id, "budzę lekarza");
        is_visit = 1;
        pthread_cond_broadcast(&visit_cond);
    }
    else
    {
        pthread_mutex_unlock(&patient_mut);
    }

    while (is_visit || patients_in_hospital_num != 0)
    {
        pthread_cond_wait(&visit_cond, &visit_mut);
    }
    patients_in_visit--;
    patient_msg(id, "kończę wizytę");
    pthread_cond_broadcast(&visit_cond);
    pthread_mutex_unlock(&visit_mut);
    return NULL;
}

void *pharmacist(void *arg)
{
    int id = *(int *)arg;
    char msg[64];
    while (1)
    {

        int wait = rand_int(MINTIME_PHARMACIST, MAXTIME_PHARMACIST);
        sprintf(msg, "ide do szpitala, bede za %d s", wait);
        pharmacist_msg(id, msg);
        sleep(wait);
        pthread_mutex_lock(&med_mut);
        if (meds >= MAX_PATIENTS)
        {
            pharmacist_msg(id, "czekam na oproznienie apteczki");
        }
        pthread_mutex_unlock(&med_mut);
        pthread_mutex_lock(&pharmacist_mut);
        pthread_mutex_lock(&med_mut);
        while (meds >= MAX_PATIENTS)
        {
            pthread_cond_wait(&med_cond, &med_mut);
        }
        pthread_mutex_lock(&visit_mut);
        while (is_visit || patients_in_visit != 0)
        {
            pthread_cond_wait(&visit_cond, &visit_mut);
        }
        pharmacist_msg(id, "budzę lekarza");
        is_visit = 1;
        pthread_cond_broadcast(&visit_cond);
        pharmacist_msg(id, "dostarczam leki");
        pthread_mutex_unlock(&med_mut);
        while (is_visit)
        {
            pthread_cond_wait(&visit_cond, &visit_mut);
        }
        pthread_mutex_unlock(&visit_mut);
        pthread_mutex_unlock(&pharmacist_mut);
        pharmacist_msg(id, "zakończyłem dostawę");
    }
    return NULL;
}

void *doctor(void *arg)
{
    char msg[64];
    while (1)
    {
        if (meds >= MAX_PATIENTS)
        {
            pthread_mutex_lock(&visit_mut);
            while (!is_visit)
            {
                pthread_cond_wait(&visit_cond, &visit_mut);
            }
            doctor_msg("budzę się");
            sprintf(msg, "konsultuję pacjentów %d, %d, %d", patient_in_hospital[0], patient_in_hospital[1], patient_in_hospital[2]);
            doctor_msg(msg);
            int time = rand_int(MINTIME_VISIT, MAXTIME_VISIT);
            sleep(time);

            pthread_mutex_lock(&med_mut);
            meds -= 3;
            pthread_cond_broadcast(&med_cond);
            pthread_mutex_unlock(&med_mut);

            pthread_mutex_lock(&patient_mut);
            patients_in_hospital_num = 0;
            patients_in_visit = 3;
            pthread_cond_broadcast(&patient_cond);
            pthread_mutex_unlock(&patient_mut);

            doctor_msg("zasypiam");
            is_visit = 0;
            pthread_cond_broadcast(&visit_cond);
            pthread_mutex_unlock(&visit_mut);

            pthread_mutex_lock(&med_mut);
            pthread_mutex_unlock(&med_mut);
        }
        else
        {
            pthread_mutex_lock(&visit_mut);
            while (!is_visit)
            {
                pthread_cond_wait(&visit_cond, &visit_mut);
            }
            pthread_mutex_lock(&med_mut);
            doctor_msg("przyjmuję dostawę leków");
            meds = MAX_MEDS;
            pthread_cond_broadcast(&med_cond);
            pthread_mutex_unlock(&med_mut);
            int time = rand_int(MINTIME_PHARMACIST_VISIT, MAXTIME_PHARMACIST_VISIT);
            sleep(time);
            doctor_msg("zasypiam");
            is_visit = 0;
            pthread_cond_broadcast(&visit_cond);
            pthread_mutex_unlock(&visit_mut);
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Podaj liczbę pacjentów i farmaceutów");
        return 1;
    }

    int patient_num = atoi(argv[1]);
    int pharmacist_num = atoi(argv[2]);

    srand(time(NULL));
    initial_time = time(NULL);
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, doctor, NULL) != 0)
    {
        printf("Failed to create doctor. errno: %d", errno);
        return errno;
    }
    for (int i = 0; i < patient_num; i++)
    {
        pthread_t thread_id;
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&thread_id, NULL, patient, (void *)id);
    }
    for (int i = 0; i < pharmacist_num; i++)
    {
        pthread_t thread_id;
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&thread_id, NULL, pharmacist, (void *)id);
    }

    while (1)
    {
    }

    return 0;
}
