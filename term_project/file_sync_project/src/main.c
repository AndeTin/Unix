#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
#define DEFAULT_LOCAL_BACKUP "backup/"
#define DEFAULT_SSH_PORT "22"

// Struct to hold remote sync information
typedef struct {
    const char *local;
    const char *backup;
    const char *remote_user;
    const char *remote_ip;
    const char *remote;
    int auth_mode;         // 1 for password-based, 2 for SSH key-based
    char password[256];    // Store password for password-based authentication
    char port[6];          // Port number for SSH connection
} sync_info_t;

// Global variables for time tracking
struct timespec last_event_time;
pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for time synchronization

// Timer thread to check elapsed time and sync if needed
void *timer_thread(void *args) {
    sync_info_t *info = (sync_info_t *)args;
    while (1) {
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        // Calculate elapsed time
        double elapsed_time = (current_time.tv_sec - last_event_time.tv_sec) +
                              (current_time.tv_nsec - last_event_time.tv_nsec) / 1e9;

        if (elapsed_time >= 10) {
            printf("No changes detected for 10 seconds. Syncing remote to local and backup...\n");
            pthread_mutex_lock(&time_mutex); // Lock time to avoid race conditions
            if (info->auth_mode == 1) {
                char command[1024];
                snprintf(command, sizeof(command),
                         "sshpass -p '%s' rsync -av -e 'ssh -p %s' --delete %s@%s:%s/ %s/",
                         info->password, info->port, info->remote_user, info->remote_ip, info->remote, info->local);
                system(command);

                snprintf(command, sizeof(command),
                         "sshpass -p '%s' rsync -av -e 'ssh -p %s' --delete %s@%s:%s/ %s/",
                         info->password, info->port, info->remote_user, info->remote_ip, info->remote, info->backup);
                system(command);
            } else {
                char command[1024];
                snprintf(command, sizeof(command),
                         "rsync -av -e ssh --delete %s@%s:%s/ %s/",
                         info->remote_user, info->remote_ip, info->remote, info->local);
                system(command);

                snprintf(command, sizeof(command),
                         "rsync -av -e ssh --delete %s@%s:%s/ %s/",
                         info->remote_user, info->remote_ip, info->remote, info->backup);
                system(command);
            }
            clock_gettime(CLOCK_MONOTONIC, &last_event_time); // Reset last_event_time
            pthread_mutex_unlock(&time_mutex);
        }

        usleep(100000); // Sleep for 100ms to reduce CPU usage
    }
}
// Function to sync local to backup
void sync_local_backup(const char *local, const char *backup) {
    char command[1024];
    snprintf(command, sizeof(command), "rsync -av --delete %s/ %s/", local, backup);
    system(command);
    printf("Synced local (%s) to backup (%s)\n", local, backup);
}

// Function to sync backup to local
void sync_backup_local(const char *backup, const char *local) {
    char command[1024];
    snprintf(command, sizeof(command), "rsync -av --delete %s/ %s/", backup, local);
    system(command);
    printf("Synced backup (%s) to local (%s)\n", backup, local);
}

// Function to sync local to remote
void sync_local_remote(const char *local, const char *remote_user, const char *remote_ip, const char *remote) {
    char command[1024];
    snprintf(command, sizeof(command), "rsync -av -e ssh --delete %s/ %s@%s:%s/", local, remote_user, remote_ip, remote);
    system(command);
    printf("Synced local (%s) to remote (%s@%s:%s)\n", local, remote_user, remote_ip, remote);
}

// Function to sync remote to local
void sync_remote_local(const char *local, const char *remote_user, const char *remote_ip, const char *remote) {
    char command[1024];
    snprintf(command, sizeof(command), "rsync -av -e ssh --delete %s@%s:%s/ %s/", remote_user, remote_ip, remote, local); 
    system(command);
    printf("Synced remote (%s@%s:%s) to local (%s)\n", remote_user, remote_ip, remote, local);
}
// Function to sync remote to backup
void sync_remote_backup(const char *backup, const char *remote_user, const char *remote_ip, const char *remote) {
    char command[1024];
    snprintf(command, sizeof(command), "rsync -av -e ssh --delete %s@%s:%s/ %s/", remote_user, remote_ip, remote, backup); 
    system(command);
    printf("Synced remote (%s@%s:%s) to local (%s)\n", remote_user, remote_ip, remote, backup);
}
// Function to sync backup to remote
void sync_backup_remote(const char *backup, const char *remote_user, const char *remote_ip, const char *remote) {
    char command[1024];
    snprintf(command, sizeof(command), "rsync -av -e ssh --delete %s/ %s@%s:%s/", backup, remote_user, remote_ip, remote);
    system(command);
    printf("Synced backup (%s) to remote (%s@%s:%s)\n", backup, remote_user, remote_ip, remote);
}

// Handle local directory changes
void handle_local_change(const char *local, const char *backup, const char *remote_user, const char *remote_ip, const char *remote) {
    sync_local_backup(local, backup);
    sync_local_remote(local, remote_user, remote_ip, remote);
}

// Handle backup directory changes
void handle_backup_change(const char *local, const char *backup, const char *remote_user, const char *remote_ip, const char *remote) {
    sync_backup_local(backup, local);
    sync_backup_remote(backup, remote_user, remote_ip, remote);
}

// Handle remote directory synchronization
void handle_remote_sync(const char *local, const char *backup, const char *remote_user, const char *remote_ip, const char *remote) {
    sync_remote_local(local, remote_user, remote_ip, remote);
    sync_remote_backup(backup, remote_user, remote_ip, remote);
}


int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <source_directory> <remote_user> <remote_ip> <remote_directory>\n", argv[0]);
        return 1;
    }

    const char *local = argv[1];
    const char *backup = DEFAULT_LOCAL_BACKUP;
    const char *remote_user = argv[2];
    const char *remote_ip = argv[3];
    const char *remote = argv[4];

    // Prompt user for authentication mode
    int auth_mode;
    printf("Select authentication mode:\n");
    printf("1. Password-based authentication\n");
    printf("2. SSH key-based authentication (default)\n");
    printf("Enter choice (1 or 2): ");
    scanf("%d", &auth_mode);

    char password[256] = {0};
    char port[6] = DEFAULT_SSH_PORT; // Default to port 22

    if (auth_mode == 1) {
        printf("Enter password for %s@%s: ", remote_user, remote_ip);
        scanf("%s", password);

        printf("Enter port number (press Enter for default 22): ");
        getchar(); // Clear newline from previous input
        fgets(port, sizeof(port), stdin);
        port[strcspn(port, "\n")] = 0; // Remove newline character
        if (strlen(port) == 0) {
            strncpy(port, DEFAULT_SSH_PORT, sizeof(port) - 1);
        }
    }

    // Initialize sync_info struct
    sync_info_t sync_info = {
        .local = local,
        .backup = backup,
        .remote_user = remote_user,
        .remote_ip = remote_ip,
        .remote = remote,
        .auth_mode = auth_mode,
    };
    if (auth_mode == 1) {
        strncpy(sync_info.password, password, sizeof(sync_info.password) - 1);
        strncpy(sync_info.port, port, sizeof(sync_info.port) - 1);
    }

    // Initialize last_event_time
    clock_gettime(CLOCK_MONOTONIC, &last_event_time);

    // Ensure backup directory exists
    char command[256];
    snprintf(command, sizeof(command), "mkdir -p %s", backup);
    system(command);

    // Start the timer thread
    pthread_t timer_tid;
    pthread_create(&timer_tid, NULL, timer_thread, (void *)&sync_info);

    int fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        return 1;
    }

    int wd_local = inotify_add_watch(fd, local, IN_CREATE | IN_MODIFY | IN_DELETE);
    int wd_backup = inotify_add_watch(fd, backup, IN_CREATE | IN_MODIFY | IN_DELETE);

    if (wd_local < 0 || wd_backup < 0) {
        perror("inotify_add_watch");
        close(fd);
        return 1;
    }

    printf("Monitoring local (%s) and backup (%s) directories for changes...\n", local, backup);

    char buffer[EVENT_BUF_LEN];
    while (1) {
        int length = read(fd, buffer, EVENT_BUF_LEN);
        if (length < 0 && errno != EAGAIN) {
            perror("read");
            break;
        }

        int i = 0;
        int local_changed = 0, backup_changed = 0;

        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len) {
                if (event->wd == wd_local) {
                    printf("Change detected in local: %s\n", event->name);
                    handle_local_change(local, backup, remote_user, remote_ip, remote);
                    local_changed = 1;
                } else if (event->wd == wd_backup) {
                    printf("Change detected in backup: %s\n", event->name);
                    handle_backup_change(local, backup, remote_user, remote_ip, remote);
                    backup_changed = 1;
                }
            }
            i += EVENT_SIZE + event->len;
        }

        if (local_changed || backup_changed) {
            pthread_mutex_lock(&time_mutex); // Lock time to avoid race conditions
            clock_gettime(CLOCK_MONOTONIC, &last_event_time); // Reset last_event_time
            pthread_mutex_unlock(&time_mutex);
        }

        usleep(100000); // Sleep for 100ms to reduce CPU usage
    }

    inotify_rm_watch(fd, wd_local);
    inotify_rm_watch(fd, wd_backup);
    close(fd);

    return 0;
}

