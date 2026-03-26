#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/vt_kern.h>
#include <linux/console_struct.h>

MODULE_DESCRIPTION("linux_led_kernel_module");
MODULE_AUTHOR("Silas");
MODULE_LICENSE("GPL");

#define PROC_FILE_NAME "kdled" // Name of the file that will appear in /proc
#define RESTORE_LEDS   0xFF // Used to help the LED drivers when we remove the mod

static int g_led_mask = 7; // Stores what LEDs should blink
static int g_hz_div   = 5; // Stores the divisor, this is used to control how fast the LEDs blink.
static struct timer_list my_timer; // This will the timer that will make the lights blink
static struct tty_driver *my_driver; // Allows us to send LED commands
static char kbledstatus = 0; // Stores current LED status for KDSETLED
static struct proc_dir_entry *proc_entry; // Proc entry

// Returns the timer delay in jiffies, this calculates the
// delay before blinking again.
static unsigned long get_delay_jiffies(void)
{
    int d = g_hz_div;
    if (d <= 0) // Protects aginst division by 0
        d = 1;

    return max(1UL, HZ / (unsigned long)d);
};

// This is the timer callback function, every time
// the timer ends this will be called.
static void my_timer_func(struct timer_list *timers)
{
    int *pstatus = (int *)&kbledstatus;
    if (*pstatus == (g_led_mask & 7))
        *pstatus = 0;
    else
        *pstatus = (g_led_mask & 7);

    (my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, *pstatus);
    my_timer.expires = jiffies + get_delay_jiffies();
    add_timer(&my_timer);
};

// This function handles reading from /proc/kbled
// If I run "cat /proc/kbled" it will return my current LED mask and divisor
static ssize_t kbled_read(struct file *filp, char __user *ubuf, size_t count, loff_t *offp)
{
    char buf[64];
    int len;
    len = snprintf(buf, sizeof(buf), "L%d D%d\n", (g_led_mask & 7), g_hz_div);
    return simple_read_from_buffer(ubuf, count, offp, buf, len);
};

// Handles writing to /proc/kbled
static ssize_t kbled_write(struct file *filp, const char __user *ubuf, size_t count, loff_t *offp)
{
    char kbuf[3];

    if (count < 2)
        return -EINVAL;

    if (copy_from_user(kbuf, ubuf, 2))
        return -EFAULT;

    kbuf[2] = '\0';

    if (kbuf[0] == 'L') 
    {
        if (kbuf[1] < '0' || kbuf[1] > '7')
            return -EINVAL;

        g_led_mask = kbuf[1] - '0';
        printk(KERN_INFO "kbled: set LED mask to %d\n", g_led_mask);
    }
    else if (kbuf[0] == 'D') 
    {
        if (kbuf[1] < '0' || kbuf[1] > '9')
            return -EINVAL;

        g_hz_div = kbuf[1] - '0';
        printk(KERN_INFO "kbled: set HZ divisor to %d\n", g_hz_div);
    }
    else 
    {
        return -EINVAL;
    }

    return count;
};

// Connects /proc file operations to the functions created
static const struct proc_ops proc_fops = 
{
    .proc_read  = kbled_read,
    .proc_write = kbled_write,
};

// Initialization function, runs when you load the mod with insmod
static int __init kbleds_init(void)
{
    my_driver = vc_cons[fg_console].d->port.tty->driver;

    // Creates /proc/kbled
    proc_entry = proc_create(PROC_FILE_NAME, 0666, NULL, &proc_fops);
    if (!proc_entry) 
    {
        printk(KERN_ERR "kbled: failed to create /proc/%s\n", PROC_FILE_NAME);
        return -ENOMEM;
    }

    // Starts timer
    timer_setup(&my_timer, my_timer_func, 0);
    my_timer.expires = jiffies + get_delay_jiffies();
    add_timer(&my_timer);

    // Turns on selected LEDs
    (my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, (g_led_mask & 7));

    return 0;
};

// Clean up function, runs when you use rmmod
static void __exit kbleds_cleanup(void)
{
    timer_shutdown_sync(&my_timer);

    // Restore the normal keyboard LEDs
    (my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);

    if (proc_entry)
        remove_proc_entry(PROC_FILE_NAME, NULL);
};

module_init(kbleds_init);
module_exit(kbleds_cleanup);