# openPaint
(Systems Final Project Term 1)

## Members
Sally Bao<br>
Sandy Fang

## Running the Project
openPaint requires gtk+2.0 (this is usually already on Ubuntu machines)<br>
Git clone the repository<br>
Run make to compile all the files<br>
Run select to start the server (this requires PORT 8003 to be unused)<br>
Run draw to open a client. Multiple clients can be run simultaneously<br>

## Files
+ Makefile
+ Design.txt
+ draw.c
+ draw.h
+ select.c
+ select.h

## References
openPaint contains some code referenced from online demos/tutorials:<br>
GTK basic drawing mechanism: http://snipplr.com/view/57664/ <br>
Managing input using select() (we originally tried with a forking server and it did not turn out well): https://vidyakv.wordpress.com/2011/11/29/multi-client-chat-server-in-c/
