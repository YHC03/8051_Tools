# 8051 CLI Simulator

[Korean](https://github.com/YHC03/8051_Tools/blob/main/8051_Simulator/README-KR.md)  

Freeware  

#### Language Used
C

###### Library(Header File) Used
stdio.h: To use stdin, stdout and File I/O.  
stdlib.h: To use exit() function.  
string.h: To process blanks on input file path.  
math.h: To use pow() function.  
windows.h: To use system() function, especially PAUSE and cls command.  

---
## Introducing the Specific Files
8051_Variables: Saves constants related to 8051 chip and global variables of 8051 Chip.  
RunFunction: Read and run commands of 8051 Chip.  
RunSpecific: Actually run functions which is called at RunFunction.  
Timer_and_Interrupt: Runs 8051 timers and interrupts.  
FileInput: Read the .hex file to input.  
8051_Simulator: Manage and run all program.  

---
## How to Run
Build all and create .exe file. After creating the file, run program at command line by entering commands as "(FileName).exe" "(Location of .hex file).hex".  
If the file was not correctly loaded, the program stops while notifying there is a error.  
If the parity error occurs at .hex file, the program stops while notifying which line has the parity error.  

---
## Introduction to Run the Program

### 1. Initial Window
If you choose 0, it runs as step-by-step debuging mode.  
If you choose 1(or any other number except 0), the simulator automatically runs until the program counter goes over the .hex file's last program counter(Regardless of the state of interrupt), the program counter's value is the same as the last value(meaning that the program has just entered an infinite loop that does nothing), or the chip encounters in sleeping mode by modifying the value of PCON register. After the program stops, it prints the results, and terminates the program.  

---
### 2. Debug Window
If you choose 0(or any other number except 1), it runs the next command.  
If you choose 1, you can set the input state of Port 0-3.  
It prints memory state of 8051(also prints the state of PSW, TCON memory in bit), latch information of Port 0-3, and command that runs next time.  

---
## Available Functions
- Checking parity of .hex file  
- All insturction set of 8051 except MOVX command  
- 8051 Timer/Counter and Interrupt  
- Visualize Latch information of Port 0-3  
- Port 0-3 Input(You cannot modify only a specific port. However, you can just type the privious state of the port.)  
- Sleeping Functions of PCON Register  

##### Unavailable Functions
- Serial-related functions(Also Serial interrupt service routine was not implemented)  
- External memory is not available  

##### Things to know
When calling getPortValue() function at inputDat() function, this simulator will assume that there is a pull-up resistor at P0 Port.  
The SBUF(0x99) has 2 different memory, one for send and another for receive. The I/O on the memory map printed means (Input for 8051, Output of 8051).  
Outputs of JMP-related command are based on assembler-available commands, so program counter value of the destination will be printed at the label's site, not the difference between the program counter value of current location and the program counter value of the label's location.  

---
## Reference 
8051 Instruction Set  
Lecture notes of Microprocessor and HDL class, Hongik University(First Semester of 2024)  

---
Creator : YHC03  
Last Modified Date : 2024/06/01  