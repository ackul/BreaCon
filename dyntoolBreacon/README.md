BreaCon
=======

Breaking the Concurrency: Randomizing the Thread Scheduler to unravel hidden bugs

Things completed
1. Framework to instrument a Pthread Binary (Used Dyninst - http://www.dyninst.org/)
2. We have created a runtime library that is loaded 

Things to be done
1.  I think we should make the tool parameterized. For example, the user should be able to choose the sleep time and probability
How to run?

1. For running Breacon.cpp the makfile should include the DYNINST "include" and "lib"directory
2. The two directories are present in compressed form.
3. Make file has targets for both the breacon (mutator) and the shared library

