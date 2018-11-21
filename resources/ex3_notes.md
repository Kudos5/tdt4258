Exercise 3 - Gamepad driver and game
====================================

`game.c` - walkthrough
----------------------

### `main()`
- Open the file `/dev/fb0` (file descriptor `fb_fd`) with RDWR access mode. 
  This file represents our frame buffer.
- Map the file descriptor to a memory location. This lets us write pixels by
  writing directly to a C array. `mmap()` returns the address of the new 
  (mapping) array, so that we can simply operate on that array to set the screen
  contents.
  + When we have mapped memory to the frame buffer file, all we need to do to
    change screen contents is
    1. Write to the mapping array (`game_screen`)
    2. Update the area that we want to change:
        `ioctl(fb_fd, 0x4680, &rect_area)`
       rect_area` here is of type `struct fb_copyarea`, which is a pretty
       straight-forward format
- Call setup functions for setting up the gamepad, snake etc.
- Enter main loop that sleeps until woken by button or timer signal
- Close files (for frame buffer and game pad)

### `setup_gamepad()`
- Subscribes to signals from the gamepad, i.e. if our (game) process receives a
  `SIGIO` signal, it runs the `input_handler` function. Uses `fcntl` to...
  1. Set the PID of the process (this process in this case) that will receive 
     `SIGIO` signals for events on the file descriptor (`gp_fd`). 
     (Set file descriptor owner)
  2. Get the file access mode and file status flags (`oflags`)
  3. Set the flags of the gamepad file to `oflags | FASYNC`. 
    When `FASYNC` (which is a synonym for `O_ASYNC`) is set, the process that
    owns `gp_fd` will receive `SIGIO` when input is available on the file
    descriptor.
- When `SIGIO` received from gamepad driver: `input_handler` gets the
  `game_button_state` and sets the `flag_button_pressed = 1` to tell `main()`
  that it should act.
- In `main()`, we call a function that sets player directions according to which
  button was pressed, i.e. we have to bitmask for individual buttons.


`driver-gamepad.c` 
----------------
- Sets the functions to be called on module load and module exit
  (`module_init(<init_func>)` and `module_exit(<exit_func>)`
- `template_init` (the `<init_func>`) registers `gp_driver` as a
  `platform_driver`. `gp_driver` is a `platform_driver` struct 
  
An incomplete attempt at visualizing `driver-gamepad.c`:

```                                                                                       
                                                             +--------------------------+       
+--------------------------------+                           |    register_chr_dev()    |       
| struct file_operations gp_fops |                           +--------------------------+       
+--------------------------------+                           |                          |       
| .owner = THIS_MODULE           |                           |    cdev_init(gp_fops)    |       
| .open = gp_open                |                           |                 ^        |       
| .release = gp_release          |                           +-----------------|--------+       
| .unlocked_ioctl = gp_ioctl     |                                             |    ^     
| .fasync = gp_fasync            +---------------------------------------------+    |     
+--------------------------------+                                                  |
^(see p. 13 in Linux Dev. Drivers)                                                  |
                                                                                    |
                                                                                    |
+----------------------------------+              +---------------------+           |      
| struct platform_driver gp_driver |              |      gb_probe()     |           |
+----------------------------------+              +---------------------+           |
| .probe = gp_probe   <---------------------------+                     |           |
| .remove = gp_remove <-------------------------+ | register_chr_dev() -------------+
| .driver = driver    <----------------------+  | |                     |
|                                  |         |  | +---------------------+
+-------------------------------+--+         |  | +---------------------+ 
                                |            |  +-+     gb_remove()     |
                                |            |    +---------------------+
+-------------------------------|-----+    +-+-----------------------------+                               
|               gp_init()       |     |    |   struct device_driver driver |                               
+-------------------------------|-----+    +-------------------------------+                               
|                               v     |    | .name = "gp"                  |                               
| platform_driver_register(gp_driver) |    | .owner = THIS_MODULE          |
|                                     |    | .of_match_table = gp_of_match |                               
+-------------------+-----------------+    +-------------------------------+                               
                    |
                    |
+-------------------v--------+            
|    module_init(gp_init)    |
+----------------------------+
^ (Registers template_init() as the function to be called on init?)                            
```                                       

### Structs
#### `file_operations`
- [Source](https://www.oreilly.com/library/view/linux-device-drivers/1565922921/ch03s03.html)

#### `platform_driver`
- Contains function pointers to some functions that will be called by the system:
  + `gp_probe()`: Called when a device is registered and its device name matches 
    the driver name (at least true for platform devices).
  + `gp_remove()` ... when device is removed, I assume...
- Sources
  + [Source 1](https://stackoverflow.com/questions/7578582/who-calls-the-probe-of-driver)
  + [Source 2](http://comments.gmane.org/gmane.linux.kernel.kernelnewbies/37050)
  + [Source 3](https://www.mjmwired.net/kernel/Documentation/driver-model/platform.txt)
    

Practical advice
----------------
- To install man section 2 (on Debian), run
    apt install manpages-dev
  
