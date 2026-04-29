# 🐧 Linux Shell Implementation

A custom mini Linux shell implemented in C, designed to demonstrate core Operating Systems concepts such as process management, command execution, I/O redirection, and environment variable handling.

---

## ✨ Overview

This project recreates a simplified Unix shell from scratch, focusing on how commands are parsed and executed at the system level.

It simulates essential behaviors of real shells like Bash, including process control, background execution, and file redirection.

---

## 🚀 Features

### 🖥️ Shell Behavior

* Custom prompt format:

  ```
  user@hostname:/path$
  ```
* Interactive command loop until `exit`
* Command history navigation (↑ / ↓) via GNU Readline

---

### ⚙️ Built-in Commands

* `cd` – change current working directory
* `pwd` – print working directory
* `export` – set or update environment variables
* `history` – display command history
* `sbg` – list running background processes
* `exit` – terminate the shell

---

### 🔧 Advanced Capabilities

* Execute external commands (`ls`, `cat`, `grep`, etc.)
* Background execution using `&`
* Output redirection:

  * `>` overwrite file
  * `>>` append to file
* Environment variable expansion (`$HOME`, `$USER`, `$PATH`)
* Integration with system `$PATH`

---

## 🧠 System Design

### Process Management

* Uses `fork()` to create child processes
* Uses `execvp()` to execute commands
* Uses `waitpid()` for foreground process synchronization

---

### Background Job Handling

* Maintains a list of active background processes (PID + command)
* Uses `WNOHANG` to detect completed processes
* Prevents zombie processes during execution

---

### I/O Redirection

* Uses `open()` to create or modify output files
* Uses `dup2()` to redirect `STDOUT_FILENO`

---

### Command Parsing

* Tokenized using `strtok`
* Supports:

  * arguments
  * background `&`
  * redirection `>` / `>>`

---

## 🛠️ Tech Stack

* C
* Linux / Unix system calls
* GNU Readline
* Git

---

## 📦 Build

```bash
gcc linux-shell-implementation.c -o shell -lreadline
```

---

## ▶️ Demo

```bash
$ ./shell
user@host:~$ pwd
/home/user

user@host:~$ echo $HOME
/home/user

user@host:~$ sleep 5 &
[Process running in background with PID 1234]

user@host:~$ sbg
Background processes:
[1] PID: 1234, Command: sleep
```

---

## 📁 Project Structure

```
.
├── linux-shell-implementation.c
├── README.md
└── .gitignore
```

---

## 🎯 What I Learned

* How Unix shells work internally
* Process creation and lifecycle management
* System calls and file descriptor manipulation
* Command parsing and execution flow

---

## 📌 Notes

This project was developed as part of an Operating Systems course, focusing on understanding how a Linux shell is implemented at a low level.
