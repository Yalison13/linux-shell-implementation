# 🐧 Linux Shell Implementation

C-based Linux shell supporting built-in commands, external execution, background jobs, I/O redirection, and command history.
---

## ✨ Overview

This project implements a mini Linux shell from scratch, simulating how real shells (like Bash) work internally.

It handles:

* Command parsing
* Process creation
* Environment variables
* Background execution
* File redirection

---

## 🚀 Features

### 🖥️ Shell Behavior

* Custom prompt:

  ```
  user@hostname:/path$
  ```
* Continuous command loop
* Exit with `exit`

---

### ⚙️ Built-in Commands

* `cd` – change directory
* `pwd` – print current directory
* `export` – set environment variables
* `history` – show command history
* `sbg` – list background processes

---

### 🔧 System Features

* Execute external commands (`ls`, `cat`, etc.)
* Background execution using `&`
* Output redirection:

  * `>` overwrite
  * `>>` append
* Environment variable expansion (`$HOME`, `$PATH`)
* Command history (↑ ↓ navigation via readline)

---

## 🧠 System Design

### Process Execution

* Uses `fork()` to create child process
* Uses `execvp()` to execute commands
* Uses `waitpid()` to handle synchronization

### Background Jobs

* Stores PID + command
* Uses `WNOHANG` to detect finished processes

### I/O Redirection

* Uses `open()` + `dup2()` to redirect stdout

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

* How a shell works internally
* Process management in Unix
* System calls and file descriptors
* Command parsing and execution flow

---

## 📌 Notes

This project was developed as part of an Operating Systems course, focusing on understanding how Unix shells are implemented.
