# COMP30023 - Assignment 1
Assignment 1 for COMP30023 - Computer Systems, a scheduler and memory manager.

## Overview
This is assignment 1 for the unimelb subject COMP30023 - Computer Systems, a scheduler and memory manager.

The basic implementation involves simulating handling incoming processes and dealing with how they are put into memory, swapped to disk and scheduled using various algorithms, specifically first come first serve and a multi level feedback queue.

Each process has a struct. Memory and the disk are represented as doubly linked lists of processes. A process can only exist on one at a time.
A queue is a cyclical array of pointers to processes. Depending on the algorithm, there is either 1 queue (fcfs) or 3 queues with different quantums (multi).
The simulation provides support for reading in processes, running them based on the scheduling algorithm, moving processes into memory (including algorithms for freeing space when required), swapping processes between disk and memory, and terminating processes once finished appropriately.

## Directory structure

The root directory has the code, including makefile, this readme, the spec and some bash scripts to run the test cases. It also contains the final report for part B.

- partA_cases contains all the input cases created for writing the initial project. These include the given test case, specInput (along with its appropriate output for the two algorithms) as well as additional tests for testing certain edge cases.
- partB_cases contains input cases and all other files relevant to part B, excluding the final report which is in the base directory.
- matt_test contains all the appropriate input and output files for matt_test.sh, as supplied by Matt Farrugia.
- old_output contains everything else. This is all non-essential to the project and was made purely for initial diagnostic purposes while writing the code, including discarded test cases and half written code.

## Results

Once I get my results back for this assignment, I will post the result so as to provide an indication of its quality.


