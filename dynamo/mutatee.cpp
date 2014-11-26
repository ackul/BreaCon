#include "log.h"
#include "mutatee.h"

#include <Event.h>
#include <BPatch.h>
#include <BPatch_addressSpace.h>
#include <BPatch_function.h>
#include <BPatch_image.h> // may not need
#include <BPatch_module.h>
#include <BPatch_point.h>
#include <BPatch_process.h>
#include <PCProcess.h>

#define nullptr 0

/* declarations for callback functions. */
static void callback_create(BPatch_process*, BPatch_thread*);
static void callback_destroy(BPatch_process*, BPatch_thread*);
static void callback_exit(BPatch_thread*, BPatch_exitType);
static void callback_signals(BPatch_point*, long, std::vector<Dyninst::Address>*);
static Dyninst::ProcControlAPI::Process::cb_ret_t callback_lowsig(Dyninst::ProcControlAPI::Event::const_ptr);

// global variable to represent dyninst library. 
static BPatch bpatch;

/* global variable to mark if BPatch has been initialized. */
static bool bpatch_initialized = false;

/* returns true if the passed name is a blacklisted module. */
static bool blacklisted_module(char* name) {
    std::string module(name);
    
    // black list dyninst, dynamo, and standard libraries.
    if (module.find("libdyninst") == 0 | module.find("libdynamo") == 0 |
        module.find("crtstuff") == 0   | module.find("libstdc++") == 0 |
        module.find("libm") == 0       | module.find("libgcc") == 0 |
        module.find("libc") == 0       | module.find("libdl") == 0 |
        module.find("libpthread") == 0) {
            return true;    
    }
    return false;
}

/* initializes the BPatch / Dyninst library. */
static bool bpatch_initializer() {
    static std::set<long> signals;
    bool success = true;
    
    
    // enable trampoline recursion to improve performance. 
    bpatch.setTrampRecursive(false);
    
    // disable parsing debugging symbols to improve peformance.
    bpatch.setDebugParsing(false);
    log::info() << "enabled optimized backend settings." << std::endl;
    
    // set error handler. this prevents errors from being printed by dyninst.
    // TODO
    
    // set signal callback.
    // build set to catch first 36 signals.
    for (int i = 0; i < 36; ++i) {
        signals.insert(i);
    }
    
    // set the callback and log.
    if (bpatch.registerSignalHandlerCallback(callback_signals, signals)) {
        log::info() << "registered signal callback for signals [0, 35]." << std::endl;
    } else {
        log::debug() << "could not register signal callback." << std::endl;
        success = false;
    }
    
    // set the thread create and destroy callbacks.
    if (bpatch.registerThreadEventCallback(BPatch_threadCreateEvent, callback_create)) {
        log::info() << "registered thread creation callback." << std::endl;
    } else {
        log::debug() << "could not register thread creation callback." << std::endl;
        success = false;
    }
    if (bpatch.registerThreadEventCallback(BPatch_threadDestroyEvent, callback_destroy)) {
        log::info() << "registered thread destruction callback." << std::endl;
    } else {
        log::debug() << "could not register thread destruction callback." << std::endl;
        success = false;
    }
    
    // fork and exec callbacks.
    // TODO
    
    // set the exit callback.
    bpatch.registerExitCallback(callback_exit);
    if (bpatch.registerExitCallback(callback_exit) == callback_exit) {
        log::info() << "registered process exit callback." << std::endl;
    } else {
        log::debug() << "could not register rpocess exit callback." << std::endl;
        success = false;
    }
    
    // set low-level signal callback.
    /*if (Dyninst::ProcControlAPI::Process::registerEventCallback(Dyninst::ProcControlAPI::EventType::Signal, callback_lowsig)) {
        log::info() << "registered low-level signal callback." << std::endl;
    } else {
        log::debug() << "could not register low-level signal callback." << std::endl;
        success = false;
    }
    
    // set low-level crash callback.
    if (Dyninst::ProcControlAPI::Process::registerEventCallback(Dyninst::ProcControlAPI::EventType::Crash, callback_lowsig)) {
        log::info() << "registered low-level crash callback." << std::endl;
    } else {
        log::debug() << "could not register low-level crash callback." << std::endl;
        success = false;
    }*/
    
    bpatch_initialized = true;
    return success;
}

static void callback_create(BPatch_process* proc, BPatch_thread* thread) {
    log::info() << "process [" << proc->getPid() << "] created new thread ["
                << thread->getLWP() << "]." << std::endl;
}

static void callback_destroy(BPatch_process* proc, BPatch_thread* thread) {
    log::info() << "process [" << proc->getPid() << "] destroyed thread ["
                << thread->getLWP() << "]." << std::endl;
}

static void callback_exit(BPatch_thread* thread, BPatch_exitType type) {
    switch (type) {
        case BPatch_exitType::ExitedNormally:
            log::info() << "process [" << thread->getProcess()->getPid() << ":"
                        << thread->getLWP() << "] is exiting normally." 
                        << std::endl;
        break;
        case BPatch_exitType::ExitedViaSignal:
            log::info() << "process [" << thread->getProcess()->getPid() << ":"
                        << thread->getLWP() << "] is exiting by signal." 
                        << std::endl;
        break;
        default:
            log::debug() << "process [" << thread->getProcess()->getPid() << ":"
                         << thread->getLWP() << "] is exiting without exit." 
                         << std::endl;
    }
}

static void callback_signals(BPatch_point* at, long signal, std::vector<Dyninst::Address>* handlers) {
    log::info() << "current process received signal " << signal << "." << std::endl;
}

static Dyninst::ProcControlAPI::Process::cb_ret_t callback_lowsig(Dyninst::ProcControlAPI::Event::const_ptr event) {
    if (event->getEventType().code() == Dyninst::ProcControlAPI::EventType::Signal) {
        log::info() << "received signal " << event->getEventSignal()->getSignal() << "." << std::endl;
    } else if (event->getEventType().code() == Dyninst::ProcControlAPI::EventType::Crash) {
        log::info() << "crashing under " << event->getEventCrash()->getTermSignal() << "." << std::endl;
    }
    return Dyninst::ProcControlAPI::Process::cbDefault;
}

/* structure to hold mutatee data and hide implementation from other files. */
struct mutatee::mdata {
    const char** argv;
    const char** envp;
    BPatch_addressSpace* addr_space;
    BPatch_image*        proc_image;
    BPatch_function*     dely_function;
    BPatch_process*      high_process;
    PCProcess*           lowl_process;
    BPatch_snippet*      dely_snippet;
    int pid;
    
    /* creates the delay runtime snippet and assigns it to dely_snippet. */
    void create_delay_snippet(const parameters& params) {
        std::vector<BPatch_snippet*> function_arguments;
        
        // push arguments.
        function_arguments.push_back(new BPatch_constExpr(params.delay_probability));
        function_arguments.push_back(new BPatch_constExpr(params.delay_lower_bound));
        function_arguments.push_back(new BPatch_constExpr(params.delay_upper_bound));
        
        // create snippet.
        dely_snippet = new BPatch_funcCallExpr(*dely_function, 
                                                function_arguments);
        log::info() << "created delay snippet with " << params.delay_probability 
                     << "% for [" << params.delay_lower_bound << ", " 
                     << params.delay_upper_bound << "]." << std::endl;
    }
};

mutatee::mutatee(char** argv, char** envp) {    
    // initialize data.
    data = new mdata;
    data->argv = (const char**)argv;
    data->envp = (const char**)envp;
    data->addr_space = nullptr;
    data->proc_image = nullptr;
    data->high_process = nullptr;
    data->lowl_process = nullptr;
    data->dely_function = nullptr;
    data->dely_snippet = nullptr;
    
    // initialize BPatch if necessary.
    if (!bpatch_initialized) { bpatch_initializer(); }
}
    
bool mutatee::create_process(int in, int out, int err) {    
    // check if a process already exists.
    if (data->addr_space != nullptr) {
        log::debug() << "creating process over existing process." << std::endl;
        destroy_process();
    }
    
    // create new process.
    data->addr_space = bpatch.processCreate(data->argv[0], data->argv, 
                                            data->envp, in, out, err);
    
    // check we succeeded.
    if (data->addr_space == nullptr) {
        log::debug() << "could not create process for `" 
                     << data->argv[0] << "'." << std::endl;
        return false;
    }
    
    // set processes.
    data->high_process = dynamic_cast<BPatch_process*>(data->addr_space);
    data->lowl_process = data->high_process->lowlevel_process();
    
    // check we succeeded.
    if (data->lowl_process == nullptr) {
        log::debug() << "could not obtain low-level process for `"
                     << data->argv[0] << "." << std::endl;
        return false;
    }
    
    // set pid.
    data->pid = data->high_process->getPid();
    
    // set image.
    data->proc_image = data->addr_space->getImage();
    log::info() << "created new process [" << data->pid << "]." << std::endl;
    
    return true;
}

bool mutatee::destroy_process() {   
    // check that a process exists.
    if (data->addr_space == nullptr) {
        log::warn() << "attempting to destroy no process." << std::endl;
        return false;
    }
    
    data->high_process->terminateExecution();
    
    // clear variables. TODO: properly destroy these.
    data->addr_space = nullptr;
    data->proc_image = nullptr;
    data->high_process = nullptr;
    data->lowl_process = nullptr;
    data->dely_function = nullptr;
    data->dely_snippet = nullptr;
    
    log::info() << "destroyed process [" << data->pid << "]." << std::endl;
}

int mutatee::execute() {
    BPatch_process* proc = data->high_process;
    int status = 50512;
    
    // check that a process exists.
    if (data->addr_space == nullptr) {
        log::debug() << "attempting to execute no process." << std::endl;
        return EXEC_FAIL;
    }
    
    // wait while execution continues.
    while (!proc->isTerminated()) {
        bpatch.waitForStatusChange();
        if (proc->isStopped()) {
            log::info() << "process [" << data->pid << "] stopped by signal " 
                        << proc->stopSignal() << " " << proc->isStopped() << "." << std::endl;
                        
            proc->continueExecution();
            log::info() << "executing process [" << data->pid << "]." << std::endl;
        } else {
            
        }

        
        //bpatch.waitForStatusChange();
        
    }
    log::info() << "process [" << data->pid << "] has finished execution." << std::endl;
    
    // save exit status and destroy process. 
    if (proc->terminationStatus() == BPatch_exitType::ExitedNormally && proc->getExitSignal() == -1) {
        status = proc->getExitCode();
    } else {
        status = EXEC_FAIL;
        log::alert() << "process [" << data->pid << "] exited due to signal " 
                     << proc->getExitSignal() << "." << std::endl;
    }
    destroy_process();
    
    
    return status;
}

bool mutatee::find_delay_function(const std::string& library_name, 
                                  const std::string& function_name) {
    std::vector<BPatch_function*> functions;
    BPatch_module* lib;
    
    // locate the library.
    lib = data->proc_image->findModule(library_name.c_str());
    
    // check we could find the library.
    if (lib == nullptr) {
        log::debug() << "could not locate library `" << library_name 
                     << "'." << std::endl;
        return false;
    }
    
    // find the delay function in the library.
    data->proc_image->findFunction(function_name.c_str(), functions, false);
    
    // check we found functions.
    if (functions.size() == 0) {
        log::debug() << "could not locate delay function `" << function_name
                     << "'." << std::endl;
        return false;
    }
    
    // warn if we found multiple functions; we will use the first.
    if (functions.size() > 1) {
        log::warn() << "located multiple delay functions for `" << function_name
                    << "'." << std::endl;
    }
    
    // set delay function.
    data->dely_function = functions.front();
    log::info() << "found delay function `" << function_name 
                << "'." << std::endl;
    
    return true;
}

bool mutatee::instrument_memory(const parameters& params) {
    char mod_name[1024];
    std::vector<BPatch_function*>* functions = nullptr;
    std::vector<BPatch_module*>* modules = nullptr;
    std::vector<BPatch_point*>* points = nullptr;
    BPatchSnippetHandle* handle = nullptr;
    std::set<BPatch_opCode> access_type;
    std::string name;
    bool success = true;
    
    // prepare set of instruction types.
    access_type.insert(BPatch_opStore);
    
    // ensure delay snippet has been created.
    if (data->dely_snippet == nullptr) {
        data->create_delay_snippet(params);
        // TODO: should check.
    }
    
    // retrieve modules.
    modules = data->proc_image->getModules();
    
    // iterate over modules.
    std::vector<BPatch_module*>::iterator mb, me;
    mb = modules->begin();
    me = modules->end();
    while (mb != me) {
        BPatch_module* mod = *mb;
    //for (BPatch_module* mod : *modules) {
        // get name.
        mod->getName((char*)&mod_name, 1023);
        
        
        // skip module if in black list.
        if (blacklisted_module((char*)&mod_name)) { continue; }
        
        // retrieve functions in module.
        log::info() << "instrumenting module `" << (char*)mod_name 
                    << "' for memory accesses." <<std::endl;
        functions = mod->getProcedures();
        
        // check it succeeded.
        if (functions == nullptr) {
            log::debug() << "could not retrieve functions in `" << mod_name 
                         << "'." << std::endl;
            continue;
        }
        
        std::vector<BPatch_function*>::iterator fb, fe;
        fb = functions->begin();
        fe = functions->end();
        while (fb != fe) {
        //for (BPatch_function* function : *functions) {
            BPatch_function* function = *fb;
            // get function name. 
            name = function->getName();

            // find instrumentation points.
            points = function->findPoint(access_type);
            
            // check we succeeded.
            if (points == nullptr) {
                log::debug() << "could not find memory instrumentation points in `"
                             << name << "'." << std::endl;
                success = false;
                break;
            }
            
            // insert snippet.
            handle = data->addr_space->insertSnippet(*data->dely_snippet, *points);
            
            // check we succceeded.
            if (handle == nullptr) {
                log::debug() << "could not instrument memory access in `"
                             << name << "'." << std::endl;
            }
            
            // clear for loop.
            points = nullptr;
            handle = nullptr;
            ++fb;
        }
        ++mb;
    }
    
    return success;
}

bool mutatee::instrument_mutex(const parameters& params) {
    std::vector<std::string> mutex_names;
    std::vector<BPatch_function*> functions;
    std::vector<BPatch_point*>* mutex_exit_points = nullptr;
    BPatchSnippetHandle* insert_handle = nullptr;
    bool success = true;
    
    // set mutex functions. TODO: move this.
    mutex_names.push_back("__pthread_mutex_unlock");
    
    // ensure delay snippet has been created.
    if (data->dely_snippet == nullptr) {
        data->create_delay_snippet(params);
        
        // TODO: should check here.
    }
    
    // loop through functions to instrument.
    std::vector<std::string>::iterator sb, se;
    sb = mutex_names.begin();
    se = mutex_names.end();
    while (sb != se) {
    //for (const std::string& name : mutex_names) {
        const std::string& name = *sb;
        // check if we are going to instrument this function.
        if (params.get_random_int() > params.instrument_probability) {
            continue;
        }
        
        // find functions that match mutex name.
        data->proc_image->findFunction(name.c_str(), functions, false);
        
        // check we found functions.
        if (functions.size() == 0) {
            log::warn() << "could not find `" << name
                         << "'." << std::endl;
            continue;
        }
        
        // warn if we found multiple functions; we will use the first.
        if (functions.size() > 1) {
            log::warn() << "found multiple functions for `" << name
                        << "'." << std::endl;
        }
        
        // get exit points.
        mutex_exit_points = functions.front()->findPoint(BPatch_exit);
        
        // check exit points.
        if (mutex_exit_points == nullptr || mutex_exit_points->empty()) {
            log::debug() << "could not find exit points for `" << name
                         << "'." << std::endl;
            success = false;
            break;
        }
        
        // insert delay snippet.
        insert_handle = data->addr_space->insertSnippet(*data->dely_snippet,
                                                        *mutex_exit_points);
        if (insert_handle == nullptr) {
            log::debug() << "could not insert snippet for `" << name << 
                            "'." << std::endl;
            success = false;
            break;
        }
        log::info() << "inserted snippet for mutex `" 
                    << name << "'." << std::endl;
        
        // clear up for next iteration.
        functions.clear();
        mutex_exit_points = nullptr;
        insert_handle = nullptr;
        ++sb;
    }
    
    return success;
}

bool mutatee::load_runtime(const std::string& rt_name) {
    // load library into mutatee process.
    bool status = data->addr_space->loadLibrary(rt_name.c_str());
    
    // check we succeeded. 
    if (!status) {
        log::debug() << "could not load library `" << rt_name 
                     << "' into process `" << data->argv[0] 
                     << "'." << std::endl;
    } else {
        log::info() << "loaded runtime library `" 
                    << rt_name << "'." << std::endl;
    }
    return status;
}
