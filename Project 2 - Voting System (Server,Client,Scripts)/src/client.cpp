#include <fcntl.h>
#include <assert.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sstream>
#include <netdb.h>
#include <iostream>

#define BUFFER_SIZE 1024

// Structure to hold voter and party names
typedef struct {
    char partyName[256];
    char voterFirstName[256];
    char voterLastName[256];
    char server[256];
    int port;
} VoterData;

int countLines(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file" << filename << std::endl;
        return -1;
    }

    int lines = 0;
    std::string line;
    while (std::getline(file, line)) { lines++; }

    file.close();
    return lines;
}
// Function executed by each client thread
void* sendVote(void* arg) {
    VoterData* voterData = (VoterData*)arg;

    // Create a socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Failed to create socket");
        return NULL;
    }

    // Set up the server address
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(voterData->port);
    serverAddress.sin_addr.s_addr = inet_addr(voterData->server);
    std::string voter;
    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Failed to connect to the server");
        close(clientSocket);
        return NULL;
    }

    char message[BUFFER_SIZE];
    memset(message, 0, BUFFER_SIZE);

    int valread = 0;
    
    const char* already_voted = "ALREADY VOTED\n";

    for(int i = 0; i < 2; i++) {

        while ((valread = read(clientSocket, message, BUFFER_SIZE)) > 0) {
            if (message[valread - 1] == '\n' || valread == BUFFER_SIZE) {
                break;  // Stop reading if a newline is encountered
            }
        }


        // Here we should check if we are eligible to send a vote
        // this is not necessary, we could still send the vote
        // without checking if we have to, but we will be sending
        // data to a closed file descriptor
        if (strcmp(already_voted, message) == 0) {
            memset(message, 0, BUFFER_SIZE);            
            break;
        }

        if (i == 0) {
            voter = voterData->voterFirstName;
            voter += ' ';
            voter += voterData->voterLastName;
            voter += "\n";
            send(clientSocket, voter.c_str(), voter.size(), 0);
        }
        else if (i == 1) {
            send(clientSocket, voterData->partyName, strlen(voterData->partyName), 0);
        }

        memset(message, 0, BUFFER_SIZE);
    }

    // shutdown(clientSocket, SHUT_RD);
    shutdown(clientSocket, SHUT_RDWR);
    
    // Close the socket
    close(clientSocket);

    free(voterData);

    return NULL;
}


int main(int argc, char** argv) {
    assert(argc == 4);
    char* server = argv[1];
    int port = atoi(argv[2]);
    char* inputFile = argv[3];

    struct hostent* machine;
    struct in_addr** addr_list;

    machine = gethostbyname(server);
    addr_list = (struct in_addr **)machine->h_addr_list;
    
    for(int i = 0; addr_list[i] != NULL; i++) {
        strcpy(server, inet_ntoa(*addr_list[i]));
        printf("%s\n", server);
    }

    int thread_number = countLines(inputFile);

    // Open the file
    FILE* file = fopen(inputFile, "r");  // Replace "voters.txt" with the actual file name
    if (!file) {
        perror("Failed to open the file");
        return 1;
    }


    char line[BUFFER_SIZE];
    pthread_t* thread_pool = new pthread_t[thread_number];

    size_t threadCount = 0;
    int i = 0;

    // Read each line from the file
    while (fgets(line, sizeof(line), file) != NULL) {
        char* voterNameFirst = strtok(line, " ");
        char* voterNameLast = strtok(NULL, " ");
        char* partyName = strtok(NULL, " ");

        printf("Read from file: %s %s,%s", voterNameFirst, voterNameLast, partyName);

        if (voterNameFirst != NULL && partyName != NULL) {
            // Create voter data structure
            VoterData* voterData = (VoterData*)malloc(sizeof(VoterData));

            strcpy(voterData->voterFirstName, voterNameFirst);
            strcpy(voterData->voterLastName, voterNameLast);
            strcpy(voterData->partyName, partyName);
            strcpy(voterData->server, server);
            voterData->port = port;

            // Create a thread for sending the vote
            if (pthread_create(&thread_pool[threadCount++], NULL, sendVote, voterData) != 0) {
                fprintf(stderr, "Failed to create thread\n");
                free(voterData);
            }
        }
    }

    fclose(file);

    // Wait for all threads to finish
    for (size_t i = 0; i < threadCount; ++i) {
        pthread_join(thread_pool[i], NULL);
    }

    delete[] thread_pool;
    
    return 0;
    
}