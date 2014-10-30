#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio> // grrrrrrrrrrrrrrrr

#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <random>

#include "busy.h"

#define STATUS_CLONE(st) ((st) >> 8 == (SIGTRAP | (PTRACE_EVENT_CLONE << 8)))
#define STATUS_FORK(st)  ((st) >> 8 == (SIGTRAP | (PTRACE_EVENT_FORK << 8)))
#define STATUS_VFORK(st) ((st) >> 8 == (SIGTRAP | (PTRACE_EVENT_VFORK << 8)))

char* tool_return_point;

/* prints the contents of all general purpose registers, rip, and eflags. */
void print_registers(user_regs_struct& regs) {
    std::clog << "rax 0x" << std::setw(16) << std::setfill('0') << regs.rax << std::endl;
    std::clog << "rbx 0x" << std::setw(16) << std::setfill('0') << regs.rbx << std::endl;
    std::clog << "rcx 0x" << std::setw(16) << std::setfill('0') << regs.rcx << std::endl;
    std::clog << "rdx 0x" << std::setw(16) << std::setfill('0') << regs.rdx << std::endl;
    std::clog << "rsi 0x" << std::setw(16) << std::setfill('0') << regs.rsi << std::endl;
    std::clog << "rdi 0x" << std::setw(16) << std::setfill('0') << regs.rdi << std::endl;
    std::clog << "rbp 0x" << std::setw(16) << std::setfill('0') << regs.rbp << std::endl;
    std::clog << "rsp 0x" << std::setw(16) << std::setfill('0') << regs.rsp << std::endl;
    std::clog << "r8  0x" << std::setw(16) << std::setfill('0') << regs.r8  << std::endl;
    std::clog << "r9  0x" << std::setw(16) << std::setfill('0') << regs.r9  << std::endl;
    std::clog << "r10 0x" << std::setw(16) << std::setfill('0') << regs.r10 << std::endl;
    std::clog << "r11 0x" << std::setw(16) << std::setfill('0') << regs.r11 << std::endl;
    std::clog << "r12 0x" << std::setw(16) << std::setfill('0') << regs.r12 << std::endl;
    std::clog << "r13 0x" << std::setw(16) << std::setfill('0') << regs.r13 << std::endl;
    std::clog << "r14 0x" << std::setw(16) << std::setfill('0') << regs.r14 << std::endl;
    std::clog << "r15 0x" << std::setw(16) << std::setfill('0') << regs.r15 << std::endl;
    std::clog << "rip 0x" << std::setw(16) << std::setfill('0') << regs.rip << std::endl;
    std::clog << "efl 0x" << std::setw(8) << std::setfill('0') << regs.eflags << std::endl;
}

/* allocate memory in the child process for the busy loop. */
bool allocate_busy_loop(pid_t pid, unsigned long& addr) {
    user_regs_struct proc_regs, inst_regs;
    unsigned long rip;
    int proc_stat;
    unsigned long proc_word, inst_word;
    unsigned long syscall_word = 0xcc050f90; 
    
    // obtain rip of process entry.
    if (ptrace(PTRACE_GETREGS, pid, 0, &proc_regs) < 0) {
        return false;
    }
    rip = proc_regs.rip;
    
    // read a word of text.
    errno = 0;
    proc_word = ptrace(PTRACE_PEEKTEXT, pid, rip, 0);
    if (errno != 0) {
        return false;
    }
    
    // insert system call and breakpoint over original word.
    inst_word = (proc_word & 0xffffffff00000000) | syscall_word;
    if (ptrace(PTRACE_POKETEXT, pid, rip, (void*)inst_word) < 0) {
        return false;
    }
    
    // update registers for mmap system call.
    inst_regs = proc_regs;
    inst_regs.rax = 9; // mmap is syscall 9
    inst_regs.rdi = 0;
    inst_regs.rsi = 4096;
    inst_regs.rdx = PROT_READ | PROT_WRITE | PROT_EXEC;
    inst_regs.r10 = MAP_SHARED | MAP_ANONYMOUS;
    inst_regs.r8  = -1;
    inst_regs.r9  = 0;
    if (ptrace(PTRACE_SETREGS, pid, 0, &inst_regs) < 0) {
        return false;
    }
    
    // let process perform mmap.
    if (ptrace(PTRACE_CONT, pid, 0, 0) < 0) {
        return false;
    }
    
    // catch the breakpoint after the system call.
    wait(&proc_stat);
    
    // read the registers to obtain mmap return value. 
    if (ptrace(PTRACE_GETREGS, pid, 0, &inst_regs) < 0) {
        return false;
    }
    addr = inst_regs.rax;
    
    // abort if mmap returned error.
    if (addr < 0) {
        return false;
    }
    
    // restore registers and first word.
    if (ptrace(PTRACE_SETREGS, pid, 0, &proc_regs) < 0) {
        return false;
    }
    if (ptrace(PTRACE_POKETEXT, pid, rip, (void*)proc_word) < 0) {
        return false;
    }
    
    std::clog << pid << " mmap instrumentation site at 0x" << addr << std::endl;
    
    return true;
}

/* handles a breakpoint. note: this only handles the one breakpoint. */
void handle_breakpoint(pid_t pid, unsigned long busy_addr, unsigned long addr, unsigned long text) {
    typedef std::pair<unsigned long, unsigned long> stop_data;
    static std::default_random_engine ran_gen;
    static std::uniform_int_distribution<unsigned int> ran_dist(1000, 2000000);
    static std::map<pid_t, stop_data> pid_map;
    user_regs_struct proc_regs;
    unsigned long breakpoint;
    unsigned int count;
    
    // get registers.
    ptrace(PTRACE_GETREGS, pid, 0, &proc_regs);
    
    // did we hit the one breakpoint?
    if (proc_regs.rip - 1 == addr) {
        std::clog << " hit breakpoint one" << std::endl;

        // store rip and rcx in map.
        pid_map[pid] = stop_data(proc_regs.rip - 1, proc_regs.rcx);
        
        // generate random number for busy loop.
        count = ran_dist(ran_gen);
        
        // write busy loop data into registers.
        proc_regs.rip = busy_addr;
        proc_regs.rcx = count;
        ptrace(PTRACE_SETREGS, pid, 0, &proc_regs);
        
        return;
    }
    
    // did we hit the end of the busy loop?
    if ((char*)proc_regs.rip == tool_return_point + 1) {
        std::clog << " hit loop return" << std::endl;
        // find data in map.
        auto it = pid_map.find(pid);
        
        // restore rip and rcx.
        proc_regs.rip = it->second.first;
        proc_regs.rcx = it->second.second;
        
        // remove from map.
        pid_map.erase(it);
        
        // temporarily erase breakpoint.
        ptrace(PTRACE_POKETEXT, pid, addr, (void*)text);
        
        // restore the registers.
        ptrace(PTRACE_SETREGS, pid, 0, &proc_regs);
        
        // single step over next instruction and wait for state change.
        if (ptrace(PTRACE_SINGLESTEP, pid, 0, 0) < 0) {
            std::clog << "error singlestep " << std::endl;
        }
        waitpid(pid, 0, 0/*__WALL*/);
        
        // replace breakpoint.
        ptrace(PTRACE_GETREGS, pid, 0, &proc_regs);
        ++proc_regs.rip;
        breakpoint = (text & 0xffffffffffffff00) | 0xcc;
        ptrace(PTRACE_POKETEXT, pid, addr, (void*)breakpoint);
        ptrace(PTRACE_SETREGS, pid, 0, &proc_regs);

        return;
    }
    
    // non-registered breakpoint.
    std::clog << " non-registered breakpoint at 0x" << proc_regs.rip - 1 << std::endl;
    
}

/* inserts a breakpoint into the child process. */
bool insert_breakpoint(pid_t pid, unsigned long addr, unsigned long& text) {
    unsigned long breakpoint;
    
    // read original text.
    errno = 0;
    text = ptrace(PTRACE_PEEKTEXT, pid, addr, 0);
    if (errno != 0) {
        return false;
    }
    
    // insert breakpoint into text. 
    breakpoint = (text & 0xffffffffffffff00) | 0xcc;
    if (ptrace(PTRACE_POKETEXT, pid, addr, (void*)breakpoint) < 0) {
        return false;
    }
    
    std::clog << pid << " inserted breakpoint one at 0x" << addr << std::endl;
    
    return true;
}

/* insert the busy loop into child process's memory. */
bool insert_busy_loop(pid_t pid, unsigned long addr) {
    unsigned long ptr = addr;
    
    // copy the tool over to the allocated space.
    for (unsigned long i = 0; i < tool_busy_length; i += 8, ptr += 8) {
        if (ptrace(PTRACE_POKETEXT, pid, ptr, (void*)*(long*)(&tool_busy_loop + i)) < 0) {
            return false;
        }
    }
    
    // set the value of tool_return_point.
    tool_return_point = (char*)addr + (&tool_busy_break - &tool_busy_loop);
    
    std::clog << pid << " inserted busy loop code of length 0x" << 
                 tool_busy_length << " bytes" << std::endl;

    return true;
}

int main(int argc, char** argv, char** envp) {
    std::list<unsigned long> run_pids;
    user_regs_struct proc_regs;
    unsigned long busy_loop_address;
    unsigned long original_text;
    unsigned long break_address = 0x40156e;
    unsigned long pid, curr_pid, new_pid;
    unsigned int pause_reps;
    int rv;
    int proc_stat;
    int proc_sig;
    bool run_proc = true;
    
    // basic args check.
    if (argc < 2) { 
        std::clog << argv[0] << ": error, requires one arg, executable name." << std::endl;
        return -1;
    }
    
    // fork to debugger. skipping file name checks.
    pid = fork();
    if (pid == 0) {
        
        // initialize child process for tracing/debugging.
        rv = ptrace(PTRACE_TRACEME, 0, 0, 0);
        if (rv < 0) { 
            std::clog << argv[0] << ": cannot initialize tracer." << std::endl;
            return -1;
        }
        
        // execute new program.
        execve(argv[1], argv + 1, envp);
        std::clog << argv[0] << ": cannot execute " << argv[1] << std::endl;
        return -1;
    }
    
    // begin tool. add the pid to list of running pids.
    run_pids.push_back(pid);
    std::clog << std::hex << "     PROTO-TOOL INITIALIZATION" << std::endl;
    
    // pause until process has begun execution or aborted. 
    wait(&proc_stat);
    
    // check if process aborted due to error.
    if (WIFEXITED(proc_stat)) {
        return 0;
    }
    
    // instrument process by allocating memory and inserting busy loop code.
    if (!allocate_busy_loop(pid, busy_loop_address)) {
        // error, kill process.
        kill(pid, SIGKILL);
        std::clog << argv[0] << ": cannot allocate process memory." << std::endl;
        return -1;
    }
    
    if (!insert_busy_loop(pid, busy_loop_address)) {
        // error, kill process.
        kill(pid, SIGKILL);
        std::clog << argv[0] << ": cannot instrument process." << std::endl;
        return -1;
    }
    
    // insert breakpoints into process.
    if (!insert_breakpoint(pid, break_address, original_text)) {
        // error, kill process.
        kill(pid, SIGKILL);
        std::clog << argv[0] << ": cannot instrument process." << std::endl;
        return -1;
    }
    
    // set ptrace options to allow us to trace additional threads.
    if (ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACECLONE | 
               PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK) < 0) {
        std::clog << argv[0] << ": cannot configure tracing." << std::endl;
        kill(pid, SIGKILL);
        return -1;
    }
    
    // start execution of process.
    ptrace(PTRACE_CONT, pid, 0, 0);
    std::clog << std::endl << "     BEGIN EXECUTION" << std::endl;
    
    // run process and act on state changes.
    while (run_proc) {
        // wait for a state change in the process and its threads.
        curr_pid = waitpid(-1, &proc_stat, __WALL);
        proc_sig = WSTOPSIG(proc_stat);
        std::clog << curr_pid;
        
        // was change due to new thread?
        if (STATUS_CLONE(proc_stat)) {
            // get pid of new thread and add to list.
            ptrace(PTRACE_GETEVENTMSG, curr_pid, 0, &new_pid);
            run_pids.push_back(new_pid);
            
            // signal new thread and its parent to start.
            ptrace(PTRACE_CONT, curr_pid, 0, 0);
            pause_reps = 0;
            do {
                rv = ptrace(PTRACE_CONT, new_pid, 0, 0);
                ++pause_reps;
            } while (rv == -1 && errno == ESRCH);
            std::clog << " cloned new thread " << new_pid << std::endl;
            //std::clog << "     looped " << pause_reps << std::endl;
            
            continue;
        }
        
        // was change due to fork or vfork?
        if (STATUS_VFORK(proc_stat) || STATUS_FORK(proc_stat)) {
            // ignoring for now...
            continue;
        }
        
        // was change due to hitting a breakpoint or signal?
        if (WIFSTOPPED(proc_stat)) {
            // handle the breakpoint.
            if (proc_sig == SIGTRAP) {
                handle_breakpoint(curr_pid, busy_loop_address, break_address, original_text);
                
                // continue executing thread.
                pause_reps = 0;
                do {
                    rv = ptrace(PTRACE_CONT, curr_pid, 0, 0);
                    ++pause_reps;
                } while (rv == -1 && errno == ESRCH);
                //std::clog << "     looped " << pause_reps << std::endl;
                continue;
            }
            
            // if not a breakpoint, then deliver the signal to process.
            // in the future, we may want to log or use this to determine
            // if the process crashed.
            std::clog << " received signal " << proc_sig << std::endl;
            pause_reps = 0;
            do {
                rv = ptrace(PTRACE_CONT, curr_pid, 0, 0);
                ++pause_reps;
            } while (rv == -1 && errno == ESRCH);
            
            continue;
        }
        
        // was change due to terminating thread?
        if (WIFEXITED(proc_stat)) {
            // remove pid from list of running pids.
            run_pids.remove(curr_pid);
            
            // exit loop if there are no running pids.
            if (run_pids.empty()) {
                run_proc = false;
            }
            
            std::clog << " exit" << std::endl;
            ptrace(PTRACE_CONT, pid, 0, 0);
            
            continue;
        }
        
        std::clog << " other event" << std::endl;
        
    }
    
    return 0; 
    
}





