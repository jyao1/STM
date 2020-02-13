
This is the CMake based build system for the STM.

   CMake Options:

        BIOS - suboption: coreboot

		This option starts the SMI Handler in a 32-bit virtual machine.

		Default: the SMI Handler is startted in a 64-bit virtual machine.

        UARTBASE - suboption: UART I/O address in hex format.
                
		This option allows the use of alternative UART ports for
		STM console output.

		Default: Console output will be directed to port 0x3F8

        HEAPSIZE - suboption: size of STM heap space in hex format

		This option declares the STM heap space.  The developer
		must ensure this size will not make the STM too large 
		for the MSEG.

		Default: STM heap space is set at 0x246000

	BUILD - suboptions: release, debug

		This option currently defines the amount of console output the
		STM generates.

		debug - all console output
		release - no console output

		Default: debug

         

Steps for building:

    (1) Create a build directory in the Stm directory.
    (2) Inside the build directory issue "cmake .. " then make
        --> For coreboot do "cmake .. -DBIOS=coreboot" then make
    (3) The StmPkg/Core directory will contain the results of the build
	--> for coreboot: (1) Create a directory
                          (2) Move the stm.bin file to that directory
                          (3) In coreboot (make menuconfig), 
			      configure the STM and point the 
                              "STM binary file" to the stm.bin file
			  (4) rebuild by doing "make clean && make"

	Other files in the StmPkg/Core directory:
			(1) Stm - an elf based load module, this is passed through
			          objdump to produce stm.bin
                        (2) stm.map - loader map


