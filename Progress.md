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

10/31/2014

Research/Paper Study
	• Read 15 papers on concurrency, race detection and thread scheduling
	• People have used instrumentation and breakpoint events to randomize thread schedules using mechanisms like thread.sleep. 
	• Our intention was to allow replays as in most of the papers we found that replay is really difficult to achieve for concurrency bugs. 

JVM Schedule or Pthreads
	• Research has been focused on both with slightly more focus on Java. Possibly, because application developers are using Java to write multithreaded apps.
	• We had a tendency towards both actually. Pthreads because less no. of people have touched c applications to find concurrency issues (Didn't find many papers :)) and Java because critical web applications are made in Java.

Which Concurrency Issues?
Issues like Race conditions, deadlocks are heavily studied and many tools have been made which do systematic analysis. This is a good approach for small program but if we want to scale to large programs and find certain bugs with some probability we need to use randomization. One type of issue that exists despite the existence of locks 

Breakpoints / Semaphores What?
We needed some algorithm to control threads. Pausing one thread, starting another thread basically causing some context switches.

Algorithm for Java Research

Blocking and Unblocking Threads
	• Each thread has a unique lock object.
	• A thread can be blocked and unblocked by calling wait and Notify on that objecy.
	• If we call wait, the current thread is blocked. If present, the next thread will be scheduled. JVM randomly chooses one of the threads. If no thread is present, we will get a deadlock.

Approach
We inject Bytecode for a function fuzz() as "appropriate positions". The method takes in a PRG and randomly chooses one of the following options:

DO NOTHING

OR

Take a random thread Q from the thread list and call Q.Unblock which basically does Q.lock.notify. Then caller thread calls. t.block on itself that is it blocks itself by calling t.lock.wait.

At one time only one thread is executing. Need to ask whether this is a good approach for randomization? If we have one thread running at a time I assume replay would be easy.

Need suggestions on the algorithm. Can we have a deadlock or any other problem in our approach. We would have to block the threads before they run. Is that a good thing to do?

Algorithm for Pthreads
Use Breakpoints "appropriate positions" and whenever a thread reaches those points, on the basis of a random number, make a signal to a function which puts the thread on a busy loop and lets other threads execute.

Finding Instrumentation Points
	• Points can be distinguished on the basis of:
		○ Unshared - Local Variables as they are stored on the thread stack
		○ Shared - Global variables and the heap variables
		○ Protected - Synchronized, locks
	• Instrument all shared object accesses/variables and synchronized functions.
	• Static analysis using existing tools like race detector.

Progress
	• Understood how Byte code works and how to instrument it. Most of the papers have used SOOT compiler to do it which does it at an abstract level. It generate an abstract representation known as Jimple which is closer to the source (makes it easy to instrument). However, I am not sure if we should use it. It may not change the semantics of the program, would it impact the structure of the program??? Need to ask Prof. Bart.
	• We are working on bytecode level instrumenters mainly ASM and Javassist. Got a basic example working.
	• We Wrote a atomicity violation replica for a C program and instrumented using Ptrace. 
	• We have an algorithm in mind that we intend to integrate in our tool. Lot of improvements needed.  


