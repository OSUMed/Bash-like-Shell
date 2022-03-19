// Instructions on how to run code:

gcc -std=gnu99 -pthread -o smallsh smallsh.c
./smallsh

gcc -std=gnu99 -pthread -o smallsh dup2Simple.c
./smallsh

Test script(not included)
chmod +x ./p3testscrip
./p3testscript 2>&1