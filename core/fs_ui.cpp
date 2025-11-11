#include "fs_ui.h"
#include <cstdio>
#include <iostream>
using namespace std;

void help() {
    printf("command: \n\
    help   ---  show help menu \n\
    cd     ---  change the shell working directory \n\
    clear  ---  clear the screen \n\
    ls     ---  show all the files and directories in particular directory \n\
    mkdir  ---  create the directory,if it does not already exist.   \n\
    touch  ---  create a new file \n\
    gedit  ---  read and write something to a particular file \n\
    rm     ---  delete a exist file or a directory\n\
    cp     ---  copy SOURSE to DEST \n\
    rename ---  rename a exist file or a directory \n\
    su     ---  switch current user \n\
    exit   ---  exit this system \n\
    ln     ---  create link (ln -s TARGET LINKNAME | ln -h TARGET LINKNAME)\n\
    checklink ---  verify link integrity and counts\n");
}

void help2(string command) {
    if (command == "cd?") printf("cd: cd [dir]\nchange the shell working directory.\n");
    else if (command == "ls?")printf("ls: ls [dir]\nShow all the files and directories in particular directory.\n");
    else if (command == "mkdir?") printf("mkdir: mkdir dir\nCreate the directory,if it does not already exist.\n");
    else if (command == "touch?")printf("touch: touch file\nCreate a new file,if it does not already exist.\n");
    else if (command == "gedit?")printf("gedit: gedit file\nRead and write something to a particular file.\n");
    else if (command == "rm?")printf("rm: rm -d|-f|-l file|dir|link\nDelete a exist file or a directory or a link.\noption:\n-d:delete a directory\n-f:delete a file\n-l:delete a symbolic link\n");
    else if (command == "cp?")printf("cp: cp -d|-f|-cd|-cf SOURSE DEST\nCopy SOURSE to DEST\noption:\n-d:copy a directory\n-f:copy a file\n-cd:cut a directory\n-cf:cut a file\n");
    else if (command == "rename?")printf("rename: rename -d|-f oldname newname\nRename a exist file or a directory.\noption:\n-d:rename a directory\n-f:rename a file\n");
    else if (command == "su?")printf("Switch current user.\n");
    else if (command == "clear?")printf("Clear the screen.\n");
    else if (command == "exit?")printf("Exit this system.\n");
    else if (command == "ln?")printf("ln: ln -s TARGET LINKNAME | ln -h TARGET LINKNAME\nCreate a symbolic or hard link.\nTARGET supports absolute(~,/...) and relative path.\nHard link must target a file.\n");
    else if (command == "checklink?")printf("checklink: check hardlink counts and broken symlinks.\n");
    else printf("Wrong Command!\n");
}