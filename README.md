# CS6210_Lab_2
Barrier Synchronization (group assignment)

## Objective
The goal of this project is to implement barrier synchronization using OpenMP and OpenMPI. 

## Overview
- OpenMP allows us to run parallel algorithms on **shared-memory** multicore machines. Here we implemented two spin barriers: **centralized barrier** and **dissemination barrier**, for OpenMP.
- OpenMPI allows us to run parallel algorithms on **distributed memory system**. We implemented two spin barriers using OpenMPI: **centralized barrier** and **MCS tree barrier**.
- Finally, we merged one of the OpenMP barriers (here we selected centralized barrier) and one of the OpenMPI barriers (we selected MCS tree barrier) together to simulate the synchronization between the threads running within a machine and between the cluster nodes.

## Notes
- More details including the environment, barrier introduction, performance evaluation and analysis, and discussion are descirbed in the ```report.pdf``` file, please refer to that document.

