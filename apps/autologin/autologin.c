#include <unistd.h>
#include <stdio.h>
int main()
{
    execlp("login", "login", "-f", "root", 0);
    execlp("/bin/mount", "mount", "/dev/mmcblk0p1", "/mnt", NULL);
}