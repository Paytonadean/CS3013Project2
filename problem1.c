#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h> 

//mutexes
//only a pair is needed for soccer
//not required to keep track of teams
//baseball exactly 18 players, extra must wait
//football exactly 22 players
//soccer any even number of players up to 22

//field constants
typedef enum sport {
    BASEBALL,
    FOOTBALL,
    SOCCER
} sport_t;

int TOTALPLAYERS = 36 + 44 + 44; //is mutable for different test cases 
int maxNumBaseball = 36;
int maxNumFootball = 44;
int maxNumSoccer = 44;
int numBaseballReady = 0;
int numFootballReady = 0;
int numSoccerReady = 0;
int fieldStatus = 0;
int fieldCount = 0;
// fieldStatus 0 means not in use
// 1 means baseball using
// 2 means football using
// 3 means soccer using
// 4 means baseball just played but is not playing now
// 5 means football just played but is not playing now
// 6 means soccer just played but is not playing now
pthread_mutex_t fieldLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t joinFieldLock = PTHREAD_MUTEX_INITIALIZER;

//TYPE justPlayed;

//player constants
pthread_mutex_t baseballPlayerLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t baseballPlayerCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t baseballFieldCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t footballPlayerLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t footballPlayerCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t footballFieldCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t soccerPlayerLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t soccerPlayerCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t soccerFieldCond = PTHREAD_COND_INITIALIZER;

sigset_t baseballSet; 
sigset_t footballSet;

int baseballSignal = 2;
int footballSignal = 12; 

typedef struct Player {
    pthread_t threadID; 
    sport_t playerType;
} Player;

void playerFunction(sport_t arg) {
    //player entering the field
    //int timesPlayed;
    //printf("I got to the beginning of playerFunction.\n");
    sleep(rand() % 5 + 1); //one to five seconds 
    //sleep(2);
    //printf("I got to the second part of playerFunction.\n");
    //printf("I am a %u type player... my ID is %u.\n", arg, pthread_self());
    tryControlField(arg); 
    printf("I got to the end of playerFunction.\n");
    printf("I am a %u type player, exiting now... my ID is %u.\n", arg, pthread_self());
}

void sigHandler(int signo) {
    if (signo = SIGUSR1) {
        //baseball
    } else if (signo = SIGUSR2) {
        //football
    } 
    //add another for soccer
}

void tryControlField (sport_t mySport) {
    sport_t player = mySport;
    printf("I made it insde the switch statement! I am a %u type player.\n", player);
    switch (player)
    {
    case BASEBALL:
        printf("I made it insde the BASEBALL case! I am a %u type player.\n", player);
        pthread_mutex_lock(&baseballPlayerLock); //baseballPlayerLock locked
        printf("I made it inside baseballPlayerLock! I am a %u type player.\n", player);
        pthread_mutex_lock(&fieldLock); //fieldLock locked
        //printf("I made it inside fieldLock! I am a %u type player.\n", player);
        printf("The field status is initially %d.\n", fieldStatus);
        if ((fieldStatus == 0) || (fieldStatus == 5) || (fieldStatus == 6)) {
            numBaseballReady++;
            printf("There are %d baseball players ready.\n", numBaseballReady);
            printf("The field status is now %d.\n", fieldStatus);
            if (numBaseballReady == 18) {
                if ((fieldStatus == 2) || (fieldStatus == 3)) {
                    pthread_mutex_unlock(&fieldLock);
                    tryControlField(player);
                }
                fieldStatus = 1; //playing baseball
                printf("The field is now being used by baseball players!\n");
                printf("The field status is %d.\n", fieldStatus);
                //wake up all ready sleeping baseball players
                pthread_mutex_unlock(&fieldLock);
                pthread_mutex_unlock(&baseballPlayerLock);
                pthread_cond_broadcast(&baseballPlayerCond);
                printf("Wake up, baseball players!\n");
                //numBaseballReady--; prevent duplicate ready
                tryControlField(player);
            } else {
                printf("I'm waiting to play baseball.\n");
                //while (fieldStatus != 1) {
                    pthread_mutex_unlock(&fieldLock);
                    pthread_cond_wait(&baseballPlayerCond, &baseballPlayerLock); //sleep until enough players
                    pthread_mutex_unlock(&baseballPlayerLock);
                //}
                printf("Baseball awakens me!\n");
                tryControlField(player); //run control field function again
            }
        } else if (fieldStatus == 1) {
            pthread_mutex_unlock(&baseballPlayerLock);
            pthread_mutex_unlock(&fieldLock);
            printf("I made it outside joinFieldLock! I am a %u type player.\n", player);
            pthread_mutex_lock(&joinFieldLock); //joinFieldLock locked
            printf("I made it inside joinFieldLock! I am a %u type player.\n", player);
            if (fieldCount < 17) {
                fieldCount++;
                printf("I am a baseball player joining the field in position %d with ID %u.\n", fieldCount, pthread_self());
                pthread_cond_wait(&baseballFieldCond, &joinFieldLock);
                // players join the game 1 by 1
                printf("I am a baseball player leaving the field in position %d with ID %u.\n", fieldCount, pthread_self());
                fieldCount--;
                //fieldStatus = 4; played baseball, past tense
                //wake up all ready sleeping football and soccer players(?)
                pthread_mutex_unlock(&joinFieldLock); //joinFieldLock unlocked
                sleep(rand() % 5 + 4); //4 to 8 seconds
                tryControlField(player);
            } else if (fieldCount == 17) {
                fieldCount++;
                printf("I am a baseball player joining the field in position %d with ID %u.\n", fieldCount, pthread_self());
                printf("We have enough for a baseball game! Starting game now.");
                sleep(rand() % 5 + 4); //4 to 8 seconds
                pthread_cond_broadcast(&baseballFieldCond);
                fieldStatus = 4;
                fieldCount--;
                printf("I am a baseball player leaving the field in position %d with ID %u.\n", fieldCount, pthread_self());
                pthread_mutex_unlock(&joinFieldLock);
                printf("The field is no longer being used by baseball players!\n");
                sleep(rand() % 5 + 4); //4 to 8 seconds
                tryControlField(player);
            } else if (fieldCount >= 18 ) {
                pthread_mutex_unlock(&fieldLock);
                pthread_cond_wait(&baseballFieldCond, &joinFieldLock);
                pthread_mutex_unlock(&joinFieldLock); //joinFieldLock unlocked
                tryControlField(player);
            }
            pthread_mutex_unlock(&fieldLock);
        } else if (fieldStatus == 4) {
            pthread_mutex_lock(&footballPlayerLock);
            pthread_mutex_lock(&soccerPlayerLock);
            if ((numFootballReady == 0) && (numSoccerReady == 0)) {
                fieldStatus = 1;
                printf("The field is now being used by baseball players!\n");
                //wake up all ready sleeping baseball players
                pthread_cond_broadcast(&baseballPlayerCond);
                pthread_mutex_unlock(&soccerPlayerLock);
                pthread_mutex_unlock(&footballPlayerLock);
                tryControlField(player);
            } else {
                pthread_mutex_unlock(&soccerPlayerLock);
                pthread_mutex_unlock(&footballPlayerLock);
                pthread_cond_broadcast(&footballPlayerCond);
                pthread_cond_broadcast(&soccerPlayerCond);
                pthread_mutex_unlock(&fieldLock);
                pthread_cond_wait(&baseballPlayerCond, &baseballPlayerLock);
                tryControlField(player);
            }
        } else if ((fieldStatus == 2) || (fieldStatus == 3)) {
                printf("HEY Baseball\n");
                if (numBaseballReady < 17) {
                    numBaseballReady++;;
                }
            pthread_cond_wait(&baseballPlayerCond, &fieldLock);
            tryControlField(player);
        }
        pthread_mutex_unlock(&fieldLock); //fieldLock unlocked
        pthread_mutex_unlock(&baseballPlayerLock); //baseballPlayerLock unlocked
        printf("I got to the end of BASEBALL.\n");
        break;
    case FOOTBALL:
        //printf("I made it insde the FOOTBALL case! I am a %u type player.\n", player);
        pthread_mutex_lock(&footballPlayerLock); //footballPlayerLock locked
        printf("I made it inside footballPlayerLock! I am a %u type player.\n", player);
        pthread_mutex_lock(&fieldLock); //footballPlayerLock locked
        //printf("I made it inside fieldLock! I am a %u type player.\n", player);
        printf("The field status is initially %d.\n", fieldStatus);
        if ((fieldStatus == 0) || (fieldStatus == 4) || (fieldStatus == 6)) {
            numFootballReady++;
            printf("There are %d football players ready.\n", numFootballReady);
            printf("The field status is now %d.\n", fieldStatus);
            if (numFootballReady == 22) {
                if ((fieldStatus == 1) || (fieldStatus == 3)) {
                    pthread_mutex_unlock(&fieldLock);
                    tryControlField(player);
                }
                fieldStatus = 2; //playing football            
                printf("The field is now being used by football players!\n");
                printf("The field status is %d.\n", fieldStatus);
                //wake up all ready sleeping football players
                pthread_mutex_unlock(&fieldLock);
                pthread_mutex_unlock(&footballPlayerLock);
                pthread_cond_broadcast(&footballPlayerCond);
                printf("Wake up, football players!\n");
                //numFootballReady--; //prevent duplicate ready
                tryControlField(player);
            } else {
                printf("I'm waiting to play football.\n");
                //while (fieldStatus != 2) {
                    pthread_mutex_unlock(&fieldLock);
                    pthread_cond_wait(&footballPlayerCond, &footballPlayerLock); //sleep until enough players
                    pthread_mutex_unlock(&footballPlayerLock);
                //}
                printf("Football awakens me!\n");
                tryControlField(player); //run control field function again
            }
        } else if (fieldStatus == 2) {
            pthread_mutex_unlock(&fieldLock);
            pthread_mutex_unlock(&footballPlayerLock);
            pthread_mutex_lock(&joinFieldLock); //joinFieldLock locked
            printf("I made it inside joinFieldLock! I am a %u type player.\n", player);
            if (fieldCount < 21) {
                fieldCount++;
                printf("I am a football player joining the field in position %d with ID %u.\n", fieldCount, pthread_self());
                pthread_cond_wait(&footballFieldCond, &joinFieldLock);
                //players join the game 1 by 1
                printf("I am a football player leaving the field in position %d with ID %u.\n", fieldCount, pthread_self());
                fieldCount--;
                //fieldStatus = 5; played football, past tense
                //wake up all ready sleeping baseball and soccer players(?)
                pthread_mutex_unlock(&joinFieldLock); //joinFieldLock unlocked
                sleep(rand() % 5 + 4); //4 to 8 seconds
                tryControlField(player);
            } else if (fieldCount == 21) {
                fieldCount++;
                printf("I am a football player joining the field in position %d with ID %u.\n", fieldCount, pthread_self());
                printf("We have enough for a football game! Starting game now.");
                sleep(rand() % 5 + 4); //4 to 8 seconds
                pthread_cond_broadcast(&footballFieldCond);
                fieldStatus = 5;
                fieldCount--;
                printf("I am a football player leaving the field in position %d with ID %u.\n", fieldCount, pthread_self());
                pthread_mutex_unlock(&joinFieldLock);
                printf("The field is no longer being used by football players!\n");
                sleep(rand() % 5 + 4); //4 to 8 seconds
                tryControlField(player);
            } else if (fieldCount >= 22) {
                pthread_mutex_unlock(&fieldLock);
                pthread_cond_wait(&footballFieldCond, &joinFieldLock);
                pthread_mutex_unlock(&joinFieldLock); //joinFieldLock unlocked
                tryControlField(player);
            }
        } else if (fieldStatus == 5) {
            pthread_mutex_lock(&soccerPlayerLock);
            pthread_mutex_lock(&baseballPlayerLock);
            if ((numSoccerReady == 0) && (numBaseballReady == 0)) {
                fieldStatus = 2;
                printf("The field is now being used by football players!\n");
                //wake up all ready sleeping football players
                pthread_cond_broadcast(&footballPlayerCond);
                pthread_mutex_unlock(&baseballPlayerLock);
                pthread_mutex_unlock(&soccerPlayerLock);
                tryControlField(player);
            } else {
                pthread_mutex_unlock(&baseballPlayerLock);
                pthread_mutex_unlock(&soccerPlayerLock);
                pthread_cond_broadcast(&soccerPlayerCond);
                pthread_cond_broadcast(&baseballPlayerCond);
                pthread_mutex_unlock(&fieldLock);
                pthread_cond_wait(&footballPlayerCond, &footballPlayerLock);
                tryControlField(player);
            }
            pthread_mutex_unlock(&baseballPlayerLock);
            pthread_mutex_unlock(&soccerPlayerLock);
            tryControlField(player);
        } else if ((fieldStatus == 1) || (fieldStatus == 3)) {
                printf("HEY Football\n");
                if (numFootballReady < 21) {
                    numFootballReady++;;
                }
            pthread_cond_wait(&footballPlayerCond, &fieldLock);
            tryControlField(player);
        }
        pthread_mutex_unlock(&fieldLock); //fieldLock unlocked
        pthread_mutex_unlock(&footballPlayerLock);  //footballPlayerLock unlocked
        printf("I got to the end of FOOTBALL.\n");
        break;
    case SOCCER:
        //printf("I made it insde the SOCCER case! I am a %u type player.\n", player);
        pthread_mutex_lock(&soccerPlayerLock); //soccerPlayerLock locked
        printf("I made it inside soccerPlayerLock! I am a %u type player.\n", player);
        pthread_mutex_lock(&fieldLock); //fieldLock locked
        //printf("I made it inside fieldLock! I am a %u type player.\n", player);
        printf("The field status is initially %d.\n", fieldStatus);
        if ((fieldStatus == 0) || (fieldStatus == 4) || (fieldStatus == 5)) {
            numSoccerReady++;
            printf("There are %d soccer players ready.\n", numSoccerReady);
            printf("The field status is now %d.\n", fieldStatus);
            if (numSoccerReady == 22) {
                if ((fieldStatus == 1) || (fieldStatus == 2)) {
                    pthread_mutex_unlock(&fieldLock);
                    tryControlField(player);
                }
                fieldStatus = 3; //playing soccer
                printf("The field is now being used by soccer players!\n");
                printf("The field status is %d.\n", fieldStatus);
                //wake up all ready sleeping soccer players
                pthread_mutex_unlock(&fieldLock);
                pthread_mutex_unlock(&soccerPlayerLock);
                printf("Wake up, soccer players!\n");
                pthread_cond_broadcast(&soccerPlayerCond);
                //numSoccerReady--; //prevent duplicate ready
                tryControlField(player);
            } else {
                printf("I'm waiting to play soccer.\n");
                //while (fieldStatus != 3) {
                pthread_mutex_unlock(&fieldLock);
                pthread_cond_wait(&soccerPlayerCond, &soccerPlayerLock); //sleep until enough players
                pthread_mutex_unlock(&soccerPlayerLock);
                //}
                printf("Soccer awakens me!\n");
                tryControlField(player); //run control field function again
            }
        } else if (fieldStatus == 3) {
            pthread_mutex_unlock(&fieldLock);
            pthread_mutex_unlock(&soccerPlayerLock);
            printf("I made it outside joinFieldLock! I am a %u type player.\n", player);
            pthread_mutex_lock(&joinFieldLock); //joinFieldLock locked
            printf("I made it inside joinFieldLock! I am a %u type player.\n", player);
            if (fieldCount < 21) {
                pthread_mutex_lock(&fieldLock);
                if (fieldStatus == 6) {
                    printf("I'm a soccer player, but I shouldn't be selfish.\n");
                    pthread_mutex_unlock(&fieldLock);
                    tryControlField(player);
                }
                pthread_mutex_unlock(&fieldLock);
                fieldCount++;
                printf("I am a soccer player joining the field in position %d with ID %u.\n", fieldCount, pthread_self());
                pthread_cond_wait(&soccerFieldCond, &joinFieldLock);
                // players join the game 1 by 1
                printf("I am a soccer player leaving the field in position %d with ID %u.\n", fieldCount, pthread_self());
                fieldCount--;
                //fieldStatus = 6; played soccer, past tense
                //wake up all ready sleeping baseball and football players(?)
                pthread_mutex_unlock(&joinFieldLock); //joinFieldLock unlocked
                sleep(rand() % 5 + 4); //4 to 8 seconds
                tryControlField(player);
            } else if (fieldCount == 21) {
                fieldCount++;
                printf("I am a soccer player joining the field in position %d with ID %u.\n", fieldCount, pthread_self());
                printf("We have enough for a soccer game! Starting game now.");
                sleep(rand() % 5 + 4); //4 to 8 seconds
                pthread_cond_broadcast(&soccerFieldCond);
                printf("The field is no longer being used by soccer players!\n");
                pthread_mutex_lock(&fieldLock);
                fieldStatus = 6;
                printf("Field Status: %u\n", fieldStatus);
                pthread_mutex_unlock(&fieldLock);
                printf("I am a soccer player leaving the field in position %d with ID %u.\n", fieldCount, pthread_self());
                fieldCount --;
                pthread_mutex_unlock(&joinFieldLock);
                sleep(rand() % 5 + 4); //4 to 8 seconds
                tryControlField(player);
            } else if (fieldCount >= 22) {
                //pthread_mutex_unlock(&fieldLock);
                pthread_cond_wait(&soccerFieldCond, &joinFieldLock);
                pthread_mutex_unlock(&joinFieldLock); //joinFieldLock unlocked
                tryControlField(player);
            }
        } else if (fieldStatus == 6) {
            pthread_mutex_lock(&baseballPlayerLock);
            pthread_mutex_lock(&footballPlayerLock);
            if ((numBaseballReady == 0) && (numFootballReady == 0)) {
                fieldStatus = 3;
                printf("The field is now being used by soccer players!\n");
                //wake up all ready sleeping soccer players
                pthread_cond_broadcast(&soccerPlayerCond);
                pthread_mutex_unlock(&footballPlayerLock);
                pthread_mutex_unlock(&baseballPlayerLock);
                tryControlField(player);
            } else {
                pthread_mutex_unlock(&footballPlayerLock);
                pthread_mutex_unlock(&baseballPlayerLock);
                pthread_cond_broadcast(&baseballPlayerCond);
                pthread_cond_broadcast(&footballPlayerCond);
                pthread_mutex_unlock(&fieldLock);
                pthread_cond_wait(&soccerPlayerCond, &soccerPlayerLock);
                tryControlField(player);
            }
        } else if ((fieldStatus == 1) || (fieldStatus = 2)) {
                printf("HEY Soccer\n");
                if (numSoccerReady <= 22) {
                    numSoccerReady++;;
                }
            pthread_cond_wait(&soccerPlayerCond, &fieldLock);
            tryControlField(player);
        }
        pthread_mutex_unlock(&fieldLock); //fieldLock unlocked
        pthread_mutex_unlock(&soccerPlayerLock); //soccerPlayerLock unlocked
        printf("I got to the end of SOCCER.\n");
        break;
    default:
        printf("I got to the end of the switch.\n");
        break;
    }

}

/*
void baseballWatcher() {
    if (fieldStatus == 0) {
        if (justPlayed == BASEBALL) {
            baseballWatcher();
        }
    } else if (fieldStatus == 2 || 3) {
        pthread_mutex_lock(&fieldLock);

        if (numBaseballReady == 18) {
            fieldStatus = 1;
        }

        pthread_mutex_unlock(&fieldLock);
        }
    baseballWatcher();
}

void footballWatcher() {
    if (fieldStatus == 0) {
        if (justPlayed == FOOTBALL) {
            footballWatcher();
        }
    } else if (fieldStatus == 1 || 3) {
        pthread_mutex_lock(&fieldLock);

        if (numFootballReady == 18) {
            fieldStatus = 2;
        }

        pthread_mutex_unlock(&fieldLock);
    }
    footballWatcher();
}

void soccerWatcher() {
    
    if (fieldStatus == 0) {
        if (justPlayed == SOCCER) {
            footballWatcher();
        }
    } else if (fieldStatus == 1 || 2) {
        pthread_mutex_lock(&fieldLock);

        if (numSoccerReady == 18) {
            fieldStatus = 3;
        }

        pthread_mutex_unlock(&fieldLock);
    }
    soccerWatcher();
}
*/

void main () {
    //srand(time(NULL));
    srand(12345);  
    
    Player* allPlayers = malloc(TOTALPLAYERS * sizeof(Player)); 
    //Player* baseballPlayers = malloc(maxNumBaseball * sizeof(Player)); 
    //Player* footballPlayers = malloc(maxNumFootball * sizeof(Player));
    //Player* soccerPlayers = malloc(maxNumSoccer * sizeof(Player));

    printf("Initializing field...\n"); 
    printf("Field is currently empty. Waiting for players to come play...\n");

    //create threads here
    //"test case 1" --> 18 baseball players

    for (int i = 0; i < maxNumBaseball; i++) {
        allPlayers[i].playerType = BASEBALL; 
        pthread_create(&allPlayers[i].threadID, NULL, &playerFunction, BASEBALL);
        printf("I am a %d player type. ", allPlayers[i].playerType);
        printf("Created baseball player with threadID %u.\n", allPlayers[i].threadID);
    }

    printf("Created %d baseball players.\n", maxNumBaseball);

    for (int i = 0; i < maxNumFootball; i++) {
        allPlayers[i].playerType = FOOTBALL; 
        pthread_create(&allPlayers[i].threadID, NULL, &playerFunction, FOOTBALL);
        printf("I am a %d player type. ", allPlayers[i].playerType);
        printf("Created football player with threadID %u.\n", allPlayers[i].threadID);
    }

    printf("Created %d football players.\n", maxNumFootball);

    for (int i = 0; i < maxNumSoccer; i++) {
        allPlayers[i].playerType = SOCCER; 
        pthread_create(&allPlayers[i].threadID, NULL, &playerFunction, SOCCER);
        printf("I am a %d player type. ", allPlayers[i].playerType);
        printf("Created soccer player with threadID %u.\n", allPlayers[i].threadID);
    }

    printf("Created %d soccer players.\n", maxNumSoccer);

    printf("Program to sleep for 100 second in Linux.\n");
  
    sleep(100);
    // after 10 seconds this next line will be executed.
    
    printf("This line will be executed after 10 second.");

    //field
    /*
    while (1 == 1) {
        for (int i = 0; i < TOTALPLAYERS; i++) {
            //check if thread is baseball player and is ready
            //how to check if a thread is sleeping?
        }
    }
    */
}

