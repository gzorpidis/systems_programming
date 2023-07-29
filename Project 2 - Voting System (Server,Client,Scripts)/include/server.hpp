#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFFER_SIZE 1024


class thread_args {
    private:
    int max;
    char* poll_log_file_name;
    public:

    thread_args(int max_i, char* poll_i) : max(max_i), poll_log_file_name(poll_i){};
    bool set_max(int max_i) { max = max_i; return true; }
    bool set_poll(char* poll_i) { poll_log_file_name = poll_i; return true; }
    const char* get_poll() { return poll_log_file_name; }
};

int create_poll_stats(char* poll_stats_file_name, char* poll_log);
void* consumer_thread(void* args);