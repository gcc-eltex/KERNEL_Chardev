#include "header/chdev.h"

#define DEVICE_NAME "my_chdev"
#define DEVICE_CLASS_NAME "my_class_chdev"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ivannikov Igor");
MODULE_DESCRIPTION("Testing char dev module");

static dev_t                    dev;        // Мажорный и минорный номера
static struct cdev              cdev;       // Зарегистрированное устройство
static struct class             *class;     // Класс устройств
static struct device            *device;    // Созданое устройство
static struct file_operations   fops = {    // Перегружаемые функции
    .read = device_read,
    .write = device_write,
};

/*
 * Инициализирует модуль. Запускается в момент загрузки в ядро
 * Возвращает 0 при успешном завершении, -1 при ошибке
 */
int init_module(void)
{
    // Получаем Свободный мажорный и первый минорный номера
    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0){
        printk(KERN_ERR "FAILED register_chrdev_region\n");
        return -1;
    }

    // Регистрируем устройство
    cdev_init(&cdev, &fops);
    if (cdev_add(&cdev, dev, 1) < 0){
        printk(KERN_ERR "FAILED cdev_add\n");
        return -1;
    }

    // Создаем класс устройств
    class = class_create(THIS_MODULE, DEVICE_CLASS_NAME);
    if (IS_ERR(class)){
        printk(KERN_ERR "FAILED class_create\n");
        cdev_del(&cdev);
        unregister_chrdev_region(dev, 1);
        return -1;
    }
    // Вот эта магия, чтобы выставить права
    class->devnode = device_devnode;

    // Создаем устройство
    device = device_create(class, NULL, dev, NULL, DEVICE_NAME);
    if (IS_ERR(device)){
        printk(KERN_ERR "FAILED device_create\n");
        class_destroy(class);
        cdev_del(&cdev);
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    printk(KERN_ALERT "Driver get major %d\n", MAJOR(dev));
    return 0;
     //Можно было использовать misc_register
}

/*
 * Финализация модуля. Удаляет все файлы устройств.  Запускается в момент
 * выгрузки из ядра.
 */
void cleanup_module(void)
{
    device_destroy(class, dev);
    class_destroy(class);
    cdev_del(&cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_ALERT "Unregister for major number %d\n", MAJOR(dev));
}

/* 
 * device_read - вызывается при чтении из файла устройств пользователем
 * @filep: указатель на структуру, описывающую открываемый файл
 * @buffer: указатель на область возвращаемых данных
 * @len: количество считываемых байт
 * @offset: смещение, при чтении
 * 
 * Возвращает количество считанных байт при успешном завершении, а при 
 * ошибке -1
 */
static ssize_t device_read(struct file *filep, char *buffer, size_t len,
                           loff_t *offset)
{
    char send_msg[] = {"Hello, user!\0"}; // Сообщение, возвращаемое 
                                          // пользователю
    ssize_t rbyte = 0;                    // Текущий считываемый байт

    while (send_msg[rbyte] != '\0' && rbyte < len){
        if (put_user(send_msg[rbyte++], buffer++)){
            return -1;
        }
    }
    printk(KERN_ALERT "User read from file\n");
    return rbyte - 1;
}

/* 
 * device_write - вызывается при чтении из файла устройств пользователем
 * @filep: указатель на структуру, описывающую открываемый файл
 * @buffer: указатель на область возвращаемых данных
 * @len: количество считываемых байт
 * @offset: смещение, при чтении
 * 
 * Возвращает количество записанных байт при успешном завершении, а при 
 * ошибке -1
 */
static ssize_t device_write(struct file *filep, const char *buffer, size_t len,
                            loff_t *offset)
{
    char *recv_msg;    // Указатель на полученные от пользователя данные

    // Выделяем динамически память по размеру полученного сообщения
    recv_msg = kmalloc(len, GFP_KERNEL);
    if (!recv_msg){
        printk(KERN_ERR "FAILED kmalloc\n");
        return -1;
    }
    if (!strncpy_from_user(recv_msg, buffer, len)){
        printk(KERN_ERR "FAILED strncpy_from_user\n");
        return -1;
    }
    printk(KERN_ALERT "User write to file: %s\n", recv_msg);
    kfree(recv_msg);
    return 0;
}

/* 
 * Какая-то дичь! Нужна, чтобы выставить права на созданный файл символьного
 * устройства
 */
static char *device_devnode(struct device *dev, umode_t *mode)
{
        if (!mode)
                return NULL;
        *mode = 0666;
        return NULL;
}