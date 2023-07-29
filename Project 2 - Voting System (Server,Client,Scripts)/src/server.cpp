#include <server.hpp>
#include <map>
#include <fcntl.h>
#include <assert.h>
#include <fstream>
#include <deque>

std::deque<int> buffer_queue;
std::map<std::string, std::string> name_vote_map;
std::map<std::string, int> vote_counter;

pthread_mutex_t votermap = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t map_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER,filled=PTHREAD_COND_INITIALIZER;
volatile bool term = false;


void sigint_handler(int signal) {

    switch (signal)
    {
        case SIGINT:
            term = true;
            pthread_cond_broadcast(&empty);
            pthread_cond_broadcast(&filled);
            pthread_mutex_unlock(&mutex);
            break;
        
        default:
            break;
    }
}

int main(int argc, char** argv) {

    struct sigaction sigint_action;
    sigint_action.sa_flags = 0;
    sigfillset(&sigint_action.sa_mask);
    sigint_action.sa_handler = &sigint_handler;
    sigaction(SIGINT, &sigint_action, NULL);


    assert(argc == 6);
    int port = atoi(argv[1]);
    int worker_threads = atoi(argv[2]);
    int queue_buffer_max_size = atoi(argv[3]);
    char* poll_log = argv[4];
    char* poll_stats = argv[5]; 

    if (worker_threads <= 0) {
        fprintf(stderr, "numWorkerthreads > 0\n");
        exit(EXIT_FAILURE);
    }

    if (queue_buffer_max_size <= 0) {
        fprintf(stderr, "bufferSize > 0\n");
        exit(EXIT_FAILURE);
    }


    int server_fd, new_socket, valread;
    int err;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the specified address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    char buf_name[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &address.sin_addr, buf_name, sizeof(buf_name));
    printf("Server address: %s\n", buf_name);
    
    // Start listening for incoming connections
    if (listen(server_fd, 1000) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    pthread_t* thread_pool = new pthread_t[worker_threads];
    thread_args arg(queue_buffer_max_size, poll_log);

    for(int i = 0; i < worker_threads; i++) {
        int kati = pthread_create(&thread_pool[i], NULL, &consumer_thread, &arg);
    }

    FILE* fp = fopen(poll_log, "w");
    fclose(fp);
    
    while (!term) {
        // Accept a new client connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            if (new_socket == -1 && errno == EINTR) {
                break;
            }
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Protect the shared queue data structure
        pthread_mutex_lock(&mutex);
            // While full, wait until something is freed
            // a.k.a. empty is signaled
            // to continue
            while (buffer_queue.size() >= queue_buffer_max_size) {
                pthread_cond_wait(&empty, &mutex);
            }
            buffer_queue.push_back(new_socket);
            pthread_cond_signal(&filled);

        pthread_mutex_unlock(&mutex);
        
    }


    for(int i = 0; i < worker_threads; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    create_poll_stats(poll_stats, poll_log);
    delete[] thread_pool;
    close(server_fd);

    return 0;
}
