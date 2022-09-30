# 2021-22 - Processes and threads

Develop a C or C++ program on a POSIX compliant Operating System (eg Linux) that:

    It includes a global integer variable number with some initial value, and a function change_number(int x) which can and does continuously increment (or decrement) the variable number by a step x given as an argument by the user.
    The program upon execution will create a child process, a copy of itself, using the fork() system call. Therefore each process (parent and child) will have its own copy of the number variable.
     The child process will in turn spawn two threads, each of which will execute change_number() , within the child process, in a different way:

    The 1st thread will increment the value of the child process number variable by 5.
    The 2nd thread will decrement the value of the child process variable number by one random step each time, between 1 and 10.
    Each thread will run for a time that is randomly chosen during its creation, between 5 and 10 sec. Before terminating each thread, it displays its attributes on the screen.
