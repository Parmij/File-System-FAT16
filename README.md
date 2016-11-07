# File System FAT16 
---
*File System FAT16* is a college project to operating system 2 (OS2) in Yarmouk Private University (YPU). It is a program under MS-DOS that reads a command from the keyboard and executes it in an infinite loop (i.e. keeps reading commands and executing them). 
The program uses absread only to access the drive "D:". it controls commands as follows:
- exit: terminate the program.
- type [filename]: shows the contents of the file on the screen.
- dir: it lists the files and the root directories of drive "D:" and the result shows exactly the same result produced by the "dir" command of MS-DOS.
- dir [path]: Same as "dir" but for a given path (like "d:\dir1\dir2").
- undelete: it goes through the deleted files in the root directory. For each one, it asks the user to enter the first character to undelete it. The user may select not to undelete this file.
- search [file]: it searches for the file recursively in drive "d:".
