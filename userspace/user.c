#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main()
{
    int  fd;                            // Дескриптор файла устройства
    char recv_msg[64];                  // Получаемое сообщение
    char send_msg[] = "Hello, kernel!"; // Отправляемое сообщение

    fd = open("/dev/my_chdev", O_RDWR); 
    if (fd < 0){
        perror("open /dev/my_chdev");
        return -1;
    }

    // Считываем сообщение из пространства ядра
    if (read(fd, recv_msg, 64) == -1){
        printf("ERROR: read from /dev/my_chdev");
        return -1;
    }
    printf("Read from kernel: %s\n", recv_msg);

    // Спускаем сообщение в пространство ядра
    if (write(fd, send_msg, strlen(send_msg) + 1) == -1){
        printf("ERROR: write to /dev/my_chdev");
        return -1;
    }
    close(fd);
    return 0;
}