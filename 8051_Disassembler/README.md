# 8051 Disassember

[Korean](https://github.com/YHC03/8051_Tools/blob/main/8051_Disassembler/README-KR.md)  

Freeware  

#### Language Used
C

###### Library(Header File) Used
stdio.h: To use stdin, stdout and File I/O.  
stdlib.h: To use exit() function.  
string.h: To process blanks on input file path.  

---
## How to Run
Build and create .exe file. After creating the file, run program at command line by entering commands as "(FileName).exe" "(Location of .hex file).hex".  
If the file was not correctly loaded, the program stops while notifying there is a error.  
If the parity error occurs at .hex file, the program stops while notifying which line has the parity error.  
Otherwise, the (Name of the .hex file).a51 file, which contains the result, will be created at the location of the .hex file.  

---
## Used when Creating this Project
[8051 Simulator in the Same Repository](https://github.com/YHC03/8051_Tools/tree/main/8051_Simulator)  

---
Creator : YHC03  
Last Modified Date : 2024/05/11  