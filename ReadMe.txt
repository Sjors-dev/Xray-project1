To get given code up and running some steps have to be taken first. Those steps depend whether you are running wsl or macos.


====================================================================================================================
IN CASE OF WSL:
====================================================================================================================
First install the USBIPD-WIN project, on windows.

When you enter "usbipd list" in power shell (run as admin) then you get output like:
Connected:
BUSID  VID:PID    DEVICE                                                        STATE
1-2    0000:0002  Unknown USB Device (Device Descriptor Request Failed)         Not shared
2-1    2341:0043  Arduino Uno (COM3)                                            Not Shared
2-2    04f2:b6bf  HP HD Camera, HP IR Camera                                    Not shared
2-7    06cb:00f0  Synaptics FS7605 Touch Fingerprint Sensor with PurePrint(TM)  Not shared
2-10   8087:0026  Intel(R) Wireless Bluetooth(R)                                Not shared
3-3    03f0:034a  USB Input Device                                              Not shared
3-4    03f0:0941  USB Input Device                                              Not shared
09:59 02/02/2026
When you enter "usbipd bind -b 2-1" in power shell you will see that the Arduino com port has become shared.
Now you can make that com port available in a running wsl via the following command "usbipd attach -a -w -b 2-1"

When you want to update arduino code, you must detach first via ctrl-c. After updating the code, attach again.





====================================================================================================================
IN CASE OF MACOS:
====================================================================================================================
To be done