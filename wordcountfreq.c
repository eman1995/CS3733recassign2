#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>

#define MAXSIZE 4096
//Struct for storig each word, its name and the number of times it appears in the file
typedef struct sword_st
{
    char wordname[128];
    int wordcount;
    struct sword *next;
} sword;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
//Head to point to begining of linked list
struct sword_st *Head=NULL;
//once the end of file is reached
void printwords()
{
    struct sword_st *temp = Head;
    //go through list starting from head
    while(temp != NULL)
    {
        printf("%s appears %d times\n", temp->wordname, temp->wordcount);
        temp = Head->next;
        free(Head);//make sure nothing is left when the list is ran through
        Head = temp;
    }
}
//only used once to initialize the list and make Head point to the start of the list
//passed in a char pointer to a string
void newlist(char *word)
{
    struct sword_st *sword = malloc(sizeof(struct sword_st));
    //sword->wordname = word;
    strcpy(sword->wordname, word);
    //printf("%s\n", sword->wordname);
    sword->wordcount = 1;//count is initialzed
    sword->next = NULL;
    Head = sword;
    return;
    //printf("%s\n", Head->wordname);
}
//used when a new word struct must be made and put into the list
//passed in a char * for the word to be stored
void newword(char *word)
{
    struct sword_st *temp = Head;
    struct sword_st *newword = malloc(sizeof(struct sword_st));
    while(temp->next != NULL)
    {
        temp = temp->next;
    }
    strcpy(newword->wordname, word);
    newword->wordcount = 1;
    temp->next = newword;
    newword->next = NULL;
    return;
}
//used todecide what to do what a word found in the file
//it it passed in a char * to the word in question
void wordlinkedlist(char *word)
{
    //printf("%s\n", word);
    if(Head == NULL)
    {
        newlist(word);
        return;
    }
    struct sword_st *temp = Head;
    while(temp != NULL)
    {
        if(strcmp(temp->wordname, word) == 0)
        {
            //printf("Same word in linked list\n");
            temp->wordcount++;
            return;
        }
        else
        {
            temp = temp->next;
        }
    }
    newword(word);
}    
//function to find and count each word in a file, passed in the file name to open it
//ensures multiple process can call thier own functions
void countword(char *fname)
{   
    FILE *f;
    char word[MAXSIZE];
    char buffer[MAXSIZE];
    int i=0, wc=0, n=0;

    //printf("function test %s\n", fname);
    f = fopen(fname, "r");

    if (f == NULL)
    {
        printf("Failed to open file: %s\n", fname);
        return;
    }

    while (fgets(buffer, MAXSIZE, f))
    {
        while(buffer[i] != '\0') 
        {
            if (buffer[i] == ' ' || buffer[i] == '\t' || buffer[i] == '\n')
            {   
                word[n] = '\0';
                pthread_mutex_lock(&lock);
                wordlinkedlist(word);
                pthread_mutex_unlock(&lock);
                wc++;
                n=0;
                word[n]='\0';
            }
            else
            {
                word[n++] = buffer[i];
            }
            i++;
        }
        i=0;
    }
    printf("Child precess for %s: number of words is %d\n", fname, wc);
    //printwords();
    close(f);
}
//thread function
void *threadfunc(void * arg) {
    char *fname;
    fname = (char *) malloc(sizeof(char));
    strcpy(fname, (char *) arg);
    countword(fname);
}

//main function calls the fork() function and passes the file name to each call
int main(int argc, char *argv[])
{
    int i = 0;
    pthread_mutex_init(&lock, NULL);
    pthread_t threadcount[argc];
    for (i=1; i<argc; i++) {
		pthread_create(&threadcount[i], NULL, threadfunc,(void *)argv[i]);
    }
    for (i=1; i<argc; i++) {
		pthread_join(threadcount[i], NULL);
    }
    printwords();
    //countword(argv[i]);
    printf("All %d files have been counted!\n", argc-1);
    return 0;
}

