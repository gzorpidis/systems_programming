# Systems Programming 2022-2023 @ DI UOA

#### Project 1 - Simple shell in C/C++

Grade: 100/100 ✅
Tests provided from professors: Pass

Detailed view:
Goals  | State
------------- | -------------
Basic shell functionality, like command input and basic commands (e.g. `ls` )| ✅
Correct returning and output printing  |  ✅| 
Redirections: >, < , >> | ✅
Pipes: multiple pipes/process and integration with redirection | ✅ 
Background process executions | ✅
Multiple chained commands using ";" (e.g. `sort file1 &; ls &; `) | ✅
Wildcard integration : ? and * | ✅
Aliases: Creating aliases using `createalias` builtin command, deleting aliases using `destroyalias` | ✅
History: Printing last 20 commands using `history` command and execution specific command in history using `history <number>` | ✅
Signals: C^C terminates current running process but not shell process , C^Z places current running process in the background, and gives control back to shell, C^D (EOF) terminates the shell | ✅


#### Project 2 - Voting System in C/C++

Implemented a voting system constisting primarily of a *poller* multithreaded server, which listens on a port specified by the user, and creates `<worker_thread>` number of  threads inside it to handle requests, `queue_buffer_max_size` is the maximum buffer size. When the buffer is full, all requests wait until a request is handled to open a spot in the buffer. When the buffer is empty, worker threads do not work. Thus we have a pool of threads. The main thread just places requests into a buffer which are then handled by worker threads. Main thread also outputs statistics before terminating with C^C, as well as a log file of requests that where handled/votes that where counted.  Communication between server-client is done with a protocol provided by the professors. Communication between main thread and worker threads follows a producer-consumer schema. Also implemented a client to support batch voting on the server.

Lastly, some scripts where created to assist in the debugging of the server.

Grade: 100/100 ✅
Tests provided from professors: Pass


Detailed view:
Goals  | State
------------- | -------------
Master thread: Place reuqests in buffer, create worker threads, update log file, C^C creates stats file and terminates poller | ✅
Worker threads: pooling, retrieving request from buffer, worker-client communication, update log file for vote, update inside structure to use for statistics | ✅
Synchronization: Block master then buffer if full, workers wait when buffer empty | ✅
Client: Multithreaded client implementation, client-server comminication, client-worker thread communication | ✅