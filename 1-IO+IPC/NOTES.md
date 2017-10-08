# Project 1A Notes

## 1 character at a time
As long as you type in one character, your program will be able to see it  
while(true) {  
    read()  
}  
read every single character from the I/O queue  
you can use read(1, buff, 512);

MIN value should be 1 (because we are reading one char at a time)  
TIME value should be 0 because we are blocking while waiting for input

## atexit

you can pass a function pointer to atexit  
When you call exit in your code, every function you passed to atexit will be called

## clflag
Comment out the clflag thing in the code and copy paste the three lines that prof provides

## waitpid()
pass 0 to waitpid(); Parent process is gonna block until child precess finishes  

## WEXITSTATUS()
you can only call this if WIFEXITED is true
What is the _status_ variable? Its the number you pass to exit()

## execvp
executes the string command and starts running a new process  
everything in your current process is wiped!

## pipe
pipe() used for interprocess communication  
call pipe before fork  

the child process then inherits the fd table from the parent process! Now you can do IPC  
The parent process can write data into a fd, then the child process can read from that file  
We need to use pipes  
1) to transfer data from parent to child
2) to transfer data from child to parent

Close unused FDs!!  
in the child process, after calling dup2(), close fd 0 and 1  

## poll

for(;;) {  
    ret = pol(fd, 2, -1)  
    if (ret > 0) {  
    }  
}
