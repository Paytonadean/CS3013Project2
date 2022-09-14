#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
main() {
    sem_t mutex;
    createRegions();
    
    pthread_t largePlanes[15];
    pthread_t smallPlanes[30];
    createTheads();

}
function createRegions() {
    for (int region = 1, region < 7; region++) {
        sem_init(&mutex, 0, region);
    }
}
function createThreads() {
    printf("creating big planes\n");
    for (int i = 0; i < 15; i++) {
        int pthread_create(largePlanes[i],
            NULL,
            scheduler(),
            NULL);
    }
    printf("Creating small planes\n");
    for (int i = 0; i < 30; i++) {
        int pthread_create(smallPlanes[i],
            NULL,
            scheduler(),
            NULL);
    }
    printf("All planes ready to go\n");
}

//Create thread function
int pthread_create(pthread_t* thread,
    const pthread_attr_t* attr,
    void* (*start_routine)(void*),
    void* arg);


// Terminate thread
void pthread_exit(void* retval);

void* scheduler(void* arg) {
    bool 





}

