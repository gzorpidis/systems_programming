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
#include "../include/shell.hpp"

class shell_info* shell;

int main() {

    mysh::myshell_init();
    mysh::myshell_loop();

    delete shell;
    return 0;
}