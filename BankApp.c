#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h> 

pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;
#define MAX_BUFFER_SIZE 5

//ideas
//in withdrawl in depsoit i can show current balnce while doing withdrawls and depsoits
// make to where i stay in screen afte doing 1 withdral/depsoti make it a  menu

//make it where failed operations are even shown for accounts and sotred in csv file 

// go back to branch selection after process in branch are done

// in deposit / withdrawl could check if id exist


struct AccountDetail
{
    int accountID;
    char branch[40];
    char name[40];
    char operation[40];
    int amount;
};

// Function prototypes

bool IdTaken(const char *fileName, int targetID); // check if ID is already taken when creating new account 
void FindBranch(char branch);
void* displayMenu(void* arg);
struct AccountDetail createAccount(int accountID, char branch, const char *name, const char *fileName);
void History(const char *fileName, int targetValue);//used to print all account details for specfied ID 
int FinalBalance(const char *fileName, int targetID);
void ShowFinalBal(const char *fileName, int targetID);
void Withdrawal(const char *fileName, int targetID, char branch, const char *name, int amount);
void Deposit(const char *fileName, int targetID, char branch, const char *name, int amount);
void Last5Transactions(const char *fileName, int targetID);







int main()
{
    //explnation for threads
    //the thread is created after user selects a branch
    // which inside the findBranch function when the wokring thread is creating it will display the menu
    //which after  the menu is exited after the user does all of his wanted operations they will be be exited out of the menu function the new thread will then join with the parent proces
   
    char choice;
    printf("Please Enter Branch (N E S W): ");
    scanf(" %c", &choice);
    FindBranch(choice); // thread creation
    
    return 0;
}

struct AccountDetail createAccount(int accountID, char branch, const char *name, const char *fileName)
{
    pthread_mutex_lock(&fileMutex); // Lock the mutex before file operations to sync access among threads

    struct AccountDetail newAccount; // create struct for account

    if (accountID < 1000 || accountID > 9999) // need to update so  people are allowed to 0001...
    {
        printf("Please enter a 4-digit account ID: ");
        scanf("%d", &accountID);
    }

    
    if (IdTaken(fileName, accountID)) // Check if the account ID is already taken
    {
        printf("Account ID %d is already taken. Please choose a different ID.\n", accountID);
        pthread_mutex_unlock(&fileMutex);  //unlocks the mutex that was locked at the begining to sync resouces enusring that mutiple threads wont interfere with each other
        newAccount.accountID = -1; // Return an invalid account ID
        return newAccount;
    }

    // assign values to the struct 
    newAccount.accountID = accountID; 
    strncpy(newAccount.branch, &branch, sizeof(newAccount.branch) - 1);
    strncpy(newAccount.name, name, sizeof(newAccount.name) - 1);
    strncpy(newAccount.operation, "New Account", sizeof(newAccount.operation) - 1);
    newAccount.amount = 0;


    
    FILE *file = fopen(fileName, "a");  //open the csv file so we can store a new account 

    if (file == NULL)
    {
        printf("Error opening file %s.\n", fileName);
        pthread_mutex_unlock(&fileMutex); // Unlock the mutex on error
        return newAccount;
    }

  
    fprintf(file, "%d,%c,%s,create,%d\n", newAccount.accountID, branch, newAccount.name, newAccount.amount);   // Record account creation transaction in CSV format

    fclose(file);

    pthread_mutex_unlock(&fileMutex); // Unlock the mutex after file operations

    return newAccount; // return new account  NOT REALLY NECCSARY.
}

// Implementation of the new function
bool IdTaken(const char *fileName, int targetID)
{
    FILE *file = fopen(fileName, "r"); // only neccsary to open file for reading 

    if (file == NULL)
    {
        printf("Error opening file %s.\n", fileName);
        return true; // Assume taken in case of an error
    }

    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), file) != NULL) // read all ID into buffer
    {
        int value;
        if (sscanf(buffer, "%d,", &value) == 1 && value == targetID) // see if account id is taken
        {
            fclose(file);
            return true; // Account ID is taken
        }
    }

    fclose(file);

    return false; // Account ID is not taken
}


void FindBranch(char branch)
{
    pthread_t thread;

    switch (branch)
    {
    case 'N':
        printf("You are now entering the North branch.\n");
        pthread_create(&thread, NULL, displayMenu, (void *)&branch); // create a thread  when menu is closed the thread is joined
        pthread_join(thread, NULL);
        printf("Work in North branch is finished.\n");
        break;
    case 'E':
        printf("You are now entering the East branch.\n");
        pthread_create(&thread, NULL, displayMenu, (void *)&branch);
        pthread_join(thread, NULL);
        printf("Work in East branch is finished.\n");
        break;
    case 'S':
        printf("You are now entering the South branch.\n");
        pthread_create(&thread, NULL, displayMenu, (void *)&branch);
        pthread_join(thread, NULL);
        printf("Work in South branch is finished.\n");
        break;
    case 'W':
        printf("You are now entering the West branch.\n");
        pthread_create(&thread, NULL, displayMenu, (void *)&branch);
        pthread_join(thread, NULL);
        printf("Work in West branch is finished.\n");
        break;
    default:
        printf("No branch exists.\n");
        break;
    }
}

void History(const char *fileName, int targetValue) //used to print all account details for specfied ID 
{
    FILE *file = fopen(fileName, "r");

    if (file == NULL)
    {
        printf("Error opening file %s.\n", fileName);
        return;
    }

    char buffer[1024]; // put everything in an array

    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        //parse each line
        int value;
        char branch[40], name[40], operation[40];
        int amount;

//%d for decimal values %39[^,] everything besideds a comma 
        if (sscanf(buffer, "%d,%39[^,],%39[^,],%39[^,],%d",
                   &value, branch, name, operation, &amount) == 5)
        {
            if (value == targetValue) // if value of id is equal to taret id print the whole line 
            {
                printf("Print transactions history: %s", buffer);
            }
        }
        else
        {
            printf("Error parsing line: %s", buffer);
        }
    }

    fclose(file);
}


void ShowFinalBal(const char *fileName, int targetID) // also does calculation for balance
{
    FILE *file = fopen(fileName, "r");

    if (file == NULL)
    {
        printf("Error opening file %s.\n", fileName);
        return;
    }

    char buffer[1024];
    int balance = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        int id;
        char branch[40], name[40], operation[40];
        int amount;

        if (sscanf(buffer, "%d,%39[^,],%39[^,],%39[^,],%d",
                   &id, branch, name, operation, &amount) == 5)
        {
            //does math for balance 
            //note withdrawls are stored as negative values inside csv for easier implmentation
            if (id == targetID)
            {
                balance += amount;
            }
        }
        else
        {
            printf("Error parsing line: %s", buffer);
        }
    }

    fclose(file);

    printf("Final balance for ID %d: %d\n", targetID, balance);
}

void Last5Transactions(const char *fileName, int targetID)
{
FILE *file = fopen(fileName, "r");

    if (file == NULL) {
        printf("Error opening file %s.\n", fileName);
        return;
    }
// stores line
    char buffer[1024];

    // makes 5 arrays to store the last 5 lines
    char lastFiveLines[5][1024];
    // use to make sure there is only 5 lines
    int lastLineIndex = 0;
    int balance = 0;//OUTDATED HAVE A BETTER WAY OF DOING THIS 

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        int id;
        char branch[40], name[40], operation[40];
        int amount;

        if (sscanf(buffer, "%d,%39[^,],%39[^,],%39[^,],%d",
                   &id, branch, name, operation, &amount) == 5) {
            if (id == targetID) {
                balance += amount;


                strcpy(lastFiveLines[lastLineIndex], buffer);
                lastLineIndex = (lastLineIndex + 1) % 5;
            }
        } else {
            printf("Error parsing line: %s", buffer);
        }
    }

    fclose(file);

    printf("Final balance for ID %d: %d\n", targetID, balance);

//now print only the last 5 lines 
    for (int i = 0; i < 5; i++) {
        if (strlen(lastFiveLines[i]) > 0) {
            printf("%s", lastFiveLines[i]);
        }
    }
}
  
  int FinalBalance(const char *fileName, int targetID)
{
    FILE *file = fopen(fileName, "r");

    if (file == NULL)
    {
        printf("Error opening file %s.\n", fileName);
        return -1; // Return a sentinel value to indicate an error
    }

    char buffer[1024];
    int balance = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        int id;
        char branch[40], name[40], operation[40];
        int amount;

        if (sscanf(buffer, "%d,%39[^,],%39[^,],%39[^,],%d",
                   &id, branch, name, operation, &amount) == 5)
        {
            // Does math for balance
            // Note: withdrawals are stored as negative values inside the CSV for easier implementation
            if (id == targetID)
            {
                balance += amount;
            }
        }
        else
        {
            printf("Error parsing line: %s", buffer);
        }
    }

    fclose(file);

    return balance;
}


void Withdrawal(const char *fileName, int targetID, char branch, const char *name, int amount)
{
   pthread_mutex_lock(&fileMutex);

    FILE *file = fopen(fileName, "a");

    amount = amount * -1;

    if (file == NULL)
    {
        printf("Error opening file %s.\n", fileName);
        pthread_mutex_unlock(&fileMutex);
        return;
    }

    // Get the current balance
    int currentBalance = FinalBalance(fileName, targetID);

    //USED TO MAKE IT POSTIVE AGAING SO IT CAN BE COMPARED WITH CURRENT BALNACE
    amount = amount * -1;
    // Check if the withdrawal is valid based on the current balance
    if (amount > currentBalance)
    {
        printf("Not enough funds for withdrawal. Current balance: %d\n", currentBalance);
        fclose(file);
        pthread_mutex_unlock(&fileMutex);
        return;
    }
    amount = amount * -1;

    // Record the withdrawal transaction in the file
    fprintf(file, "%d,%c,%s,withdraw,%d\n", targetID, branch, name, amount);

    fclose(file);

    printf("Withdrawal of %d from account ID %d completed. New balance: %d\n", amount, targetID, currentBalance + amount);

    pthread_mutex_unlock(&fileMutex);
}



void Deposit(const char *fileName, int targetID, char branch, const char *name, int amount)
{
    pthread_mutex_lock(&fileMutex);

    FILE *file = fopen(fileName, "a");

    if (file == NULL)
    {
        printf("Error opening file %s.\n", fileName);
        pthread_mutex_unlock(&fileMutex);
        return;
    }

    fprintf(file, "%d,%c,%s,deposit,%d\n", targetID, branch, name, amount);

    fclose(file);

    printf("Deposit of %d to account ID %d completed.\n", amount, targetID);

    pthread_mutex_unlock(&fileMutex);
}

void* displayMenu(void* arg)
{
    char branch = *((char*)arg);
    int choice;
    char *branchFile = NULL;
    FILE *file;
    int searchID;
    int bal;

    switch (branch)
    {
    case 'N':
        file = fopen("North.csv", "a");
        branchFile = "North.csv";
        break;
    case 'E':
        file = fopen("East.csv", "a");
        branchFile = "East.csv";
        break;
    case 'S':
        file = fopen("South.csv", "a");
        branchFile = "South.csv";
        break;
    case 'W':
        file = fopen("West.csv", "a");
        branchFile = "West.csv";
        break;
    default:
        printf("No branch exists.\n");
        pthread_exit(NULL);
    }

    do
    {
        printf("\n1. Create a new Account with a unique ID\n");
        printf("2. Show balance\n");
        printf("3. Deposit or withdrawal\n");
        printf("4.Print transaction histroy\n");
        printf("5.Print last 5 transactations\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");

        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            printf("You selected Option 1\n\n");

            int id;
            char name[40];

            printf("Enter Account ID (4 digits): ");
            scanf("%d", &id);

            printf("Enter Name: ");
            scanf("%s", name);

            struct AccountDetail myAccount = createAccount(id, branch, name,branchFile);

            printf("Account ID: %d\n", myAccount.accountID);
            printf("Branch: %s\n", myAccount.branch);
            printf("Name: %s\n", myAccount.name);
            printf("Operation: %s\n", myAccount.operation);
            printf("Amount: %d\n", myAccount.amount);

            break;
        case 2:
            printf("You selected Option 2\n");

            int searchID;
            printf("Enter Account ID to search: ");
            
            scanf("%d", &searchID);
            
            //could replace function with this 
           // bal = getFinalBalance(branchFile,searchID);
            ShowFinalBal(branchFile, searchID);

            break;
        case 3:
            int opt;
            int amount;
            int id2;


            printf("You selected Option 3\n");
            // could add some more checking 
            printf("Please enter the 4-digit ID for the transaction: ");
            scanf("%d", &id2);

            printf("    For withdrawal(1);  for deposits(2): ");
            scanf("%d", &opt);
            bal = FinalBalance(branchFile,id2);

            if (opt == 1)
            {
                printf("Please enter the amount you would like to withdraw: ");
                scanf("%d", &amount);
                Withdrawal(branchFile, id2, branch, "_", amount);
            }
            else if (opt == 2)
            {
                printf("Please enter the amount you would like to deposit: ");
                scanf("%d", &amount);
                Deposit(branchFile, id2, branch, "_", amount);
            }

            break;
        case 4:
            printf("You selected Option 4\n");

            printf("Enter Account ID to search: ");
            scanf("%d", &searchID);
            History(branchFile, searchID);
            break;

        case 5:
            printf("Enter Account ID to search: ");
            scanf("%d", &searchID);
            Last5Transactions(branchFile,searchID);

            break;
        case 6:
            printf("Exiting the program\n");
            break;
        default:
            printf("Invalid choice. Please enter a valid option.\n");
        }
    } while (choice != 6);

    pthread_exit(NULL);
}
