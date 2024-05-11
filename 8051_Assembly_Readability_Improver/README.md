# 8051 Assembly Code Readability Improver

[Korean](https://github.com/YHC03/8051_Tools/blob/main/8051_Assembly_Readability_Improver/README-KR.md)  

Freeware  

#### Language Used
C

###### Library(Header File) Used
stdio.h: To use stdin, stdout and File I/O.  
string.h: To process blanks on input file path.  

---
## How to Run
.Build and create .exe file. After creating the file, run program at command line by entering commands as "(FileName).exe" "(Location of Assembly file).a51".  
If the file was not correctly loaded, the program stops while notifying there is a error.  
Otherwise, the (Name of the .a51 file)_Processed.a51 file, which contains the result, will be created at the location of the .a51 file.  

## Functions
1. If there is blanks or tabs, change with a blank(If the next letter was found, blanks will be writted with the next letter).  
- If the line starts with blanks, delete that blanks.  
- If the :(colon) was first encountered at the line, delete the blank between label name and colon.  
  
2. Change small letter Alphabet to Capital Letter, except for comments.  
3. If there is no blanks behind ,(comma), create a blank.  
4. Make at least one blank just behind the ;(semicolon), which means comment in Assembly.  
- There is also a commented code that writes comment data exactly the same, regardless the blank right behind the semicolon.  

5. Delete the blanks on the line which only has blanks.(It does not delete the entire line only with blanks.)  

---
Creator : YHC03  
Last Modified Date : 2024/05/11  