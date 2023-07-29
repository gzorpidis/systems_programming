#include <server.hpp>
#include <map>
#include <fcntl.h>
#include <assert.h>
#include <fstream>
#include <deque>

extern std::deque<int> buffer_queue;
extern std::map<std::string, std::string> name_vote_map;
extern std::map<std::string, int> vote_counter;

extern pthread_mutex_t votermap ;
extern pthread_mutex_t mutex ;
extern pthread_mutex_t write_mutex;
extern pthread_mutex_t map_mutex ;
extern pthread_cond_t empty,filled;
extern volatile bool term;


void handle_connection(int new_socket, const char* file_name) {
    int i = 0;
    char buffer[BUFFER_SIZE] = {0};
    const char* name = "SEND NAME PLEASE\n";
    const char* vote = "SEND VOTE PLEASE\n";
    const char* already_voted = "ALREADY VOTED\n";
    int valread;
    
    std::string voter_name;
    std::string party;
    std::string read_input = "";
    size_t read = 0;
    
    do {
        // Clear the buffer
        memset(buffer, 0, sizeof(buffer));

        // Send messages
        if (i == 0) {
            // First message, send "SEND NAME PLEASE"
            send(new_socket, name, strlen(name), 0);
        } else if (i == 1) {
            // Second message, send "SEND VOTE PLEASE"
            send(new_socket, vote, strlen(vote), 0);
        }
        
        // Read client's response
        read_input = "";
        read = 0;
        while ((valread = recv(new_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
            buffer[valread] = '\0';
            read_input += buffer;
            read += valread;

            if (read == BUFFER_SIZE - 1) {
                break;
            }

            if (read_input.back() == '\n') {
                read_input.pop_back();
                break;  // Stop reading if a newline is encountered
            }
            
        }
        

        
        if (i == 0) {
            // Insert the voter's name in the map, (voter_name -> ""), since he hasn't voted yet
            voter_name = read_input;

            // Lock the mutex for the map, as I am about to enter the structure
            pthread_mutex_lock(&map_mutex);
            auto search = name_vote_map.find(voter_name);
            // Not found, write inside
            if (search == name_vote_map.end()) {
                // voter_name = buffer;
                name_vote_map[voter_name] = "";
                pthread_mutex_unlock(&map_mutex);
            } else {
                pthread_mutex_unlock(&map_mutex);
                send(new_socket, already_voted, strlen(already_voted), 0);
                break;
            }

        } else if (i == 1) {
            party = read_input;

            pthread_mutex_lock(&votermap);
            vote_counter[party]++;
            pthread_mutex_unlock(&votermap);
            
            pthread_mutex_lock(&write_mutex);
                FILE* fd = fopen(file_name, "a+");

                if (fd == NULL) {
                    perror("opening file");
                    exit(EXIT_FAILURE);
                }
                
                fprintf(fd, "%s %s\n", voter_name.c_str(), party.c_str());
                fclose(fd);
            pthread_mutex_unlock(&write_mutex);

            std::string recorded = "VOTE for PARTY " + party + " RECORDED\n" ;
            send(new_socket, recorded.c_str(), recorded.size(), 0);
            break;
        }
        i++;
        // // Send response to the client
    } while ( true );
    
    shutdown(new_socket, SHUT_WR);
    close(new_socket);
}


// Consumer thread, every worker thread
// reads a connection from the queue(buffer)
// and proceeds to work on it
void* consumer_thread(void* args) {
    thread_args* parameters = (thread_args*)args;

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    pthread_sigmask(SIG_BLOCK, &mask, NULL);
    // if queue is empty:
    //     do not process
    
    // process data...

    // remove connection(job)
    // and signal main thread that a connection was handled
    // so a connection has been freed
    
    while(1) {
        pthread_mutex_lock(&mutex);
            while(buffer_queue.size() == 0) {
                pthread_cond_wait(&filled, &mutex);
                if (term) {
                    pthread_mutex_unlock(&mutex);
                    pthread_exit(NULL);
                }
            }

            
            // Retrieve fd, (consume)
            int client_fd = buffer_queue.front(); 
            buffer_queue.pop_front();

            // signal that a position has emptied
            pthread_cond_signal(&empty);

        pthread_mutex_unlock(&mutex);

        handle_connection(client_fd, parameters->get_poll());
    }
}

int create_poll_stats(char* poll_stats_file_name, char* poll_log) {

    FILE* fp = fopen(poll_stats_file_name, "w");
    
    if (fp == NULL) {
        fprintf(stderr, "Problem opening poll log file.\n");
        exit(-1);
    }
    int total = 0;
    for (const auto& pair : vote_counter) {
        // fprintf(stdout, "%s %d\n", pair.first.c_str(), pair.second);
        fprintf(fp, "%s %d\n", pair.first.c_str(), pair.second);
        total += pair.second;
    }

    fprintf(fp, "TOTAL %d\n", total);
    fclose(fp);
    return 0;
}
