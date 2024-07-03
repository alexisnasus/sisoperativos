#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <ncurses.h>

#define MAX_COMMAND_LENGTH 256
#define MAX_PATH_LENGTH 1024
#define MAX_HISTORY 200

typedef struct CommandHistory {
    char commands[MAX_HISTORY][MAX_COMMAND_LENGTH];
    int count;
    int current;
} CommandHistory;

CommandHistory history;

void add_to_history(const char* command) {
    if (history.count < MAX_HISTORY) {
        strcpy(history.commands[history.count], command);
        history.count++;
    } else {
        for (int i = 1; i < MAX_HISTORY; i++) {
            strcpy(history.commands[i - 1], history.commands[i]);
        }
        strcpy(history.commands[MAX_HISTORY - 1], command);
    }
    history.current = history.count;
}

void show_history() {
    for (int i = 0; i < history.count; i++) {
        printw("%d: %s\n", i + 1, history.commands[i]);
    }
}

void execute_from_history(int index) {
    if (index > 0 && index <= history.count) {
        printw("Executing: %s\n", history.commands[index - 1]);
        refresh();
        system(history.commands[index - 1]);
    } else {
        printw("Invalid history index\n");
    }
}

void create_file(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file) {
        printw("File %s created\n", filename);
        fclose(file);
    } else {
        printw("Error creating file: %s\n", strerror(errno));
    }
}

void create_directory(const char* dirname) {
    if (mkdir(dirname, 0755) == 0) {
        printw("Directory %s created\n", dirname);
    } else {
        printw("Error creating directory: %s\n", strerror(errno));
    }
}

void rename_file_or_directory(const char* oldname, const char* newname) {
    if (rename(oldname, newname) == 0) {
        printw("%s renamed to %s\n", oldname, newname);
    } else {
        printw("Error renaming: %s\n", strerror(errno));
    }
}

void delete_directory_recursive(const char* path) {
    DIR* dir = opendir(path);
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char filepath[MAX_PATH_LENGTH];
                snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
                struct stat st;
                if (stat(filepath, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        delete_directory_recursive(filepath);
                    } else {
                        if (remove(filepath) != 0) {
                            printw("Error deleting file: %s\n", strerror(errno));
                        }
                    }
                }
            }
        }
        closedir(dir);
        if (rmdir(path) != 0) {
            printw("Error deleting directory: %s\n", strerror(errno));
        } else {
            printw("Directory %s deleted\n", path);
        }
    } else {
        printw("Error opening directory: %s\n", strerror(errno));
    }
}

void delete_file_or_directory(const char* name) {
    struct stat st;
    if (stat(name, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            delete_directory_recursive(name);
        } else {
            if (remove(name) == 0) {
                printw("File %s deleted\n", name);
            } else {
                printw("Error deleting file: %s\n", strerror(errno));
            }
        }
    } else {
        printw("Error finding file or directory: %s\n", strerror(errno));
    }
}

void print_permissions(mode_t mode) {
    printw((S_ISDIR(mode)) ? "d" : "-");
    printw((mode & S_IRUSR) ? "r" : "-");
    printw((mode & S_IWUSR) ? "w" : "-");
    printw((mode & S_IXUSR) ? "x" : "-");
    printw((mode & S_IRGRP) ? "r" : "-");
    printw((mode & S_IWGRP) ? "w" : "-");
    printw((mode & S_IXGRP) ? "x" : "-");
    printw((mode & S_IROTH) ? "r" : "-");
    printw((mode & S_IWOTH) ? "w" : "-");
    printw((mode & S_IXOTH) ? "x" : "-");
}

void list_files(const char* path) {
    DIR* dir = opendir(path);
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                struct stat st;
                char filepath[MAX_PATH_LENGTH];
                snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
                if (stat(filepath, &st) == 0) {
                    attron(COLOR_PAIR((S_ISDIR(st.st_mode)) ? 2 : 1));
                    printw("%s\n", entry->d_name);
                    attroff(COLOR_PAIR((S_ISDIR(st.st_mode)) ? 2 : 1));
                } else {
                    printw("Error getting file status: %s\n", strerror(errno));
                }
            }
        }
        closedir(dir);
    } else {
        printw("Error opening directory: %s\n", strerror(errno));
    }
}

void list_files_detailed(const char* path) {
    DIR* dir = opendir(path);
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                struct stat st;
                char filepath[MAX_PATH_LENGTH];
                snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
                if (stat(filepath, &st) == 0) {
                    printw("%lu ", st.st_ino); // Inode number
                    print_permissions(st.st_mode); // File permissions
                    printw(" %ld ", st.st_nlink); // Number of hard links

                    struct passwd *pw = getpwuid(st.st_uid);
                    struct group *gr = getgrgid(st.st_gid);
                    printw("%s %s ", pw->pw_name, gr->gr_name); // User and group name
                    printw("%ld ", st.st_size); // File size

                    char timebuf[80];
                    struct tm *timeinfo = localtime(&st.st_mtime);
                    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", timeinfo);
                    printw("%s ", timebuf); // Last modification time

                    attron(COLOR_PAIR((S_ISDIR(st.st_mode)) ? 2 : 1));
                    printw("%s\n", entry->d_name); // File or directory name
                    attroff(COLOR_PAIR((S_ISDIR(st.st_mode)) ? 2 : 1));
                } else {
                    printw("Error getting file status: %s\n", strerror(errno));
                }
            }
        }
        closedir(dir);
    } else {
        printw("Error opening directory: %s\n", strerror(errno));
    }
}

void list_files_recursively_detailed(const char* basepath, int indent) {
    DIR* dir = opendir(basepath);
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                for (int i = 0; i < indent; i++) {
                    printw("  ");
                }
                struct stat st;
                char path[MAX_PATH_LENGTH];
                snprintf(path, sizeof(path), "%s/%s", basepath, entry->d_name);
                if (stat(path, &st) == 0) {
                    printw("%lu ", st.st_ino); // Inode number
                    print_permissions(st.st_mode); // File permissions
                    printw(" %ld ", st.st_nlink); // Number of hard links

                    struct passwd *pw = getpwuid(st.st_uid);
                    struct group *gr = getgrgid(st.st_gid);
                    printw("%s %s ", pw->pw_name, gr->gr_name); // User and group name
                    printw("%ld ", st.st_size); // File size

                    char timebuf[80];
                    struct tm *timeinfo = localtime(&st.st_mtime);
                    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", timeinfo);
                    printw("%s ", timebuf); // Last modification time

                    attron(COLOR_PAIR((S_ISDIR(st.st_mode)) ? 2 : 1));
                    printw("%s\n", entry->d_name); // File or directory name
                    attroff(COLOR_PAIR((S_ISDIR(st.st_mode)) ? 2 : 1));

                    if (S_ISDIR(st.st_mode)) {
                        list_files_recursively_detailed(path, indent + 1);
                    }
                } else {
                    printw("Error getting file status: %s\n", strerror(errno));
                }
            }
        }
        closedir(dir);
    } else {
        printw("Error opening directory: %s\n", strerror(errno));
    }
}

void change_permissions(const char* path, mode_t mode) {
    if (chmod(path, mode) == 0) {
        printw("Permissions for %s changed\n", path);
    } else {
        printw("Error changing permissions: %s\n", strerror(errno));
    }
}

void search_files(const char* path, const char* name) {
    DIR* dir = opendir(path);
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strstr(entry->d_name, name) != NULL) {
                printw("Found: %s/%s\n", path, entry->d_name);
            }
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char newpath[MAX_PATH_LENGTH];
                snprintf(newpath, sizeof(newpath), "%s/%s", path, entry->d_name);
                search_files(newpath, name);
            }
        }
        closedir(dir);
    } else {
        printw("Error opening directory: %s\n", strerror(errno));
    }
}

void change_directory(const char* path) {
    if (chdir(path) == 0) {
        char cwd[MAX_PATH_LENGTH];
        getcwd(cwd, sizeof(cwd));
        printw("Changed directory to %s\n", cwd);
    } else {
        printw("Error changing directory: %s\n", strerror(errno));
    }
}

void print_working_directory() {
    char cwd[MAX_PATH_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printw("Current directory: %s\n", cwd);
    } else {
        printw("Error getting current directory: %s\n", strerror(errno));
    }
}

void show_help() {
    printw("Available commands:\n");
    printw("help - Show this help message\n");
    printw("exit - Exit the program\n");
    printw("touch <filename> - Create a new file\n");
    printw("mkdir <dirname> - Create a new directory\n");
    printw("rename <oldname> <newname> - Rename a file or directory\n");
    printw("rm <name> - Remove a file or directory and its contents\n");
    printw("chmod <path> <mode> - Change permissions of a file or directory\n");
    printw("ls - List files in the current directory\n");
    printw("ls -R - List files in the current directory and subdirectories with detailed information\n");
    printw("ls -l - List files in the current directory with detailed information\n");
    printw("cd <path> - Change directory\n");
    printw("pwd - Print working directory\n");
    printw("search <name> - Search for files and directories by name\n");
    printw("history - Show command history\n");
}

void clear_command_line(int row, const char* cwd) {
    move(row, 0); 
    clrtoeol();   
    attron(COLOR_PAIR(1)); 
    printw("%s> ", cwd); 
    attroff(COLOR_PAIR(1)); 
    refresh();
}

void clear_command_output() {
    for (int i = 1; i < LINES - 2; i++) {
        move(i, 0);
        clrtoeol();
    }
    move(1, 0);
}

void handle_command(char* command) {
    add_to_history(command);

    clear_command_output(); // Clear the output area before executing a new command

    if (strcmp(command, "exit") == 0) {
        endwin();
        exit(0);
    } else if (strcmp(command, "help") == 0) {
        show_help();
    } else if (strncmp(command, "touch ", 6) == 0) {
        create_file(command + 6);
    } else if (strncmp(command, "mkdir ", 6) == 0) {
        create_directory(command + 6);
    } else if (strncmp(command, "rename ", 7) == 0) {
        char* oldname = strtok(command + 7, " ");
        char* newname = strtok(NULL, " ");
        if (oldname && newname) {
            rename_file_or_directory(oldname, newname);
        } else {
            printw("Usage: rename <oldname> <newname>\n");
        }
    } else if (strncmp(command, "rm ", 3) == 0) {
        delete_file_or_directory(command + 3);
    } else if (strncmp(command, "chmod ", 6) == 0) {
        char* path = strtok(command + 6, " ");
        char* mode_str = strtok(NULL, " ");
        if (path && mode_str) {
            mode_t mode = strtol(mode_str, NULL, 8);
            change_permissions(path, mode);
        } else {
            printw("Usage: chmod <path> <mode>\n");
        }
    } else if (strcmp(command, "ls") == 0) {
        list_files(".");
    } else if (strcmp(command, "ls -R") == 0) {
        list_files_recursively_detailed(".", 0);
    } else if (strcmp(command, "ls -l") == 0) {
        list_files_detailed(".");
    } else if (strncmp(command, "search ", 7) == 0) {
        search_files(".", command + 7);
    } else if (strcmp(command, "history") == 0) {
        show_history();
    } else if (strncmp(command, "cd ", 3) == 0) {
        change_directory(command + 3);
    } else if (strcmp(command, "pwd") == 0) {
        print_working_directory();
    } else {
        printw("Unknown command: %s\n", command);
    }
    char cwd[MAX_PATH_LENGTH];
    getcwd(cwd, sizeof(cwd));
    move(LINES - 1, 0); // Move to the bottom of the screen for input prompt
    clear_command_line(LINES - 1, cwd);
}

int main() {
    initscr();
    start_color();
    use_default_colors();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    init_pair(1, COLOR_GREEN, -1); 
    init_pair(2, COLOR_BLUE, -1); 
    init_pair(3, COLOR_CYAN, -1); 

    char command[MAX_COMMAND_LENGTH];
    int i = 0;
    int ch;
    int input_row = LINES - 1; 

    char cwd[MAX_PATH_LENGTH];
    getcwd(cwd, sizeof(cwd));
    printw("Shell interactiva con historial de comandos. Usa las flechas arriba/abajo para navegar por el historial:\n");
    refresh();
    move(input_row, 0);
    attron(COLOR_PAIR(1));
    printw("%s> ", cwd);
    attroff(COLOR_PAIR(1));
    refresh();

    while (1) {
        ch = getch();
        if (ch == '\n') {
            command[i] = '\0';

            if (i > 0) {
                handle_command(command);
                clear_command_line(input_row, cwd);
            }
            i = 0;
            memset(command, 0, sizeof(command));
            getcwd(cwd, sizeof(cwd));
            move(input_row, 0);
            clear_command_line(input_row, cwd);
        } else if (ch == KEY_UP) {
            if (history.current > 0) {
                history.current--;
                strcpy(command, history.commands[history.current]);
                i = strlen(command);
                clear_command_line(input_row, cwd);
                attron(COLOR_PAIR(2));
                printw("%s", command);
                attroff(COLOR_PAIR(2));
                refresh();
            }
        } else if (ch == KEY_DOWN) {
            if (history.current < history.count - 1) {
                history.current++;
                strcpy(command, history.commands[history.current]);
                i = strlen(command);
                clear_command_line(input_row, cwd);
                attron(COLOR_PAIR(2));
                printw("%s", command);
                attroff(COLOR_PAIR(2));
                refresh();
            } else {
                history.current = history.count;
                i = 0;
                memset(command, 0, sizeof(command));
                clear_command_line(input_row, cwd);
            }
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (i > 0) {
                i--;
                command[i] = '\0';
                clear_command_line(input_row, cwd);
                attron(COLOR_PAIR(2));
                printw("%s", command);
                attroff(COLOR_PAIR(2));
                refresh();
            }
        } else {
            if (i < MAX_COMMAND_LENGTH - 1) {
                command[i++] = ch;
                attron(COLOR_PAIR(2));
                addch(ch);
                attroff(COLOR_PAIR(2));
                refresh();
            }
        }
    }

    endwin();
    return 0;
}

