# Readme.md

## Why `NamePipe` was chosen for IPC
- Small data volumes (1 string up to 10 characters) and a low message frequency (1-2 times per second).
- It is capable of establishing bidirectional communication with other applications.
- It supports synchronization.
- It is easy to configure and use.
- 
## Why `message queue` and `mutex` was chosen for synchronization threads
-It provides a simple way to organize a queue for sequential processing of messages.

## How to Build and Run
````
- mkdir build
- cd build
- cmake ..

