
#pragma once 
#include <iostream>
#include <unistd.h>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <sys/wait.h>
#include "../include/parsing.hpp"
#include <bits/stdc++.h>
#include <pwd.h>
#include <glob.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <map>
#include <stdlib.h>
#include <unistd.h>

#define READ_END 0
#define WRITE_END 1
#define JOBS_RUNNING 20

#define HISTORY 20
#define BUFFER 1024

#define COMMAND_EXTERNAL 0
#define COMMAND_EXIT 1
#define COMMAND_CD 2
#define COMMAND_CREATEALIAS 3
#define COMMAND_DESTROYALIAS 4
#define COMMAND_ALIAS 5
#define COMMAND_HISTORY 6

#define CUR_USER_BUFFER 1024
#define PW_DIR_BUFFER 1024

#define BG_EXEC 0
#define FG_EXEC 1
#define PL_EXEC 2

#define STATUS_RUNNING 0

class job* mysh_parse_command(std::vector<std::string> tcl, std::string command_given);
int mysh_job_exec(class job* job);
int mysh_proc_exec(struct job *job, struct process *proc, int in_fd, int out_fd, int mode, int* pipefds, int command_processed, int pipes);
void parse_and_execute( std::string& userInput );

class process {
    public:
        class process *next;
        char **argv;
        int argc;
        char *infd;
        char *outfd;
        bool append;
        char *command;
        pid_t pid;
        int type;
        int mode;

        int build_command(
            char* infd,
            char* outfd,
            int pid,
            const int argc,
            char** argv,
            const int mode
        ) {
            this->infd = infd;
            this->outfd = outfd;
            this->pid = pid;
            this->argc = argc;
            this->argv = argv;
            this->mode = mode;
            this->set_next(NULL);

            return 1;
        }

        ~process() {
            free(command);

            free(infd);
            free(outfd);
            for(int i = 0; i < argc; i++)
                free(argv[i]);
            
            free(argv);
        }


        // Set pid, type, status, next process
        void set_data(pid_t pid, int type, int status, class process* next)  {
            this->pid = pid;
            this->type = type;
            set_next(next);
        }

        int get_process_type() { return this->type; }
        void set_next(class process* proc) { this->next = proc; }
        class process* get_next_process() { return this->next; }
        void set_pid(pid_t pid) { this->pid = pid; }
        int get_pid() { return this->pid; }

};

class job {
    public:
        class process *root;
        std::string command;
        pid_t pgid;
        int mode;
        job(int pgid, int mode, std::string command, class process* process);
        void set_root(struct process* new_root);
        int get_pgid() { return pgid; }
        void set_pgid(int pgid) { this->pgid = pgid; }
        int get_root_process_type() { return root->get_process_type(); }
        void mysh_set_job_id(int new_id ) { this->id = new_id; }
        int id;
        ~job();
};

class shell_info {
    public:
        char cur_dir[CUR_USER_BUFFER];
        bool last_inserted;

        shell_info() {
            for(int i = 0; i < 20; i++) {
                this->jobs_running.push_back(NULL);
            }
        }

        void myshell_promt() { fprintf(stdout, "in-mysh-now:> "); }

        bool insert_history(std::string command);

        void set_command_type(char* command, class process* p);
        int mysh_execute_builtin(class process* p);


        int insert_new_job( class job* job_to_insert ) {
            int id = this->get_available_job_id();

            // Job vector if full
            if (id < 0) { return -1 ; } 
            // Else, we found a spare place to place it

            // Set the job's id to the acquired one
            // And place it into the the shell

            job_to_insert->mysh_set_job_id(id);
            mysh_set_id_to_job(id, job_to_insert);

            return id;
        }


        int remove_job_by_id(int id) {
            if (id > 20 || this->jobs_running[id] == NULL)
                return -1;

            this->jobs_running[id] = NULL;

            return 0;
        }

        int wait_for_job(int id);


    private:
        std::vector<job*> jobs_running;
        std::deque<std::string> history;
        std::map<std::string , std::string> alias;
        void mysh_execute_command_in_history(int n);


        // Internal process, get first available id of job
        int get_available_job_id() {
            for(int i = 1; i <= 20; i++) {
                if (this->jobs_running[i] == NULL) {
                    return i;
                }
            }
            return -1;
        }

        void mysh_set_id_to_job(const int id, class job* job_to_insert ) {
            jobs_running[id] = job_to_insert;
        }

        // Internal functions for aliases
        int mysh_create_alias(char** argv, int argc);
        int mysh_execute_alias(char** argv, int argc);
        int mysh_delete_alias(char** argv, int argc);
        int mysh_show_aliases();

        // Internal functions for history
        void mysh_print_history();
        void mysh_execute_history(char** argv, int argc);

        // Internal functions for cd
        int mysh_cd(char** argv, int argc);


};

namespace mysh {
    // Main shell init function, should be called before any other call to other mysh functions
    void myshell_init();

    // Main shell loop through function    
    void myshell_loop();

    // Main shell read command function
    char* mysh_read_cmd_input();
}
