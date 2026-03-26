# Linux Kernel LED Module

## Description
This is a Linux kernel module that allows the user to control the blinking behavior of the scroll lock, num lock, and caps lock LEDs through the /proc filesystem.

## Technologies Used
- Kernel C
- Makefile
- Linux Kernel Modules

## How to Run

### 1. Clone the Repository
```bash
git clone https://github.com/elsberrysilas/linux-led-kernel-module.git
```
### 2. Build the Kernel Module
```bash
make
```
### 3. Insert Module
```bash
sudo insmod linux_led_kernel_module.ko
```
Note: Secure Boot may need to be disabled to load this kernel module.

### 4. Interacting with the Module
Setting LED mask (0-7)
```bash
echo L1 | sudo tee /proc/linux_led_kernel_module
```
LED Mask Values

| Value | LEDs Enabled |
|------|-------------|
| 0 | None |
| 1 | Scroll Lock |
| 2 | Num Lock |
| 3 | Scroll Lock + Num Lock |
| 4 | Caps Lock |
| 5 | Caps Lock + Scroll Lock |
| 6 | Caps Lock + Num Lock |
| 7 | All |

Setting blink speed (0-9) (faster-slower)
```bash
echo D1 | sudo tee /proc/linux_led_kernel_module
```
### 6. Remove the Module
```bash
sudo rmmod linux_led_kernel_module
```
## What I Learned
- How to develop and load Linux kernel modules
- How to use the /proc filesystem
- Working with kernel timers

## Future Improvements
- Add supprot for more LEDs
-  Improve user input/output
