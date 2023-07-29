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