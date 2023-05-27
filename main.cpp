#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

int f(int x) {
    if (x == 0) {
        return 0;
    }

    if (x < 0)
        return 0;
    return 10 * x;
}

int g(int x) {
    if (x == 0) {
        while (true);
    }
    if (x < 0)
        return -x;
    else
        return 3 * x;
}


int status1, status2;
int value;
bool res1, res2;


int main() {
    int x;

    std::cout << "Enter x: ";
    std::cin >> x;

    int fd1[2]; // Pipe 1: Parent to f Child
    int fd2[2]; // Pipe 2: f Child to g Child

    pipe(fd1);
    pipe(fd2);

    pid_t fpid, gpid;

    fpid = fork();

    if (fpid == 0) {
        // Child process for f()
        fflush(stdout);

        read(fd1[0], &value, sizeof(value));
        close(fd1[0]);

        res1 = f(value);
        int result = f(value);

        write(fd1[1], &result, sizeof(res1));
        close(fd1[1]);

        exit(0);
    } else {
        // write init x to fd[1]

        write(fd1[1], &x, sizeof(x));
        write(fd2[1], &x, sizeof(x));

        gpid = fork();
        if (gpid == 0) {
            // Child process for g()

            read(fd2[0], &value, sizeof(value));
            close(fd2[0]);

            res2 = g(value);
            int result = g(value);
            write(fd2[1], &result, sizeof(res2));
            close(fd2[1]);

            return 0;
        } else {
            int time = 1;
            bool firstComplete = false, secondComplete = false, userStop = false;
            int bool_res;
            while (true) {
                // блокує виконання батьківського процесу, поки дочірній не зваершиться
                // але якщо не завершаються (while true), він їх не чікає
                pid_t firstCheck = waitpid(fpid, &status1, WNOHANG);
                pid_t secondCheck = waitpid(gpid, &status2, WNOHANG);
                std::cout << "  First process with pid: " << firstCheck << "\n  Second process with pid: " << secondCheck
                          << std::endl << std::endl;
                if (firstCheck > 0) firstComplete = true;
                if (secondCheck > 0) secondComplete = true;

                if (firstComplete && secondComplete) { bool_res = 1; }
                else if ((!firstComplete) && (!secondComplete)) { bool_res = -1; }
                else { bool_res = 0; }

                if (firstComplete && secondComplete) break;

                if (time % 5 == 0) {
                    int choice;
                    if (!userStop) {
                        printf("Do you want to continue?\n "
                               "0 - Terminate\n "
                               "1 - Continue\n "
                               "2 - Continue without prompting further\n>> ");
                        std::cin >> choice;
                        if (choice == 0) {
                            kill(fpid, SIGKILL);
                            kill(gpid, SIGKILL);
                            break;
                        }
                    }
                    if (choice == 2)
                        userStop = true;
                }
                sleep(1);
                time++;
            }

            if (bool_res == -1) {
                printf("Result: Undefined\n");
            } else if (bool_res == 0) {
                printf("Result: %d\n", res1 && res2);
            } else if (bool_res == 1) {
                read(fd1[0], &res1, sizeof(res1));
                read(fd2[0], &res2, sizeof(res2));
                printf("f(x) = %d,  g(x) = %d\n", res1, res2);
                printf("Result: %d && %d == %d\n", res1, res2, res1 && res2);
            }
        }

    }

}
