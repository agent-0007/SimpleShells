**SimpleTCPBindShell.c** is a TCP Bind Shell written in C. I wasn't able to find any proper bind shell in Google, so I've quickly written my own.

The purpose of this repository is to help conducting penetration tests on authorized hosts. Otherwise, the code may be used for educational purposes only.

Features
---------------------------
* The code is written in pure C and can be compiled for any architecture (x86/PowerPC/MIPS)
* The usage of the MUSL library is tested
* The shell is protected by a password
* The shell can be terminated by a special keyword even when a command execution is not available
* The command-line interpreter is executed with a proper `argv[0]`, so it should work on BusyBox
* The shell properly detaches from a parent process and doesn't hand the execution flow
* Multiple shell sessions can be opened at the same time
* The bind port can be automatically added to iptables

Usage
---------------------------
**Command Execution**
```
$ ncat 127.0.0.1 65002
2023
id
uid=0(root) gid=0(root) groups=0(root)
```

**Process Termination**
```
$ ncat 127.0.0.1 65002
2023T
Ncat: Broken pipe.
```
