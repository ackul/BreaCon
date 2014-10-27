BreaCon
=======

Tracking Document

10/12/2014
Meeting notes
Alex and I concurred on the following design problems that need to be solved before we begin implementation

1. Where to instrument, JVM or the source code. What should we be doing?
2. Find tools which can be used to detect concurrent problems for preliminary analysis, static evaluation.
3. Code coverage. BIG Problem.
4. Understanding the scheduling and how context switches can happen internally.
5. We can test our approach for deadlock and race conditions in popular problems like Dining philosphers.
6. We need a seeding approach and our seeding should take into account the history of previous interruption decision when arriving at the next on.
7. Network load and I/O delays lead to different Interleavings
8. Preservation of the order of shared memory access and synchronization events.

Division of Paper Reading

[ALEX]
///////////////////////////
Finding and reproducing heisenbugs in the program
http://research.microsoft.com/en-us/projects/chess/osdi2008-chess.pdf
Replay is pretty good. We can use it for replay design later.

Effective data race detection for the kernel
http://research.microsoft.com/pubs/139266/DataCollider%20-%20OSDI2010.pdf
Breakpoints on data address. 

Improved multithreaded unit testing
http://mir.cs.illinois.edu/gliga/papers/JagannathETAL11IMunit.pdf

Eraser
http://homes.cs.washington.edu/~tom/pubs/eraser.pdf

Concurrent BreakPoints
http://www.eecs.berkeley.edu/Pubs/TechRpts/2011/EECS-2011-159.pdf

A randomized scheduler with probabilistic gurantees of finding bugs
http://research.microsoft.com/pubs/118655/asplos277-pct.pdf
////////////////////////////////

RaceTrack
http://research.microsoft.com/pubs/65170/sosp05-racetrack.pdf


[ACHIN]
///////////////////////////
Berkley Papers
There are 4 papers here.
http://srl.cs.berkeley.edu/~ksen/calfuzzer/

Multithreaded Java Program Test Generation
https://www.cs.purdue.edu/homes/xyzhang/fall07/Papers/test-thread.pdf

A Lightweight approach to make concurrency issues reproducible
http://zhang-sai.github.io/pdf/luo-fase10.pdf

First-class Concurrency Testing and Debugging
http://research.microsoft.com/en-us/projects/chess/ec2-submission.pdf
/////////////////////

25/10/2014

a. The priority of threads is also not a forceful mechanism of pausing threads because of the concept of priority inversion
b. I have made a program in which if the breakpoint is toggled at the withdraw operation, exactly how the atomicity paper mentions, interleaving happens and this leads to an atomicity violation.



