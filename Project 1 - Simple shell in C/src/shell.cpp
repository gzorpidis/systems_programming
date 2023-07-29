#include "../include/shell.hpp"

void parse_and_execute(std::string& userInput ) {
    std::vector<std::string> strip_quotes = parsing::parseString(userInput);
    
    strip_quotes = parsing::splitOnAppend(strip_quotes);
    std::vector<std::string> tokens = parsing::tokenize(strip_quotes);
    
    // Now we have the complete token list, which however may contain ";"
    std::vector<std::string> seperated_jobs;
    std::vector<std::vector<std::string>> command_list;
    int jobs = 1;
    for(int i = 0; i < tokens.size();i++) {
        if (tokens[i] == ";" || i == tokens.size()-1) {
            if (i == tokens.size()-1 && tokens[i]!= ";" ) seperated_jobs.push_back(tokens[i]);
            command_list.push_back(seperated_jobs);
            seperated_jobs.clear();
        } else {
            seperated_jobs.push_back(tokens[i]);                
        }
    }
    
    std::vector<std::string> token;
    std::string command_rebuilt = "";
    for(int i = 0; i < command_list.size();i++) {
        token =  command_list[i];
        // Add null termination
        token.push_back("\0");
        
        for(int j = 0; j < token.size(); j++) {
            command_rebuilt += " " + token[j];
        }
        class job* job = mysh_parse_command(token, command_rebuilt);
        if (job == NULL) {
            break;
        }
        int status = mysh_job_exec(job);
        delete job;
        command_rebuilt = "";
    }
}

// SHELL BUILTIN FUNCTION : CD
int shell_info::mysh_cd(char** argv, int argc) {
    if (argc != 3) {
        fprintf(stderr, "mysh: cd takes one argument");
        return 0;
    }
    if (argc == 3) {
        if (chdir(argv[1]) == 0) {
            getcwd(this->cur_dir, sizeof(this->cur_dir));
            printf("Updated directory to: %s\n", this->cur_dir);
            return 0;
        } else {
            fprintf(stderr, "mysh: cd : %s: No such file or directory", argv[1]);
            return 0;
        }
    }
    return 1;
}

// SHELL BUILTIN FUNCTION : HISTORY
// argc == 3 -> "history <number>" was given
// argc == 2 -> "history" was given

void shell_info::mysh_execute_history(char** argv, int argc)  {
    char** line = NULL;
    if ( argc == 3 ) {
        int n = atoi(argv[1]);
        if (n <= 0 || n >= 21) {
            fprintf(stderr, "mysh:: history second argument should be numerinal values between [1,20]\n");
            if (history.size()>0) this->history.pop_front();
            return;
        }
        if (history.size()>0) this->history.pop_front();
        mysh_print_history();
        if (history.size() <= HISTORY) {
            if (!this->last_inserted) {
                mysh_execute_command_in_history(n-1);
            } else {
                mysh_execute_command_in_history(n-1);
            }
        } else {
            if (!this->last_inserted) {
                mysh_execute_command_in_history(n-1);
            } else {
                mysh_execute_command_in_history(n-1);
            }
        }
    } else if (argc == 2) {
        mysh_print_history();
    } else {
        fprintf(stderr, "mysh: history not called with zero or one arguments\n");
    }
}

// mysh_execute_command_in_history -> INTERNAL FUNCTION to call the Nth command
void shell_info::mysh_execute_command_in_history(int n) {
    if (n >= history.size() || n < 0) {
        fprintf(stderr, "history out of bounds \n");
    } else {
        if (history.size() == 0) return;
        std::string command_to_be_executed = history.at(n);
        parse_and_execute(command_to_be_executed);
    }
}

job::~job() {
    struct process *proc, *temp;
    for(proc = root; proc != NULL;) {
        temp = proc->next;
        delete proc;
        proc = temp;
    }
}

job::job(int pgid, int mode, std::string command, class process* process)
:pgid(pgid),
mode(mode),
command(command),
root(process)
{

}

void job::set_root(struct process* new_root)  { this->root = new_root;}


// Shell

// Return true if was present, 
// Return false if it was not inside

bool shell_info::insert_history(std::string command ) {
    std::deque<std::string>::iterator it = find(history.begin(), history.end(),command);
    // If not found in the history continue
    if (it == history.end()) {
        history.push_front(command);
        if (history.size() > HISTORY + 1) {
            std::string removed = history.at(history.size()-1);
            history.pop_back();
        }
        return false;
    } else {
        history.erase(it);
        history.push_front(command);
        if (history.size() > HISTORY + 1) {
            std::string removed = history.at(history.size()-1);
            history.pop_back();
        }
        return true;
    }

    return true;
}

void shell_info::set_command_type(char* command, class process* p){
    // Now check if it's a aliased command
    std::string key = command;
    std::map<std::string,std::string>::iterator it;
    it = alias.find(key);
    if (it != alias.end()) {
        p->type = COMMAND_ALIAS;
    } else {
        if (strcmp(command, "cd") == 0) {
            p->type = COMMAND_CD;
        } else if (strcmp(command, "createalias") == 0) {
            p->type = COMMAND_CREATEALIAS;
        } else if (strcmp(command, "destroyalias") == 0) {
            p->type = COMMAND_DESTROYALIAS;
        } else if (strcmp(command, "history") == 0) {
            p->type = COMMAND_HISTORY;
        } else {
            p->type = COMMAND_EXTERNAL;
        }
    }
}

int shell_info::mysh_create_alias(char** argv, int argc) {
    assert(strcmp(argv[0], "createalias") == 0);
    if ( argc != 4 ) { fprintf(stderr, "createalias: 2 arguments only"); return -1;}
    // Insert to map holding the aliases: <string> -> <string>
    // e.g. if "createalias myhome "cd /home/something" : <cd> -> <"home/something">
    std::string key = argv[1];
    std::string value = argv[2];
    
    // Search if it is inside the aliases
    std::map<std::string,std::string>::iterator it;
    it = alias.find(key);
    // If not found, insert it to the aliases
    if (it == alias.end()) {
        alias.insert(std::pair<std::string,std::string>(key,value));
    }
    else {
        // Dispay error and do not change the value
        fprintf(stderr, "mysh: alias error: there already exists a alias for: %s\n", argv[1]);
    }
    return mysh_show_aliases();
}

int shell_info::mysh_delete_alias(char** argv, int argc) {
    assert(strcmp(argv[0], "destroyalias") == 0);
    if ( argc != 3 ) { fprintf(stderr, "destroyalias: 1 argument only"); return -1;}
    std::map<std::string,std::string>::iterator it;
    std::string key = argv[1];
    int res = alias.erase(key);
    if (res == 0) {
        fprintf(stderr, "mysh: alias error: could not perform alias destruction for: %s\n", argv[1]);
        return -1;
    } else {
        fprintf(stdout, "successfully deleted alias with key: %s\n", argv[1]);
    }
    mysh_show_aliases();
    return 0;
}

int shell_info::mysh_show_aliases() {
    std::map<std::string,std::string>::iterator it;
    for (it=alias.begin(); it!=alias.end(); ++it)
        std::cout << it->first << " => " << it->second << '\n';
    return 1;
}

int shell_info::wait_for_job(int id) {
    if (id > 20 || this->jobs_running[id] == NULL) { return -1; }
    int status = 0, pid = 0;
    bool stopped = false;
    while( (pid = waitpid(-this->jobs_running[id]->pgid, &status, WUNTRACED)) != -1 || errno != ECHILD) {
        if (WIFSTOPPED(status)) {
            // fprintf(stdout, "Child %d was stopped by a signal\n", pid);
            stopped = true;
            break;
        }
    }

    return status;
}

int shell_info::mysh_execute_alias(char** argv, int argc) {
    std::string command_to_be_executed = alias.find(argv[0])->second;
    std::string hold_alias_value = command_to_be_executed;
    assert(parsing::isDoubleQuoted(command_to_be_executed));
    command_to_be_executed.erase(command_to_be_executed.begin());
    command_to_be_executed.erase(command_to_be_executed.end()-1);
    
    alias.erase(argv[0]);
    parse_and_execute(command_to_be_executed);
    // std::cout << "Re-entering alias: <" << std::string(argv[0]) << "-> " << hold_alias_value << ">" << std::endl;
    alias.insert(std::pair<std::string, std::string>(std::string(argv[0]), hold_alias_value));
    return 0;
}

int shell_info::mysh_execute_builtin(class process* p){
    int status = 1;
    switch (p->type) {
        case COMMAND_CD:
            mysh_cd(p->argv, p->argc);
            break;
        case COMMAND_CREATEALIAS:
            mysh_create_alias(p->argv, p->argc);
            break;
        case COMMAND_DESTROYALIAS:
            mysh_delete_alias(p->argv, p->argc);
            break;
        case COMMAND_ALIAS:
            mysh_execute_alias(p->argv, p->argc);
            break;
        case COMMAND_HISTORY:
            mysh_execute_history(p->argv, p->argc);
            break;
        default:
            status = 0;
            break;
    }
    return status;
}

void shell_info::mysh_print_history() {
    int current = 0;
    int MAX = HISTORY - 1;
    int i = (history.size()>MAX) ? MAX : history.size()-1;
    for (i; i >= 0; i--) {
        std::cout << i+1  << ":" <<  history[i] <<  "\n";
        if (++current == MAX + 1) break;
    }
}


char* mysh::mysh_read_cmd_input() {
    int buffer_length = BUFFER;
    int character_read;
    int index_position = 0;
    char *buffer = (char*) malloc(buffer_length * sizeof(char) );

    if (buffer == NULL) {
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (true) {
        character_read = getchar();
        // If reached the end or a new line character
        if (character_read == EOF || character_read == '\n') {
            if (character_read == EOF) {
                exit(EXIT_SUCCESS);
            }
            buffer[index_position] = '\0';
            return buffer;
        } else {
        // Else, place it in the buffer
            buffer[index_position] = character_read;
        }
        // Proceed to the next character
        index_position++;

        // If BUFFERSIZE exceeded, realloc a bigger
        if (index_position >= buffer_length) {
            buffer_length += BUFFER;
            buffer = (char*) realloc(buffer, buffer_length);
            if (buffer == NULL) {
                fprintf(stderr, "mysh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

extern class shell_info* shell;

void sigint_handler(int signal) {
    switch(signal) {
        case SIGCHLD:
            pid_t pid; int status;
            while ((pid = waitpid(-1, &status, WNOHANG|WUNTRACED)) > 0) {
            }
            break;
        case SIGINT:
            fprintf(stdout, "Caught ^C\n");
            fflush(stdout);
            break;
        case SIGTSTP:
            fprintf(stdout, "Caught ^Z\n");
            fflush(stdout);
            break;
    }
}

void mysh::myshell_init(void) {

    struct sigaction sigint_action;
    sigint_action.sa_flags = SA_RESTART;

    sigemptyset(&sigint_action.sa_mask);
    sigaddset(&sigint_action.sa_mask, SIGINT);
    sigaddset(&sigint_action.sa_mask, SIGCHLD);
    sigaddset(&sigint_action.sa_mask, SIGQUIT);
    sigaddset(&sigint_action.sa_mask, SIGTSTP);
    sigaddset(&sigint_action.sa_mask, SIGTTIN);
    sigaddset(&sigint_action.sa_mask, SIGTTOU);


    sigint_action.sa_handler = &sigint_handler;
    sigaction(SIGCHLD, &sigint_action, NULL);

    sigint_action.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &sigint_action, NULL);
    sigaction(SIGINT, &sigint_action, NULL);
    sigaction(SIGQUIT, &sigint_action, NULL);
    sigaction(SIGTTIN, &sigint_action, NULL);
    sigaction(SIGTTOU, &sigint_action, NULL);

    sigint_action.sa_handler = SIG_DFL;
    sigaction(SIGSTOP, &sigint_action, NULL);



    // signal(SIGQUIT, SIG_IGN);
    // signal(SIGTSTP, SIG_IGN);
    // signal(SIGTTIN, SIG_IGN);

    pid_t pid = getpid();

    // leader can be identified by its process group ID == PID
    // set the group id of the process specified by pid to pgid
    setpgid(pid,pid);

    // make it the leader of the group
    // The function tcsetpgrp() makes the process group with process group ID pgrp the foreground  process  group  on
    // the terminal associated to fd
    tcsetpgrp(0, pid);

    shell = new shell_info();
    shell->last_inserted = false;
    getcwd(shell->cur_dir, sizeof(shell->cur_dir));
}

void mysh::myshell_loop() {
    std::string userInput;
    char* line = NULL;

    shell->myshell_promt();

    bool got_back = false;
    while (1) {
        got_back = false;
        int pid;
        int status;

        line = mysh::mysh_read_cmd_input();
        userInput = line;
        
        if (userInput == "") { free(line); continue; }
        if (userInput == "exit") { break; }

        if (userInput != "") {
            shell->last_inserted = shell->insert_history(userInput);
        }
        
        parse_and_execute(userInput);


        if (!got_back) {
            shell->myshell_promt();
        }

        free(line);
    }
    if (line != NULL) free(line);

}

// Function use:
//      Produce a process from a TCL (Token Command List)
class process* mysh_parse_cmd_segment(std::vector<std::string> tcl) {

    // tcl is NOT NULL terminated

    class process* process_created = new process();
    
    int glob_count = 0, position = 0;
    char* input_path = NULL;
    char* output_path = NULL;
    process_created->append = false;
    int mode = FG_EXEC;

    // Now to complete the command, remove any redirections
    std::vector<const char*> commands;

    if (tcl[tcl.size()-1] == "&") {
        mode = BG_EXEC;
        tcl.erase(tcl.end()-1);
    }

    if (tcl[0] == "createalias") {
        // If the command that was given was "createalias"
        // we have a builtin command, with special treatment:
        // First parameter parsed is the alias name-key
        // Everything after that must be inside quotes
        // and no other processing will take place
        // tcl[0] -> "createalias" , tcl[1] -> key , tcl[2] -> values
        
        if (tcl.size() < 3 || tcl.size() > 3) {
            fprintf(stderr, "mysh: createalias takes exactly two parameters\n");
            return NULL;
        } else {  
            commands.push_back(tcl[0].c_str());
            commands.push_back(tcl[1].c_str());
            commands.push_back(tcl[2].c_str());

            if (parsing::isDoubleQuoted(tcl[2]) != true) {
                fprintf(stderr, "mysh: createalias: parameters should be contained in double quotes\n");
                fprintf(stderr, "createalias <KEY> \" <command> \" ");
                return NULL;
            }
        }
    } else if (tcl[0] == "history") {
        if (tcl.size() > 2) {
            fprintf(stderr, "mysh: history takes no or one parameter(s)\n");
            return NULL;
        } else if (tcl.size() == 1) {
            commands.push_back(tcl[0].c_str());
        } else if (tcl.size() == 2) {
            commands.push_back(tcl[0].c_str());
            commands.push_back(tcl[1].c_str());
        }
    } else if (tcl[0] == "cd") {
        if (tcl.size() != 2) {
            fprintf(stderr, "mysh: cd takes one parameter\n");
            return NULL;
        } else if (tcl.size() == 2) {
            commands.push_back(tcl[0].c_str());
            commands.push_back(tcl[1].c_str());
        }
    }
    else {
        for(int i = 0; i < tcl.size();i++) {
            
            // If token is inside double quotes, simply add remove its double quotes, no expanding
            if (parsing::isDoubleQuoted(tcl[i])) {
                parsing::removeDoubleQuotes(tcl[i]);
                continue;
            } 

            // Initialize after every iteration, free when filled
            glob_t globbuf;
            glob_count = 0;

            // Go through every token
            // If there exists a token with a wildcard, expand it using glob
            if (tcl[i].find('*') < tcl[i].length() || tcl[i].find('?') < tcl[i].length()) {

                // If there is a wildcard inside
                // If it's enclosed it single quotes, do not apply expand rule
                if ( tcl[i][0] == '\'') {
                    continue;
                }

                glob( tcl[i].c_str() , 0 , NULL, &globbuf);
                glob_count = globbuf.gl_pathc;

                // Remove the token with the wildcard
                tcl.erase(tcl.begin()+i);
                position--;
            } else if (tcl[i].at(0) == '$') {
                std::string new_string = tcl[i];
                new_string.erase(new_string.begin());
                std::string output = getenv(new_string.c_str());
                tcl.erase(tcl.begin()+i);
                tcl.insert(tcl.begin() + i, output);
            } else {
                // Anything else without a *
                position++;
            }

            if (glob_count > 0) {
                // Expand the wildcard, placing the expanded paths in the place of the *
                for(int j = 0; j < glob_count; j++) {
                    assert(globbuf.gl_pathv[j] != NULL);
                    std::string str(globbuf.gl_pathv[j]);
                    tcl.insert(tcl.begin() + position + 1 + j, globbuf.gl_pathv[j]);
                }
                globfree(&globbuf);
            }

        }

        for(int i = 0 ; i < tcl.size();++i) {
            // If it's a redirection , save the input and output paths, but don't put into the
            // command
            if (tcl[i] == ">" || tcl[i] == "<" || tcl[i] == ">>") {
                if (tcl[i] == ">") {
                    if (tcl.size() <= i+1) {
                        fprintf(stderr, "mysh: No file given for output\n");
                        return NULL;
                    }
                    output_path = (char*) malloc(sizeof(char)*(tcl[i+1].length() + 1));
                    strcpy(output_path, tcl[i+1].c_str() );
                    i++;
                    process_created->append = false;
                } else if (tcl[i] == "<") {
                    if (tcl.size() <= i+1) {
                        fprintf(stderr, "mysh: No file given for input\n");
                        return NULL;
                    }
                    input_path = (char*) malloc(sizeof(char)*(tcl[i+1].length() + 1));
                    strcpy(input_path, tcl[i+1].c_str() );
                    i++;
                } else if (tcl[i] == ">>") {
                    if (tcl.size() <= i+1) {
                        fprintf(stderr, "mysh: No file given for appending output\n");
                        return NULL;
                    }
                    output_path = (char*) malloc(sizeof(char)*(tcl[i+1].length() + 1));
                    strcpy(output_path, tcl[i+1].c_str() );
                    i++;
                    process_created->append = true;
                }
            } else {
                // If there isn't a redirection symbol we take it a part of the command's parameters
                commands.push_back(tcl[i].c_str());
            }
        }
    }

    // process_created->infd = input_path;
    // process_created->outfd = output_path;
    // process_created->pid = -1;
    // process_created->argc = commands.size() + 1;
    // process_created->argv = (char**) malloc(sizeof(char*) * (commands.size()+1) );
    // process_created->set_next(NULL);
    // process_created->mode = mode;

    process_created->build_command(input_path, output_path, -1, commands.size()+1, (char**) malloc(sizeof(char*) * (commands.size()+1)), mode);
    for(int i = 0; i < commands.size();i++) {
        process_created->argv[i] = strdup(commands[i]);
    }
    process_created->argv[commands.size()] = NULL;

    // Get the command/process type from the shell
    // Because it might be aliased, and the alias lives inside the shell
    shell->set_command_type(process_created->argv[0], process_created);

    process_created->command = strdup(process_created->argv[0]);
    return process_created;
}

// A job consists of many jobs seperated by pipes ("|")
// tcl -> Token Command List, NULL TERMINATED
// inside it, call parse_cmd_segment to get the many processes that make up the job

// Function use:
//      Create the job structure that represents a command given by the user
//      Given a TCL (Token Command List) and the whole command
class job* mysh_parse_command(std::vector<std::string> tcl, std::string command_given) {
    
    // By default the mode of execution in foreground
    int mode = FG_EXEC;

    // Check for background execution, by searching for "&" 
    // If it is present in the last position of the TCL set execution mode to background
    // tcl is NULL terminated, so one before the end
    mode = (tcl.size() >= 2 && tcl[tcl.size()-2] == "&") ? BG_EXEC : mode;

    // // Check for background execution
    // // tcl is NULL terminated, so one before
    // if (tcl.size() >= 2) {
    //     if (tcl[tcl.size()-2] == "&") {
    //         mode = BG_EXEC;
    //     }
    // }

    // A job is a series of processes
    // which are connected as a linked list
    class process* root = NULL, *piped = NULL;

    // Here have the individual commands/processes
    std::vector<std::string> cmd;
    int i = 0;
    for(i = 0; i < tcl.size(); i++) {

        // Use cmd to accumulate the string built up to the pipe operator

        // If we get a piping operator or reach the end, we should split,
        // creating the process
        if ( (tcl[i] == "|" ) || tcl[i] == "\0") {
            
            if (i == 0) {
                fprintf(stderr, "Pipe operation at wrong place");
                return NULL;
            }

            class process* new_proc = mysh_parse_cmd_segment(cmd);

            if (new_proc == NULL) {
                fprintf(stderr, "mysh:: exiting with parse error\n");
                return NULL;
            }

            // This is the very first process
            if (root == NULL) {
                root = new_proc;
                piped = root;
            }
            else {
                // Set old process to have its new as
                // the newly created
                piped->set_next(new_proc);
                piped = new_proc;
            }
            // Start from scratch again
            cmd.clear();
        }
        else {

            if (tcl[i] == "&") {
                mode = BG_EXEC;
                continue;
            }
            // If we don't get a pipe, we push to the current cmd
            cmd.push_back(tcl[i]);
        }
    }

    // Initialize job
    //      -1 -> No process group yet
    //      mode -> Mode of execution as concluded from above
    //      command_given -> The whole command
    //      root -> The root of the job begins with root 
    job* job_created = new job(-1, mode, command_given, root);

    // // Doesn't have a group id yet
    // ret->pgid = -1;

    // // And will be executed with mode
    // ret->mode = mode;
    // ret->command = command_given;

    // // And set the root of the job to be from the root process
    // ret->set_root(root);

    return job_created;
}

//  Function use:
//      Execute the job "job"
int mysh_job_exec(class job* job) {
    
    class process* proc = NULL;
    
    int status, in_fd = STDIN_FILENO, job_id = -1;
    
    int procs = 0, all_external = 0;
    
    if (job->root->get_process_type() == COMMAND_EXTERNAL && job->mode != BG_EXEC) {
        job_id = shell->insert_new_job(job);
        if (job_id == -1) {
            fprintf(stderr, "Error\n");
            return -1;
        }
    }

    for(proc = job->root; proc != NULL; proc = proc->get_next_process()) {
        if (proc->get_process_type() != COMMAND_EXTERNAL) all_external = 1;
        procs++;
    }

    // Not allowed:
    //      pipelined processes out of which at least one is internal
    // Allowed:
    //      as many pipelines as possible for all external commands
    if (all_external == 1 && procs > 1) {
        fprintf(stderr, "mysh: pipelines can't contain internal commands\n");
        return -1;
    }

    int pipes = procs - 1, *pipefds = NULL, command_processed = 0;
    if (pipes > 0) pipefds = new int[2 * pipes];


    // Create the pipes from the pipefds array
    for(int i = 0; i < pipes; i++) {
        // E.g. for i = 0 we create a pipe using
        // pipefds, and it will use the first two positions for the pipe
        // 0 > READ
        // 1 > WRITE
        // if i = 1, we create a pipe using pipefds + 2
        if (pipe(pipefds + i*2) < 0) {
            perror("pipe creation failed");
            return EXIT_FAILURE;
        }
    }
    
    proc = NULL;

    for(proc = job->root; proc != NULL; proc = proc->get_next_process()) {
        // If it's the first and it has input, open the appropriate file
        if (proc == job->root) {
            if ( proc->infd != NULL ) {
                in_fd = open(proc->infd, O_RDONLY);
                if (in_fd < 0) {
                    fprintf(stderr, "mysh: no such file or directory: %s\n", proc->infd);
                    return -1;
                }
            } else {
                in_fd = 0;
            }
        }

        // If we have a next process, 
        // this goes for all except the last,
        // we want to create the pipes
        if (proc->get_next_process() != NULL) {
            if (proc == job->root) {
                if (pipes > 0)
                    status = mysh_proc_exec(job, proc, in_fd, pipefds[(command_processed*2+1)], PL_EXEC , pipefds, command_processed, pipes);
                else
                    status = mysh_proc_exec(job, proc, in_fd, pipefds[(command_processed*2+1)], job->mode , pipefds, command_processed, pipes);
            } else {
                status = mysh_proc_exec(job, proc, pipefds[(command_processed-1)*2], pipefds[(command_processed*2+1)], PL_EXEC , pipefds, command_processed, pipes);
            }
        } else {
            // We are the very last process
            int out_fd = STDOUT_FILENO;
            if (proc->outfd != NULL) {
                int flags = O_WRONLY | O_CREAT;
                flags =  (proc->append) ? flags | O_APPEND : flags | O_TRUNC; 
                out_fd = open(proc->outfd, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (out_fd < 0) { perror("open out_fd"); return -1 ; }
            }
            // Now call the process with the appropriate mode, 
            // if it's foreground -> will wait for all processes to end
            // if background -> no waiting

            // If only one process is present, it will get here
            // but we have already taken care of its stdin so skip
            if (proc != job->root)
                in_fd = ((command_processed-1)*2 < 0) ? STDIN_FILENO : pipefds[(command_processed-1)*2];
            
            status = mysh_proc_exec(job, proc, in_fd, out_fd, job->mode, pipefds, command_processed, pipes);
        }
        command_processed++;
    }
    
    delete[] pipefds;

    // If the entered command is BACKGROUND, no need to remove, was not even entered
    // Else, remove it 
    if (job->get_root_process_type() == COMMAND_EXTERNAL && job->mode == FG_EXEC) {
        if (status >= 0) {
            shell->remove_job_by_id(job_id);
        }
    }

    return status;
}

//  Function use: 
//      Execute process "proc"
//      belonging to job "job", 
//      given the pipes, filedescriptors for input and output, 
//      its mode of execution (inherited from its parent job)
//      its number on the pipelined command, and the number of pipes in the command

int mysh_proc_exec(struct job *job, struct process *proc, int in_fd, int out_fd, int mode, int* pipefds, int command_processed, int pipes) {

    // If process is a built in command , execute it differently
    if (proc->get_process_type() != COMMAND_EXTERNAL) {
        shell->mysh_execute_builtin(proc);
        return 0;
    }

    pid_t childpid;
    int status = 0;
    childpid = fork();

    if (childpid < 0) {
        perror("Creating child process");
        return -1;
    } else if (childpid == 0) {
        
        struct sigaction act;
        sigfillset(&act.sa_mask);
        act.sa_flags = SA_RESTART;

        act.sa_handler = SIG_DFL;
        sigaction(SIGINT, &act, NULL);
        sigaction(SIGSTOP, &act, NULL);
        sigaction(SIGTSTP, &act, NULL);
        sigaction(SIGQUIT, &act, NULL);
        sigaction(SIGCHLD, &act, NULL);

        // Should ignore TTIN and TTOU before calling tcsetpgrp as seen on manual
        act.sa_handler = SIG_IGN;
        sigaction(SIGTTIN, &act, NULL);
        sigaction(SIGTTOU, &act, NULL);

        // Set the process id to the created one, found by using getpid()
        proc->set_pid(getpid());

        // Will only get executed once per job
        // to set the job's group id to that of the first process' ID

        // Inherit if it has been set by a previous process
        if (job->pgid > 0) {
            setpgid(0, job->pgid);
        } else
        // Else set it for the first time
        {
            job->pgid = proc->pid;
            setpgid(0, job->pgid);
        }

        // If foreground execution is mandatory/set we place it to the foreground process group on the terminal
        if (job->mode == FG_EXEC)
            tcsetpgrp(0, job->get_pgid());
        
        // Now having called tcsetpgrp, we can restore the default behaviour for TTIN,TTOU
        act.sa_handler = SIG_DFL;
        sigaction(SIGTTIN, &act, NULL);
        sigaction(SIGTTOU, &act, NULL);

        if (pipes == 0) {
            if (in_fd != 0) {
                int err = dup2(in_fd, 0);
                close(in_fd);
                if (err < 0) { perror("not connected to output"); return EXIT_FAILURE;}
            }
            if (out_fd != 1) {
                int err = dup2(out_fd, 1);
                close(out_fd);
                if (err < 0) { perror("not connected to output"); return EXIT_FAILURE;}
            }
        } else 
        // If pipes do exist, then we should be able to chain inputs and outputs
        {
            // If not first process, it gets its output from whatever is given in in_fd
            if (command_processed != 0) {
                if (in_fd != 0) {
                    int err = dup2(in_fd, 0);
                    if (err < 0) { perror("not connected to input"); return EXIT_FAILURE;}
                }
            }

            // If not last process, it outputs to whatever is in out_fd
            if (command_processed != pipes) {
                int err = dup2(out_fd, 1);
                if (err < 0) { perror("not connected to output"); return EXIT_FAILURE;}
            }

            // If last process, it may output somewhere
            if (command_processed == pipes) {
                if (out_fd != 1) {
                    int err = dup2(out_fd, 1);
                    if (err < 0) { perror("not connected to output"); return EXIT_FAILURE;}
                }
            }
        }

        // Close dangling file descriptors 
        for (int i = 0; i < 2*pipes; i++) close(pipefds[i]);
        
        // Execute the command
        if (execvp(proc->argv[0], proc->argv) < 0) {
            printf("mysh: %s: command not found\n", proc->argv[0]);
            exit(0);
        }

        assert(false);
        exit(0);

    } else {
        proc->set_pid(childpid);

        if (job->pgid > 0) {
            setpgid(childpid, job->pgid);
        } else {
            job->pgid = proc->pid;
            setpgid(childpid, job->pgid);
        }

        // If we are at the last process of the piped command
        // close pipes as needed
        if (mode == FG_EXEC || mode == BG_EXEC) {
            if (pipes == 0) {
                if (in_fd != 0)
                    close(in_fd);
                if (out_fd != 1)
                    close(out_fd);
                    
                for (int i = 0; i < 2*pipes; i++) {
                    close(pipefds[i]); 
                }
            }
            else {
                for (int i = 0; i < 2*pipes; i++) {
                    close(pipefds[i]); 
                }
            }
        }

        if (mode == FG_EXEC) {
            // If foreground, set it to have control of the terminal
            tcsetpgrp(0, job->get_pgid());
            status = shell->wait_for_job(job->id);
            // Get back control
            tcsetpgrp(0, getpid());
        }

    }

    return status;
}