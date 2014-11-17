#include "log.h"
#include "mutatee.h"

#include <BPatch.h>
#include <BPatch_addressSpace.h>
#include <BPatch_function.h>
#include <BPatch_image.h> // may not need
#include <BPatch_module.h>
#include <BPatch_point.h>
#include <BPatch_process.h>

// global variable to represent dyninst library. 
static BPatch bpatch;

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

/* structure to hold mutatee data and hide implementation from other files. */
struct mutatee::mdata {
    const char** argv;
    const char** envp;
    BPatch_addressSpace* addr_space;
    BPatch_image*        proc_image;
    BPatch_function*     dely_function;
    BPatch_snippet*      dely_snippet;
    
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
    data = new mdata;
    data->argv = (const char**)argv;
    data->envp = (const char**)envp;
    data->addr_space = nullptr;
    data->proc_image = nullptr;
    data->dely_function = nullptr;
    data->dely_snippet = nullptr;
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
        log::debug() << "could not create process `" 
                     << data->argv[0] << "'." << std::endl;
        return false;
    }
    
    // set image.
    data->proc_image = data->addr_space->getImage();
    log::info() << "created new process `" << data->argv[0] << "'." << std::endl;
    
    return true;
}

bool mutatee::destroy_process() {
    BPatch_process* proc;
    
    // check that a process exists.
    if (data->addr_space == nullptr) {
        log::warn() << "attempting to destroy no process." << std::endl;
        return false;
    }
    
    // get process.
    proc = dynamic_cast<BPatch_process*>(data->addr_space);
    proc->terminateExecution();
    
    // clear variables. TODO: properly destroy these.
    data->addr_space = nullptr;
    data->proc_image = nullptr;
    data->dely_function = nullptr;
    data->dely_snippet = nullptr;
    
    log::info() << "destroyed process `" << data->argv[0] << "'." << std::endl;
}

int mutatee::execute() {
    BPatch_process* proc;
    int status;
    
    // check that a process exists.
    if (data->addr_space == nullptr) {
        log::debug() << "attempting to execute no process." << std::endl;
        return EXEC_FAIL;
    }
    
    // get process.
    proc = dynamic_cast<BPatch_process*>(data->addr_space);
    
    // wait while execution continues.
    while (!proc->isTerminated()) {
        proc->continueExecution();
        log::info() << "executing process." << std::endl;
        bpatch.waitForStatusChange();
        log::info() << "status change in process." << std::endl;
    }
    log::info() << "process has finished execution." << std::endl;
    
    // save exit status and destroy process. 
    status = proc->getExitCode();
    destroy_process();
    
    if (status == EXEC_FAIL) {
        log::debug() << "exit status is EXEC_FAIL." << std::endl;
    }
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
    const std::vector<BPatch_function*>* functions = nullptr;
    const std::vector<BPatch_module*>* modules = nullptr;
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
    for (BPatch_module* mod : *modules) {
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
        
        for (BPatch_function* function : *functions) {
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
        }
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
    for (const std::string& name : mutex_names) {
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
