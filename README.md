# linux-shell-implementation

C-based Linux shell supporting built-in commands, external execution, background jobs, I/O redirection, and command history.



\# Linux Shell Implementation



A simple Linux shell implemented in C for an Operating Systems course project.  

This project demonstrates basic shell behavior, process creation, command execution, I/O redirection, background process management, and command history.



\## Features



\- Custom shell prompt showing username, hostname, and current working directory

\- Built-in commands:

&#x20; - `cd`

&#x20; - `pwd`

&#x20; - `export`

&#x20; - `history`

&#x20; - `sbg`

&#x20; - `exit`

\- External command execution using `fork()` and `execvp()`

\- Background execution using `\&`

\- Background process tracking with `sbg`

\- Output redirection:

&#x20; - `>` overwrite output file

&#x20; - `>>` append to output file

\- Command history support using GNU Readline

\- Environment variable expansion, such as `$HOME` or `$PATH`



\## Technologies Used



\- C

\- Linux / Unix system calls

\- GNU Readline

\- Git



\## Build



```bash

gcc linux-shell-implementation.c -o shell -lreadline

