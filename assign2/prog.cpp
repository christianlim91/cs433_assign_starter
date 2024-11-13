/**
 * Assignment 2: Simple UNIX Shell
 * @file prog.cpp
 * @author Noya Hafiz, Christian Lim
 * @brief This is the main function of a simple UNIX Shell. You may add additional functions in this file for your implementation
 * @version 0.1
 */

// Include necessary libraries
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <sys/wait.h>
#include <vector>

using namespace std;

#define MAX_LINE 80 // Maximum command length

//history to store executed commands
vector<string> history;

//function to parse command and arguments
int ParseCommand(char command[], char *args[]){
    int i = 0;
    //splits command into tokens
    char *token = strtok(command, " \n"); 
    while(token != nullptr){
    //stores the token in args
        args[i++] = token; 
        token = strtok(nullptr, " \n");
    }
    //null terminates args
    args[i] = nullptr; 
    //returns the number of arguments
    return i; 
}
//function to execute a command
void ExecuteCommand(char *args[], bool background){
    //creates a new child process
    pid_t pid = fork(); 
    if(pid < 0){
        //handles fork error
        perror("Fork failed"); 
        return;
    }
    if(pid == 0){
        //executes command in child process
        execvp(args[0], args); 
        //handles  execvp failure
        perror("Command not found"); 
         //exits child process on failure
        exit(1); 
    }else{
        //now in parent process, waits for child if not running in the background
        if(!background) wait(nullptr); 
    }
}
//function to execute commands with pipes
void ExecuteWithPipes(char *args[], int pipeIndex){
    //file descriptors for pipe
    int pipefd[2]; 
    if(pipe(pipefd) == -1){
        //handles pipe errors
        perror("Pipe failed"); 
        return;
    }

    //forks first child
    pid_t pid1 = fork(); 
    if(pid1 == -1){
        perror("Fork failed");
        return;
    }

    if(pid1 == 0){
        //first child process
        //ends the first command before the pipe
        args[pipeIndex] = nullptr;
        //redirects stdout to pipe write end
        dup2(pipefd[1], STDOUT_FILENO);
        //close unused read end
        close(pipefd[0]);
        //close write end after redirecting
        close(pipefd[1]); 
        //execute first command
        execvp(args[0], args); 
        //handles exec failure
        perror("Command not found"); 
        exit(1);
    }

    //forks second child 
    pid_t pid2 = fork(); 
    if(pid2 == -1){
        perror("Fork failed");
        return;
    }

    if(pid2 == 0){
        //second child process
        //redirects stdin to pipe read end
        dup2(pipefd[0], STDIN_FILENO); 
        //close unsued write end
        close(pipefd[1]); 
        //close read end after redirecting
        close(pipefd[0]); 
        //executes second command
        execvp(args[pipeIndex + 1], &args[pipeIndex + 1]);
        //handles exec failure
        perror("Command not found"); 
        exit(1);
    }

    //parent process
    //close pipe ends in parent
    close(pipefd[0]); 
    close(pipefd[1]);
    //waits for first child to finish
    waitpid(pid1, nullptr, 0); 
    //waits for second child to finish
    waitpid(pid2, nullptr, 0); 
}
//function to handle input/output redirection
void HandleRedirection(char *args[], int num_args){
    for(int i = 0; i < num_args; i++){
        //output redirection
        if(strcmp(args[i], ">") == 0){
            //terminates args before redirection
            args[i] = nullptr; 
            //open file for writing
            int fd = open(args[i + 1], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
            if(fd < 0){
                //handles file open error
                perror("Error opening file for output redirection"); 
                return;
            }
            //redirect stdout to file
            dup2(fd, STDOUT_FILENO); 
            close(fd);
            //break after redirection
            break; 
        }
        //input redirection
        else if(strcmp(args[i], "<") == 0){
            //terminate args before redirection
            args[i] = nullptr; 
            //open file for reading
            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) {
                //handles file open error
                perror("Error opening file for input redirection"); 
                return;
            }
            //redirect stdin to file
            dup2(fd, STDIN_FILENO); 
            close(fd);
            //break after redirection
            break; 
        }
    }
}

//main function of a simple UNIX Shell
int main(int argc, char *argv[]){
    //command entered by user
    char command[MAX_LINE];       /
    //holds parsed command line arguments
    char *args[MAX_LINE / 2 + 1]; 
    //flag that determines when to exit the program
    int should_run = 1;           

    while(should_run){
        //prompt for user input
        printf("osh> "); 
        fflush(stdout);
        //exit on EOF
        if(fgets(command, MAX_LINE, stdin) == nullptr) break; 

        //stores command in history
        history.push_back(string(command)); 
        //parse input command
        int num_args = ParseCommand(command, args); 

        if(strcmp(args[0], "exit") == 0){
            //exits the shell
            should_run = 0; 
        } 
        else if(strcmp(args[0], "!!") == 0){
            //executes previous command if history is not empty
            if(history.size() > 1){
                //prints previous command
                cout << history[history.size() - 2]; 
                ParseCommand((char*)history[history.size() - 2].c_str(), args);
                //executes without background
                ExecuteCommand(args, false); 
            }else{
                //no history found
                cout << "No command history found." << endl; 
            }
        } 
        else{
            //flag for background execution
            bool background = false;
            if(num_args > 0 && strcmp(args[num_args - 1], "&") == 0){
                //properly remove '&' from args
                args[num_args - 1] = nullptr; 
                //sets background flag
                background = true; 
            }
            //index of the pipe if present
            int pipeIndex = -1; 
            for(int i = 0; i < num_args; i++){
                if(strcmp(args[i], "|") == 0){
                    //records the position of the pipe
                    pipeIndex = i; 
                    break;
                }
            }
            if(pipeIndex != -1){
                //if pipe is found, execute with pipe
                ExecuteWithPipes(args, pipeIndex);
            }else{
                //handles redirection
                HandleRedirection(args, num_args); 
                //execute command
                ExecuteCommand(args, background);
           }
        }
    }
    //prints command history
    cout << "Command history:" << endl; 
    //print each command from history
    for (size_t i = 0; i < history.size(); i++) {
        //print each command followed by a newline
        cout << history[i] << endl; 
    }

    //returns exit status
    return 0; 
}

