# Single Lane Bridge Problem
## A solution written in C to prevent deadlock and starvation

This repository contains four implementations related to the Single Lane Bridge problem, offering an analogous representation of synchronization issues in distributed and parallel systems. The goal is to apply theoretical knowledge acquired in the Parallel and Concurrent Programming course to practical scenarios.

The Single Lane Bridge problem serves as a model for scenarios where multiple processes contend for access to a shared resource, necessitating conflict resolution. In this analogy, vehicles symbolize individual processes, while the bridge represents the shared resource. The constraint that the bridge can accommodate traffic in only one direction at a time creates an ideal environment for exploring synchronization and coordination techniques. Consequently, this project encompasses a series of concurrent programs, written in C, designed to regulate vehicle access to the bridge, thereby mitigating deadlock (where no vehicle can cross) and starvation (where one or more vehicles fail to access the bridge).

The algorithm implementation is divided into four stages of increasing complexity, each corresponding to one of the proposed questions in the course assessment.

### Technologies
![Code::Blocks Badge](https://img.shields.io/badge/Code%3A%3ABlocks-41AD48?logo=codeblocks&logoColor=fff&style=for-the-badge) ![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)

### Problem 1: Cars Only

In this scenario, 100 cars ( n ) will traverse the bridge, 50 in each direction. Each car's arrival time ( T<sub>a</sub> ) is randomly distributed between 1 and 3 seconds, while the crossing time ( T<sub>c</sub> ) for each car is fixed at 5 seconds. A 1-second space ( T<sub>s</sub> ) is maintained between vehicles traveling in the same direction, allowing up to 5 cars to cross simultaneously. Upon approaching the bridge, a car checks if the current traffic aligns with its direction before proceeding. Once all cars in one direction have crossed, vehicles from the opposite direction gain access.

**Given instances:** n = 100, T<sub>a</sub> = 1 to 3 sec, T<sub>c</sub> = 5 sec, T<sub>s</sub> = 1 sec.

### Problem 2: Cars Only without Starvation

Problem 1 may lead to starvation, particularly when the queue becomes congested towards the execution's end, potentially hindering vehicles from the opposite direction. To address this, a mechanism ( P ) is introduced to balance bridge usage by alternating the traffic direction after every 5 vehicles have crossed. Since there are no signals on the bridge, drivers autonomously decide whether to cross or wait.

**Given instances:** n = 100, T<sub>a</sub> = 1 to 3 sec, T<sub>c</sub> = 5 sec, T<sub>s</sub> = 1 sec, P = 5.

### Problem 3: Cars and Trucks without Starvation

This scenario introduces trucks, which move at a slower pace and must cross the bridge individually for safety reasons. Now, 100 cars and 10 trucks ( m ) will traverse the bridge, with 50 cars and 5 trucks in each direction. Similar to previous problems, arrival times for both cars and trucks follow a random distribution, while each vehicle type has a distinct crossing time. The crossing time ( T<sub>t</sub> ) for each truck is fixed at 10 seconds. To prevent starvation, a mechanism ( U ) is implemented to alternate the traffic direction after every 5 cars or 1 truck has crossed.

**Given instances:** n = 100, m = 10, T<sub>a</sub> = 1 to 3 sec, T<sub>c</sub> = 5 sec, T<sub>t</sub> = 10 sec, T<sub>s</sub> = 1 sec, P = 5 or U = 1.

### Problem 4: Cars and Trucks on Two Bridges

In this variation, an additional single-lane bridge, suitable only for cars, is introduced alongside the main bridge. This secondary bridge can accommodate ( Q ) 2 cars simultaneously, with a total crossing time ( T<sub>c</sub>' ) of 4 seconds. With 120 cars expected to cross, this setup aims to improve vehicle flow by directing cars to the secondary bridge when the main bridge is occupied.

**Given instances:** n = 120, m = 10, T<sub>a</sub> = 1 to 3 sec, T<sub>c</sub> = 5 sec, T<sub>c</sub>' = 4 sec, T<sub>t</sub> = 10 sec, T<sub>s</sub> = 1 sec, T<sub>s</sub>' = 2 sec, P = 5 or U = 1, and Q = 2.

### Statistics

After the completion of each script, the following statistics are provided:

* Number of vehicles (cars and trucks) that crossed each direction.
* Minimum, maximum, and average waiting time of vehicles in the queue.
* Bridge utilization time.
