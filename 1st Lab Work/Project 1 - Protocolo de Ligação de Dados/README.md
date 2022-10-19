# FEUP-RC-P1

## INSTRUCTIONS FOR SERIAL PORT PROTOCOL

## :nut_and_bolt: Development Environment

A significant part of the project can be executed with the aid of virtual serial ports. To set up this environment do the following steps:

* Use a Linux machine (it can be a virtual machine).
* Use the socat command to create 2 virtual serial ports linked together
```zsh
sudo socat -d  -d  PTY,link=/dev/ttyS10,mode=777   PTY,link=/dev/ttyS11,mode=777
```
* And use the ttyS10 and ttyS11 to establish the connection
* On different terminals run the transmitter connected to ttyS10 and the receiver connected to ttyS11
* In more advanced stages of the work, you can use a program that simulates the serial cable on and off

## :file_folder: Project Structure

```
.
├── bin
├── cable
│	└── cable.c
├── include
│   ├── application_layer.h
│   └── link_layer.h
├── src
│   ├── application_layer.c
│   └── link_layer.c
├── .gitignore
├── main.c
├── Makefile
├── penguin.gif
└── README.md
```

| File        | Details                                                                                                                                          |
|-------------|--------------------------------------------------------------------------------------------------------------------------------------------------|
| bin/        | Compiled binaries.                                                                                                                               |
| cable/      | Virtual cable program to help test the serial port. This file must not be changed.                                                               |
| include/    | Header files of the link-layer and application layer protocols. These files must not be changed.                                                 |
| src/        | Source code for the implementation of the link-layer and application layer protocols. Students should edit these files to implement the project. |
| main.c      | Main file. This file must not be changed.                                                                                                        |
| Makefile    | Makefile to build the project and run the application.                                                                                           |
| penguin.gif | Example file to be sent through the serial port.                                                                                                 |

## :rocket: Instructions to Run the Project

1. Edit the source code in the src/ directory.
2. Compile the application and the virtual cable program using the provided Makefile.
3. Run the virtual cable program (either by running the executable manually or using the Makefile target):
	```zsh
	./bin/cable_app
	make run_cable
	```

4. Test the protocol without cable disconnections and noise
	>Run the receiver (either by running the executable manually or using the Makefile target):
	>	```zsh
	>	./bin/main /dev/ttyS11 rx penguin-received.gif
	>	make run_tx
	>	```

	> Run the transmitter (either by running the executable manually or using the Makefile target):
	>	```zsh
	>	./bin/main /dev/ttyS10 tx penguin.gif
	>	make run_rx
	>	```

	> Check if the file received matches the file sent, using the diff Linux command or using the Makefile target:
	>	```zsh
	>	diff -s penguin.gif penguin-received.gif
	>	make check_files
	>	```

5. Test the protocol with cable disconnections and noise 
   1. Run receiver and transmitter again 
   2. Quickly move to the cable program console and press 0 for unplugging the cable, 2 to add noise, and 1 to normal 
   3. Check if the file received matches the file sent, even with cable disconnections or with noise

##  :page_facing_up: Resources
* [All resources provided by the teacher](https://drive.google.com/file/d/13jhRVbEnQ_5UDf5XCluBkXh_ODMqLdvO/view?usp=sharing)
* [Serial HOWTO](https://tldp.org/HOWTO/Serial-HOWTO.html)
* [Serial Programming HOWTO](https://tldp.org/HOWTO/Serial-Programming-HOWTO/)