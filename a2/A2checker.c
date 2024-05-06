#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#define MAX_LINE_LENGTH 10000

pthread_mutex_t g_Mutex;
int wordcount = 0;
int runningThreads = 0;
char  allThread[1000][100];
int  allWrong[1000] = {0};


//pass in filenames to thread function
typedef struct{
    char filename[100];
    char dictionary[100];
} ThreadArgs;

//store misspelt words
typedef struct{
    char badWord[100];
    int count[100];
} WordMisspelt;

//find most incorrect amongst files
void totalIncorrect(char arr[1000][100], int * high){
    pthread_mutex_lock(&g_Mutex); //access global variable
    int i=0, j=0;
    if((i<3) && (high[i] != -1)){
        while(allWrong[j] != 0){
            if((strcmp(arr[high[i]], allThread[j]))==0){
                allWrong[j]++;
                break;
            }
            j++;
        }
        strcpy(allThread[j], arr[high[i]]);
        allWrong[j]=1;
        i++;
    }
    if((i<3) && (high[i] != -1)){
        while(allWrong[j] != 0){
            if((strcmp(arr[high[i]], allThread[j]))==0){
                allWrong[j]++;
                break;
            }
            j++;
        }
        strcpy(allThread[j], arr[high[i]]);
        allWrong[j]=1;
        i++;
    }
    if((i<3) && (high[i] != -1)){
        while(allWrong[j] != 0){
            if((strcmp(arr[high[i]], allThread[j]))==0){
                allWrong[j]++;
                break;
            }
            j++;
        }
        strcpy(allThread[j], arr[high[i]]);
        allWrong[j]=1;
        i++;
    }
    pthread_mutex_unlock(&g_Mutex);
}

//find the 3 most misspelt words
void findThreeHighestIndexes(int arr[], int size, int *highestIndexes) {
    // Initialize variables to store the indices of the three highest elements
    int firstHighestIndex = -1;
    int secondHighestIndex = -1;
    int thirdHighestIndex = -1;
    for (int i = 0; i < size; i++) {
        if (firstHighestIndex == -1 || arr[i] > arr[firstHighestIndex]) {
            thirdHighestIndex = secondHighestIndex;
            secondHighestIndex = firstHighestIndex;
            firstHighestIndex = i;
        }else if (secondHighestIndex == -1 || arr[i] > arr[secondHighestIndex]) {
            thirdHighestIndex = secondHighestIndex;
            secondHighestIndex = i;
        }else if (thirdHighestIndex == -1 || arr[i] > arr[thirdHighestIndex]) {
            thirdHighestIndex = i;
        }
    }
    highestIndexes[0] = firstHighestIndex;
    highestIndexes[1] = secondHighestIndex;
    highestIndexes[2] = thirdHighestIndex;
  //  printf("\n index are %d %d %d\n", firstHighestIndex, secondHighestIndex, thirdHighestIndex);
}

//add word to array of misspelt
int addWord(char array[1000][100], int numWrong[1000], char*  word, int max){
    int i=0;
    while(i < max){
        if( strcmp(word, array[i]) == 0){
            (numWrong)[i] = (numWrong)[i] + 1;
            return 0;
        }
        i++;
    }
    strcpy(array[i], word);
    (numWrong)[i] = 1;
    return 1;
}

//write output summary to file
void writeFile(char * file, int errors, int * num, char array[][100]){
    pthread_mutex_lock(&g_Mutex);  //writing to file
    FILE * output = fopen("ppatel46_A2.out","a");
    if (output != NULL){
        fprintf(output, "%s %d ", file, errors);
        int i=0;
        while ((num[i] != -1) && (i<3)){
            fprintf(output, "%s ", array[num[i]]);
            i++;
        }
        fprintf(output, "\n");
        fclose(output);
    }
    pthread_mutex_unlock(&g_Mutex);
}

//check if word is in the dictionary
int wordInDictionary(char * word, char **dictionArray, int numWords){
    for (int i=0; i<numWords; i++){
        if(strcmp(word, dictionArray[i]) == 0){
            return 1;
        }
    }
    return 0;
}

//change word to lowercase
void toLower(char *str) {
    while (*str) {
        *str = tolower(*str);
        str++;
    }
}

//remove delimiters from word
void removeDelimiter(char * word){
    char * src = word;
    char * final = word;
    while (*src){
        if (*src == ',' || *src == '.' || *src == ';' || *src == ':' || *src == '?' ||
            *src == '!' || *src == '(' || *src == ')' || *src == '[' || *src == ']' ||
            *src == '{' || *src == '}' || *src == '\"') {
                src++;
        }else{
            *final++ = *src++;
        }
    }
    *final = '\0';
}

//read and process the dictionary
void readDictionary(char *** dictArray, char * dictionary, int * numWords){
    FILE * fp = fopen(dictionary, "r");
    int cap = 100, len=0;
    
    (*dictArray) = malloc(cap * sizeof(char*));
    char word[100];
    while (fscanf(fp, "%s", word) != EOF){
        (*dictArray)[*numWords] = malloc((strlen(word)+1)*sizeof(char));
        strcpy((*dictArray)[*numWords], word);
        (*numWords)++;
        if(*numWords >= cap){
            cap *=2;
            (*dictArray) = realloc((*dictArray), cap * sizeof(char*));
        }
    }
    for(int k=0; k< (*numWords); k++){
        toLower((*dictArray)[k]);
        len = strlen((*dictArray)[k]);
        if((*dictArray)[k][len-1] == '\n'){
            (*dictArray)[k][len-1] = '\0';
        }
        removeDelimiter((*dictArray)[k]);
    }
    fclose(fp);
}

//thread function to spellcheck
void *spellCheck(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    char *dictionary = args->dictionary;
    char *fileName = args->filename;

    char localError[1000][100];
    int localErr[1000] = {0}; // Initialize all elements to 0
    int max = 5, counter = 0, localErrors = 0;
    int numWords = 0;
    char **dictArray = NULL;

    readDictionary(&dictArray, dictionary, &numWords);

    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file: %s\n", fileName);
        return NULL;
    }

    char line[MAX_LINE_LENGTH]; // Assuming MAX_LINE_LENGTH is defined somewhere
    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
        char *token = strtok(line, " \t\n"); // Split the line into tokens separated by space, tab, or newline
        while (token != NULL) {
        //    printf("\nWhen added, %s is\n", token);
            toLower(token);
            removeDelimiter(token);
            if (!wordInDictionary(token, dictArray, numWords)) {
                if (counter >= max) {
                    max += 5;
                }
                if (addWord(localError, localErr, token, counter)) {
                    counter++;
                }
                localErrors++; //access global variable
                pthread_mutex_lock(&g_Mutex);
                wordcount++;
                pthread_mutex_unlock(&g_Mutex);
            }
            token = strtok(NULL, " \t\n"); // Move to the next token
        }
    }
    fclose(fp);
    int highest[3];
    findThreeHighestIndexes(localErr, counter, highest);
    totalIncorrect(localError, highest);
    writeFile(fileName, localErrors, highest, localError);
   // totalError(highest, localError);
    for (int p=0; p<numWords; p++){
        free(dictArray[p]);
    }
    free(dictArray);   //access global
    pthread_mutex_lock(&g_Mutex);
    printf("\nDone Executing\n");
    runningThreads--;
    pthread_mutex_unlock(&g_Mutex);
    return NULL;
}

//check if user file input is valid
int checkValid(char * dictionary, char * file){
    FILE * fp;
    fp = fopen(dictionary, "r");  //open file
    if(fp == NULL){
        printf("File name %s not found\n", dictionary);
        return -1;   //return -1, main will exit(1)
    }
    fclose(fp);
    fp = fopen(file, "r");
    if(fp == NULL){
        printf("File name %s not found\n", file);
        return -1;
    }
    fclose(fp);
    return 1;
}

int findTotal(){
    int i=0;
    while(allWrong[i] != 0){
        i++;
    }
    return i;
}

int main(int argc, char ** argv){
    pthread_t * tid;
    pthread_attr_t attr; 
    pthread_attr_init(&attr); //set detached attribute
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_mutex_init(&g_Mutex, NULL);
    ThreadArgs args[100];
    int print = 1;
    
    if((argc == 2) && (strcmp(argv[1],"-l")==0)){
        print = 0;
    }
    tid = (pthread_t*)malloc(sizeof(pthread_t) * 100);
    for(int k=0; k<100; k++){
        tid[k] = 0;
    }
    int userChoice = 1;
    int subMenu = 0, number=0;
    int i=0;

    while (userChoice != 2){
        printf("\nMain Menu:\n");
        printf("1. Start a new spellchecking task\n");
        printf("2. Exit\n");
        printf("Enter (1/2): ");

        scanf("%d", &userChoice);
        if (userChoice == 1){
            printf("Please enter file for dictionary: ");
            scanf("%s", args[i].dictionary);
            printf("Please enter file for text file to spellcheck: ");
            scanf("%s", args[i].filename);

            printf("\nSub Menu: \n");
            printf("1: Start spell checking now\n");
            printf("2: Return back to main menu\n");
            printf("Enter (1/2): ");
            scanf("%d", &subMenu);

            if(subMenu == 1){
                number = checkValid(args[i].dictionary, args[i].filename);
                if (number == 1){
                    pthread_mutex_lock(&g_Mutex);
                    pthread_create(&tid[i], &attr, spellCheck, &args[i]);
                    pthread_mutex_unlock(&g_Mutex);
                    i++;
                }
            }
            else if(subMenu != 2){
                printf("Invalid Option\n");
            }
        }
        else if(userChoice != 2){
            printf("Invalid Option (1/2) Please\n");
        }
    }
    int running = i + runningThreads;
    int done = i-running;

    printf("\nExiting %d threads running\n", running);
    printf("\nExiting %d threads done\n", done);
    pthread_attr_destroy(&attr);
    int highest[3];
    int totalWrong = findTotal();
    findThreeHighestIndexes(allWrong, totalWrong, highest);
    
    if(print==0){ 
        FILE * output = fopen("ppatel46_A2.out","a");
        if (output != NULL){
            fprintf(output, "Number of files processed: %d\n", i);
            fprintf(output, "Number of spelling errors: %d\n", wordcount);
            fprintf(output, "Three most common misspellings: ");
            int k=0;
            while((highest[k] != -1) && (k<3)){
                fprintf(output, "%s (%d times) ", allThread[highest[k]], allWrong[highest[k]]);
                k++;
            }
            fclose(output);
        }
    }
    else{
        printf("Number of files processed: %d\n", i);
        printf("Number of spelling errors: %d\n", wordcount);
        printf("Three most common misspellings: ");
        int k=0;
        while((k<3) && (highest[k] != -1)){
            printf("%s (%d times) ", allThread[highest[k]], allWrong[highest[k]]);
            k++;
        }
        printf("\n");
    }
    free(tid);
    printf("Program will terminate now counter is %d\n", i);
    return 0;
}
