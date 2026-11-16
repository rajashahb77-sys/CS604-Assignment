#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int read_positive(const char *prompt)
{
    int value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1 && value > 0)
            return value;
        printf("Invalid input. Enter a positive integer.\n");
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
    }
}

int main(void)
{
    char student_id[64];
    int times[3];
    int p2c[2];
    int c2p[2];

    printf("Enter Student ID: ");
    scanf("%63s", student_id);
    times[0] = read_positive("Enter Task 1 Execution Time: ");
    times[1] = read_positive("Enter Task 2 Execution Time: ");
    times[2] = read_positive("Enter Task 3 Execution Time: ");

    if (pipe(p2c) == -1 || pipe(c2p) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        int recv[3], result[3];
        close(p2c[1]);
        close(c2p[0]);
        read(p2c[0], recv, sizeof(recv));
        int total = 0, longest = 1, max = recv[0];
        for (int i = 0; i < 3; i++) {
            total += recv[i];
            if (recv[i] > max) {
                max = recv[i];
                longest = i + 1;
            }
        }
        result[0] = total;
        result[1] = longest;
        result[2] = max;
        write(c2p[1], result, sizeof(result));
        close(p2c[0]);
        close(c2p[1]);
        exit(0);
    }

    int result[3];
    close(p2c[0]);
    close(c2p[1]);
    write(p2c[1], times, sizeof(times));
    close(p2c[1]);
    waitpid(pid, NULL, 0);
    read(c2p[0], result, sizeof(result));
    close(c2p[0]);

    char filename[128];
    snprintf(filename, sizeof(filename), "%s_report.txt", student_id);

    char report[512];
    snprintf(report, sizeof(report),
             "--- Task Processing Report ---\n"
             "Student ID: %s\n"
             "Task 1: %d units\n"
             "Task 2: %d units\n"
             "Task 3: %d units\n"
             "Total Execution Time: %d units\n"
             "Longest Task: Task %d\n"
             "Maximum Execution Time: %d units\n",
             student_id, times[0], times[1], times[2],
             result[0], result[1], result[2]);

    printf("\n%s", report);

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    write(fd, report, strlen(report));
    close(fd);

    printf("Report saved to: %s\n", filename);
    return 0;
}