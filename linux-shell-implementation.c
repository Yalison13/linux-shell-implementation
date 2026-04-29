#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_BG_PROCESSES 100

typedef struct {
    pid_t pid;
    char command[256];
} BackgroundProcess;

BackgroundProcess bg_processes[MAX_BG_PROCESSES];
int bg_count = 0;

// -------------------- Utility functions --------------------
char *expand_variables_alloc(const char *command) {
    size_t cap = 256;
    char *out = malloc(cap);
    if (!out) return NULL;
    size_t out_len = 0;

    const char *p = command;
    while (*p) {
        if (*p == '$') {
            // 取得變數名
            p++;
            char name[256] = {0};
            size_t ni = 0;
            while (*p && (isalnum((unsigned char)*p) || *p == '_')) {
                if (ni < sizeof(name)-1) name[ni++] = *p;
                p++;
            }
            name[ni] = '\0';
            if (ni == 0) {
                // 單獨 '$' 保留它
                if (out_len + 1 >= cap) {
                    cap *= 2;
                    char *tmp = realloc(out, cap);
                    if (!tmp) { free(out); return NULL; }
                    out = tmp;
                }
                out[out_len++] = '$';
            } else {
                char *val = getenv(name);
                if (val) {
                    size_t vlen = strlen(val);
                    while (out_len + vlen + 1 >= cap) {
                        cap *= 2;
                        char *tmp = realloc(out, cap);
                        if (!tmp) { free(out); return NULL; }
                        out = tmp;
                    }
                    memcpy(out + out_len, val, vlen);
                    out_len += vlen;
                }
                // 如果 getenv 沒有值就跳過
            }
        } else {
            if (out_len + 2 >= cap) {
                cap *= 2;
                char *tmp = realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            out[out_len++] = *p++;
        }
    }
    out[out_len] = '\0';
    return out;
}

void update_path(const char *new_path) {
    char *current_path = getenv("PATH");
    if (!current_path) current_path = "";
    size_t needed = strlen(current_path) + 1 + strlen(new_path) + 1;
    char *updated_path = malloc(needed);
    if (!updated_path) return;
    snprintf(updated_path, needed, "%s:%s", current_path, new_path);
    setenv("PATH", updated_path, 1);
    free(updated_path);
}

void print_prompt_str(char *buf, size_t size) {
    char hostname[HOST_NAME_MAX];
    char cwd[PATH_MAX];
    struct passwd *pw = getpwuid(getuid());
    const char *username = pw ? pw->pw_name : getenv("USER");
    const char *homedir = pw ? pw->pw_dir : getenv("HOME");

    if (gethostname(hostname, sizeof(hostname)) != 0)
        strncpy(hostname, "host", sizeof(hostname));
    if (!getcwd(cwd, sizeof(cwd)))
        strncpy(cwd, "?", sizeof(cwd));

    if (username == NULL) username = "user";
    if (homedir == NULL) homedir = "";

    if (strcmp(cwd, homedir) == 0)
        snprintf(buf, size, "%s@%s:~$ ", username, hostname);
    else if (strncmp(cwd, homedir, strlen(homedir)) == 0 && strlen(homedir) > 0)
        snprintf(buf, size, "%s@%s:~%s$ ", username, hostname, cwd + strlen(homedir));
    else
        snprintf(buf, size, "%s@%s:%s$ ", username, hostname, cwd);
}

// -------------------- Background management --------------------
void add_background_process(pid_t pid, const char *command) {
    if (bg_count < MAX_BG_PROCESSES) {
        bg_processes[bg_count].pid = pid;
        strncpy(bg_processes[bg_count].command, command, 255);
        bg_processes[bg_count].command[255] = '\0';
        bg_count++;
    }
}

void check_background_processes() {
    for (int i = 0; i < bg_count; i++) {
        pid_t r = waitpid(bg_processes[i].pid, NULL, WNOHANG);
        if (r > 0) {
            printf("[PID %d] %s finished\n", bg_processes[i].pid, bg_processes[i].command);
            bg_processes[i] = bg_processes[--bg_count];
            i--;
        }
    }
}

void list_background_processes() {
    if (bg_count == 0)
        printf("No background processes running\n");
    else {
        printf("Background processes:\n");
        for (int i = 0; i < bg_count; i++)
            printf("[%d] PID: %d, Command: %s\n", i + 1, bg_processes[i].pid, bg_processes[i].command);
    }
}

// -------------------- Main shell --------------------
int main() {
    char *input;
    char prompt[512];
    char *args[64];

    using_history(); // 初始化 readline history

    while (1) {
        check_background_processes();

        print_prompt_str(prompt, sizeof(prompt));
        input = readline(prompt);

        if (!input) break; // Ctrl+D
        if (strlen(input) == 0) { free(input); continue; }

        add_history(input); // 加入歷史

        // exit
        if (strcmp(input, "exit") == 0) {
            free(input);
            break;
        }

        // 將展開結果放入新的 buffer 並替換 input
        char *expanded = expand_variables_alloc(input);
        if (expanded) {
            free(input);
            input = expanded;
        } // 若 expand 失敗 繼續使用原 input

        // 檢查背景執行
        int background = 0;
        size_t ilen = strlen(input);
        if (ilen > 0 && input[ilen - 1] == '&') {
            background = 1;
            input[ilen - 1] = '\0';
        }

	// ==================== Redirection handling ====================
	char *output_file = NULL;
	int append = 0;

	// 找出重導向符號（>> 或 >）
	char *redir_pos = strstr(input, ">>");
	if (redir_pos) {
	    append = 1;
	} else {
	    redir_pos = strstr(input, ">");
	}

	if (redir_pos) {
	    // 記錄重導向符號的位置
	    char *redir_start = redir_pos;

	    // 移到 '>' 後面
	    while (*redir_pos == '>') redir_pos++;
	    while (isspace((unsigned char)*redir_pos)) redir_pos++;

	    output_file = redir_pos;

	    // 清除命令部分（把原來的 '>' 改成 '\0'）
	    *redir_start = '\0';

	    // 去除 output_file 尾端空白
	    char *end = output_file + strlen(output_file) - 1;
	    while (end > output_file && isspace((unsigned char)*end)) {
		*end = '\0';
		end--;
	    }

	    // 去除 input 尾端空白
	    end = input + strlen(input) - 1;
	    while (end > input && isspace((unsigned char)*end)) {
		*end = '\0';
		end--;
	    }
	}

	
        // 分割命令參數
        int i = 0;
        char *token = strtok(input, " ");
        while (token != NULL && i < 63) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;
        if (!args[0]) { free(input); continue; }

        // 處理內建指令
        if (strcmp(args[0], "cd") == 0) {
            const char *dir = args[1] ? args[1] : getenv("HOME");
            if (chdir(dir) != 0)
                perror("cd");
        } else if (strcmp(args[0], "pwd") == 0) {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd))) printf("%s\n", cwd);
            else perror("pwd");
        } else if (strcmp(args[0], "export") == 0 && args[1]) {
            char *eq = strchr(args[1], '=');
            if (eq) {
                *eq = '\0';
                setenv(args[1], eq + 1, 1); // setenv 會複製字串
            } else {
                fprintf(stderr, "export: invalid format, expected NAME=VALUE\n");
            }
        } else if (strcmp(args[0], "history") == 0) {
            HIST_ENTRY **hist_list = history_list();
            if (hist_list) {
                for (int j = 0; hist_list[j]; j++)
                    printf("%d  %s\n", j + history_base, hist_list[j]->line);
            }
        } else if (strcmp(args[0], "sbg") == 0) {
            list_background_processes();
        } else {
        
            pid_t pid = fork();
            if (pid == 0) {
                
                if (output_file) {
                    int fd = open(output_file, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
                    if (fd < 0) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                if (execvp(args[0], args) == -1) {
                    perror("exec");
                    exit(EXIT_FAILURE);
                }
            } else if (pid < 0) {
                perror("fork");
            } else {
                if (background) {
                    printf("[Process running in background with PID %d]\n", pid);
                    add_background_process(pid, args[0]);
                } else {
                    waitpid(pid, NULL, 0);
                }
            }
        }
        free(input);
    }
    return 0;
}

