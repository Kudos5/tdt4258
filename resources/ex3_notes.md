## `file_operations`
- [Source](https://www.oreilly.com/library/view/linux-device-drivers/1565922921/ch03s03.html)

## `platform_driver`
- `probe()` happens when a device is registered and its device name matches the
  driver name (at least true for platform devices).
  + [Source 1](https://stackoverflow.com/questions/7578582/who-calls-the-probe-of-driver)
  + [Source 2](http://comments.gmane.org/gmane.linux.kernel.kernelnewbies/37050)
  + [Source 3](https://www.mjmwired.net/kernel/Documentation/driver-model/platform.txt)


                                                                                          
An incomplete attempt at visualizing `driver-gamepad.c`:
```                                                                                       
                                                             +--------------------------+       
+--------------------------------+                           |    register_chr_dev()    |       
| struct file_operations gp_fops |                           +--------------------------+       
+--------------------------------+                           |                          |       
| .owner = THIS_MODULE           |                           |    cdev_init(gp_fops)    |       
| .open = gp_open()              |                           |                 ^        |       
| .release = gp_release()        |                           +-----------------|--------+       
| .unlocked_ioctl = gp_ioctl()   |                                             |    ^     
| .fasync = gp_fasync()          +---------------------------------------------+    |     
+--------------------------------+                                                  |
^(see p. 13 in Linux Dev. Drivers)                                                  |
                                                                                    |
                                                                                    |
+----------------------------------+              +---------------------+           |      
| struct platform_driver gp_driver |              |      gb_probe()     |           |
+----------------------------------+              +---------------------+           |
| .probe = gp_probe()   <-------------------------+                     |           |
| .remove = gp_remove()            |              | register_chr_dev() -------------+
| .driver = driver <-------------------------+    |                     |
|                                  |         |    +---------------------+
|                                  |         |       
+-------------------------------+--+         |                            
                                |            |     
                                |            |     
+-------------------------------|-----+    +-+-----------------------------+                               
|         template_init()       |     |    |   struct device_driver driver |                               
+-------------------------------|-----+    +-------------------------------+                               
|                               v     |    | .name = "gp"                  |                               
| platform_driver_register(gp_driver) |    | .owner = THIS_MODULE          |
|                                     |    | .of_match_table = gp_of_match |                               
+-------------------+-----------------+    +-------------------------------+                               
                    |
                    |
+-------------------v--------+            
| module_init(template_init) |
+----------------------------+
^ (Registers template_init() as the function to be called on init?)                            
```                                       



  
  
  
  
  
  
  
