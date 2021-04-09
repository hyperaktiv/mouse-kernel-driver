# Fake mouse driver control from terminal
### - Mouse kernel driver on Centos7
	alread run successfully test on Centos7
### - How to run? from `fakemouse` folder
1. `make`
2. `sudo insmod mousekernel.ko`
3. `dmesg` 	:to check and debug
4. `sudo mknod /dev/mousekernel c [MAJOR] 0`	: can be found in 'dmesg' debugging
5. `sudo chmod 666 /dev/mousekernel`
6. `echo "ll" > /dev/moousekernel`
	- step 6 to check the work of the driver with those instruction:
	##### echo "i [...]" > /dev/mousekernel
		-"i" is command
		-"l" for -10 pixel X axis
		-"r" for 10 pixel X axis
		-"d" for 10 pixel Y axis
		-"u" for -10 pixel Y axis
		-"q" for left click down
		-"Q" for left click Up
		-"w" for right click down
		-"W" for right click Up
		ex: echo "i udrrl" > mousek
	* echo "x [...]" > /dev/mousekernel:	moving the mouse following the X axios<br/>
		ex: `echo "x 250" > mousek`
	* echo "y [...]" > /dev/mousekernel:	moving the mouse following the Y axios<br/>
		ex: `echo "y 50" > mousek`
	* echo "l" > /dev/mousekernel:	handling the left click of the mouse device<br/>
		ex: `echo "ll" > mousek` or `echo "l" > mousek`
	* echo "r" > /dev/mousekernel:	handling the right click of the mouse device<br/>
<br/>
### More details: 
[fake_mouse_driver](https://github.com/kushsharma/mouse-driver)
<br/>
***
# Mouse driver using C and read the data from user space
### - How to run?
- Go to the directory that contains the source code.<br/>
1. `sudo mknod /dev/mouseDriver c 90 1`
2. `sudo chmod a+r+w /dev/mouseDriver`
3. `make`
4. `sudo insmod mouse.ko`
5. `sudo ./test`
6. - \[move mouse and see the result] 
<br/>
### More details: 
[mouse_driver](https://github.com/sachinparyani/mouse-device-driver)
<br/>