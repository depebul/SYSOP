#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define TIME_PATTERN "[%d:%d:%d]"
#define TM_TO_STRING(tmp) (tmp->tm_hour), (tmp->tm_min), (tmp->tm_sec)

// Globalne zmienne
int patients_waiting = 0, patients_remaining, available_cures = 6, pharmacist_waiting = 0, doctor_task = 0;
int patients_waiting_indices[3];

pthread_mutex_t waiting_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t remaining_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cures_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pharmacist_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t doctor_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t remaining_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cures_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t doctor_cond = PTHREAD_COND_INITIALIZER;

void *patient(void *arg)
{
    int id = *(int *)arg;

    // Idź do szpitala
    int wait_time = 2 + rand() % 4;
    time_t timestamp = time(NULL);
    struct tm *tmp = localtime(&timestamp);
    printf(TIME_PATTERN " - Pacjent(%d): idę do szpitala, będę za %d s\n", TM_TO_STRING(tmp), id, wait_time);
    sleep(wait_time);

    // Sprawdź miejsce w szpitalu
    pthread_mutex_lock(&waiting_mutex);
    while (patients_waiting == 3)
    {
        pthread_mutex_unlock(&waiting_mutex);
        wait_time = 2 + rand() % 4;
        timestamp = time(NULL);
        tmp = localtime(&timestamp);
        printf(TIME_PATTERN " - Pacjent(%d): za dużo pacjentów, wracam później za %d s\n", TM_TO_STRING(tmp), id, wait_time);
        sleep(wait_time);
        pthread_mutex_lock(&waiting_mutex);
    }

    // Wejdź do poczekalni
    patients_waiting_indices[patients_waiting] = id;
    patients_waiting++;
    timestamp = time(NULL);
    tmp = localtime(&timestamp);
    printf(TIME_PATTERN " - Pacjent(%d): czeka %d pacjentów na lekarza\n", TM_TO_STRING(tmp), id, patients_waiting);

    // Jeśli trzeci pacjent - obudź lekarza
    if (patients_waiting == 3)
    {
        pthread_mutex_unlock(&waiting_mutex);

        pthread_mutex_lock(&cures_mutex);
        while (available_cures < 3)
            pthread_cond_wait(&cures_cond, &cures_mutex);
        pthread_mutex_unlock(&cures_mutex);

        pthread_mutex_lock(&doctor_mutex);
        while (doctor_task != 0)
            pthread_cond_wait(&doctor_cond, &doctor_mutex);
        timestamp = time(NULL);
        tmp = localtime(&timestamp);
        printf(TIME_PATTERN " - Pacjent(%d): budzę lekarza\n", TM_TO_STRING(tmp), id);
        doctor_task = 1;
        pthread_cond_signal(&doctor_cond);
        pthread_mutex_unlock(&doctor_mutex);
    }
    else
    {
        pthread_mutex_unlock(&waiting_mutex);
    }

    // Czekaj na koniec wizyty
    pthread_mutex_lock(&remaining_mutex);
    int before = patients_remaining;
    while (patients_remaining == before)
        pthread_cond_wait(&remaining_cond, &remaining_mutex);
    pthread_mutex_unlock(&remaining_mutex);

    timestamp = time(NULL);
    tmp = localtime(&timestamp);
    printf(TIME_PATTERN " - Pacjent(%d): kończę wizytę\n", TM_TO_STRING(tmp), id);
    return NULL;
}

void *pharmacist(void *arg)
{
    int id = *(int *)arg;

    // Idź do szpitala
    int wait_time = 5 + rand() % 11;
    time_t timestamp = time(NULL);
    struct tm *tmp = localtime(&timestamp);
    printf(TIME_PATTERN " - Farmaceuta(%d): idę do szpitala, będę za %d s\n", TM_TO_STRING(tmp), id, wait_time);
    sleep(wait_time);

    pthread_mutex_lock(&cures_mutex);
    if (available_cures >= 3)
    {
        pthread_mutex_unlock(&cures_mutex);
        return NULL;
    }

    pthread_mutex_lock(&pharmacist_mutex);
    while (pharmacist_waiting)
    {
        pthread_mutex_unlock(&pharmacist_mutex);
        timestamp = time(NULL);
        tmp = localtime(&timestamp);
        printf(TIME_PATTERN " - Farmaceuta(%d): czekam na opróżnienie apteczki\n", TM_TO_STRING(tmp), id);
        pthread_cond_wait(&cures_cond, &cures_mutex);
        pthread_mutex_lock(&pharmacist_mutex);
    }
    pharmacist_waiting = 1;
    pthread_mutex_unlock(&pharmacist_mutex);
    pthread_mutex_unlock(&cures_mutex);

    // Obudź lekarza
    pthread_mutex_lock(&doctor_mutex);
    while (doctor_task != 0)
        pthread_cond_wait(&doctor_cond, &doctor_mutex);
    timestamp = time(NULL);
    tmp = localtime(&timestamp);
    printf(TIME_PATTERN " - Farmaceuta(%d): budzę lekarza\n", TM_TO_STRING(tmp), id);
    doctor_task = 2;
    pthread_cond_signal(&doctor_cond);
    pthread_mutex_unlock(&doctor_mutex);

    timestamp = time(NULL);
    tmp = localtime(&timestamp);
    printf(TIME_PATTERN " - Farmaceuta(%d): dostarczam leki\n", TM_TO_STRING(tmp), id);

    // Czekaj na dostawę
    pthread_mutex_lock(&cures_mutex);
    while (available_cures < 6)
        pthread_cond_wait(&cures_cond, &cures_mutex);
    pthread_mutex_unlock(&cures_mutex);

    pthread_mutex_lock(&pharmacist_mutex);
    pharmacist_waiting = 0;
    pthread_mutex_unlock(&pharmacist_mutex);

    timestamp = time(NULL);
    tmp = localtime(&timestamp);
    printf(TIME_PATTERN " - Farmaceuta(%d): zakończyłem dostawę\n", TM_TO_STRING(tmp), id);
    return NULL;
}

void *doctor(void *arg)
{
    pthread_mutex_lock(&doctor_mutex);
    while (1)
    {
        // Śpij
        while (doctor_task == 0)
        {
            time_t timestamp = time(NULL);
            struct tm *tmp = localtime(&timestamp);
            printf(TIME_PATTERN " - Lekarz: zasypiam\n", TM_TO_STRING(tmp));
            pthread_cond_wait(&doctor_cond, &doctor_mutex);
        }

        time_t timestamp = time(NULL);
        struct tm *tmp = localtime(&timestamp);
        printf(TIME_PATTERN " - Lekarz: budzę się\n", TM_TO_STRING(tmp));

        if (doctor_task == 1)
        { // Lecz pacjentów
            pthread_mutex_lock(&waiting_mutex);
            timestamp = time(NULL);
            tmp = localtime(&timestamp);
            printf(TIME_PATTERN " - Lekarz: konsultuję pacjentów %d", TM_TO_STRING(tmp), patients_waiting_indices[0]);
            for (int i = 1; i < patients_waiting; i++)
            {
                printf(", %d", patients_waiting_indices[i]);
            }
            printf("\n");
            patients_waiting = 0;
            pthread_mutex_unlock(&waiting_mutex);

            sleep(2 + rand() % 3);

            pthread_mutex_lock(&cures_mutex);
            available_cures -= 3;
            if (available_cures < 3)
                pthread_cond_signal(&cures_cond);
            pthread_mutex_unlock(&cures_mutex);

            pthread_mutex_lock(&remaining_mutex);
            patients_remaining -= 3;
            if (patients_remaining == 0)
            {
                pthread_mutex_unlock(&remaining_mutex);
                pthread_cond_broadcast(&remaining_cond);
                return NULL;
            }
            pthread_cond_broadcast(&remaining_cond);
            pthread_mutex_unlock(&remaining_mutex);
        }
        else if (doctor_task == 2)
        { // Uzupełnij leki
            timestamp = time(NULL);
            tmp = localtime(&timestamp);
            printf(TIME_PATTERN " - Lekarz: przyjmuję dostawę leków\n", TM_TO_STRING(tmp));
            sleep(1 + rand() % 3);

            pthread_mutex_lock(&cures_mutex);
            available_cures = 6;
            pthread_cond_broadcast(&cures_cond);
            pthread_mutex_unlock(&cures_mutex);
        }
        doctor_task = 0;
        pthread_cond_broadcast(&doctor_cond);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Nieprawidłowa liczba argumentów\n");
        return 1;
    }

    int patient_count = atoi(argv[1]);
    int pharmacist_count = atoi(argv[2]);
    if (patient_count < 1 || pharmacist_count < 1)
    {
        printf("Liczba pacjentów i farmaceutów musi być dodatnia\n");
        return 2;
    }

    patients_remaining = patient_count;

    pthread_t threads[patient_count + pharmacist_count + 1];
    int ids[patient_count + pharmacist_count];

    // Lekarz
    pthread_create(&threads[0], NULL, doctor, NULL);

    // Pacjenci
    for (int i = 0; i < patient_count; i++)
    {
        ids[i] = i + 1;
        pthread_create(&threads[i + 1], NULL, patient, &ids[i]);
    }

    // Farmaceuci
    for (int i = 0; i < pharmacist_count; i++)
    {
        ids[patient_count + i] = i + 1;
        pthread_create(&threads[patient_count + i + 1], NULL, pharmacist, &ids[patient_count + i]);
    }

    // Czekaj na zakończenie
    pthread_mutex_lock(&remaining_mutex);
    while (patients_remaining > 0)
        pthread_cond_wait(&remaining_cond, &remaining_mutex);
    pthread_mutex_unlock(&remaining_mutex);

    time_t timestamp = time(NULL);
    struct tm *tmp = localtime(&timestamp);
    printf(TIME_PATTERN " - Wszyscy pacjenci zostali uleczeni\n", TM_TO_STRING(tmp));

    return 0;
}