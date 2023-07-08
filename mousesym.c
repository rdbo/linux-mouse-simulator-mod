#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

MODULE_LICENSE("GPL");

#define DEVNAME "mousesym"

static struct input_dev *input_device;
static int major;

static int mouse_char_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int mouse_char_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t mouse_char_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
	return 0;
}

static ssize_t mouse_char_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
	/* Accept 1 byte at a time */
	if (count > 1)
		count = 1;

	/* Simulate left click */
	input_report_key(input_device, BTN_LEFT, 1);  // Press the left button
	input_sync(input_device);
	input_report_key(input_device, BTN_LEFT, 0);  // Release the left button
	input_sync(input_device);

	return count;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = mouse_char_open,
	.release = mouse_char_release,
	.read = mouse_char_read,
	.write = mouse_char_write,
};

static int __init mouse_simulator_init(void)
{
	int err;

	printk(KERN_INFO "Loading 'mousesym' module...\n");

	/* Register char dev */
	major = register_chrdev(0, DEVNAME, &fops);
	if (major < 0) {
		printk(KERN_ERR "Failed to register char dev: %d\n", major);
		return major;
	}
	printk(KERN_INFO "mousesym major: %d\n", major);
	printk(KERN_INFO "Use 'mknod /dev/%s c %d 0' to set up the device file\n", DEVNAME, major);

	/* Allocate a new input device */
	input_device = input_allocate_device();
	if (IS_ERR(input_device)) {
		printk(KERN_ERR "Failed to allocate input device\n");
		return PTR_ERR(input_device);
	}

	/* Set up input device */
	input_device->name = DEVNAME;
	input_device->id.bustype = BUS_VIRTUAL; /* Tell the kernel that this is a virtual device, not physical */
	input_device->id.vendor  = 0x0000;
	input_device->id.product = 0x0000;
	input_device->id.version = 0x0000;

	/* Set up events and keys */
	input_device->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
	input_device->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT) | BIT_MASK(BTN_RIGHT) | BIT_MASK(BTN_MIDDLE);
	input_device->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y) | BIT_MASK(REL_WHEEL);

	/* Register the input device */
	if ((err = input_register_device(input_device))) {
		printk(KERN_ERR "Failed to register input device\n");
		input_free_device(input_device);
		return err;
	}

	/* Simulate left click */
	input_report_key(input_device, BTN_LEFT, 1);  // Press the left button
	input_sync(input_device);
	input_report_key(input_device, BTN_LEFT, 0);  // Release the left button
	input_sync(input_device);
	
	printk(KERN_INFO "The 'mousesym' module was successfully loaded\n");

	return 0;
}

static void __exit mouse_simulator_exit(void)
{
	printk(KERN_INFO "Unloading 'mousesym' module...\n");
	
	/* Unregister and deallocate input device */
	input_unregister_device(input_device);
	input_free_device(input_device);

	/* Unregister char dev */
	unregister_chrdev(major, DEVNAME);

	printk(KERN_INFO "The 'mousesym' module was successfully unloaded\n");
}

module_init(mouse_simulator_init);
module_exit(mouse_simulator_exit);