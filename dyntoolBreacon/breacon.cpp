#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_point.h"
#include "BPatch_flowGraph.h"
#include "BPatch_module.h"
#include "InstructionCategories.h"
#include <string>
#include <string.h>
#include <set>
#include <stdlib.h>
#include <stdio.h>

bool atomicityFlag=false; 
bool raceFlag= false;

using namespace std;

BPatch bpatch;
typedef enum{create, attach, open} accessType_t;

/*Array for Function Names */
char bufferFuncName[1024];

/*Array for Call Site Function Name*/
char calledFuncName[1024];

/* handles of run-time library functions */
BPatch_function *breaconDelay;

bool createAndInsertSnippet(BPatch_addressSpace* app, std::vector<BPatch_point*>* points);


/* 
 * FindLibFunction - Find the Delay function in the Breacon Runtime library
 * */
void findLibFunction(BPatch_module *rtLibrary){

  BPatch_Vector<BPatch_function*> funcVector;
  if (rtLibrary->findFunction("breaconDelay", funcVector) == NULL){
     fprintf(stderr, "cannot find breaconDelay function in the run time library!\n");
  } else {
     breaconDelay = funcVector[0];
  }
}

// Find a point at which to insert instrumentation
std::vector<BPatch_point*>* findPoint(BPatch_addressSpace* app, const char* name, BPatch_procedureLocation loc) {
    std::vector<BPatch_function*> functions;
    std::vector<BPatch_point*>* points;
    // Scan for functions named "Name"
    BPatch_image* appImage = app->getImage();
    appImage->findFunction(name, functions);
    
    //printf("Found Function %s\n",name);
    if (functions.size() == 0) {
        fprintf(stderr, "No function %s\n", name);
        return points;
    }
    else if (functions.size() > 1) {
        fprintf(stderr, "More than one %s; using the first one\n", name);
    }
    
    // Locate the relevant points
    points = functions[0]->findPoint(loc);
    return points;
}


/**
 * Fuzz Driver - This function finds call points and initiates instrumentation
 * */
bool fuzzDriver(BPatch_addressSpace* app) {

    int err = 0;
    // Find the entry point for function and instrument
    if(atomicityFlag) {
        const char* funcName = "__pthread_mutex_unlock";
        std::vector<BPatch_point*>* exitPoint =  findPoint(app, funcName, BPatch_entry);
        if (!exitPoint || exitPoint->size() == 0) {
            fprintf(stderr, "No entry points for %s\n", funcName);
            atomicityFlag = false;
        }
        // Create and insert instrumentation snippet
        if (atomicityFlag && !createAndInsertSnippet(app, exitPoint)) {
                fprintf(stderr, "createAndInsertSnippet in %s failed\n",funcName);
                exit(1);
       }
        
    }
    if(raceFlag) {
        //TODO:Instrument memory Accesses and Race condition algorithms
        /*
        const char* funcName = "";
        std::vector<BPatch_point*>* exitPoint =  findPoint(app, funcName, BPatch_entry);
        if (!exitPoint || exitPoint->size() == 0) {
            fprintf(stderr, "No entry points for %s\n", funcName);
            exit(1);
        }
        // Create and insert instrumentation snippet
        if (!createAndInsertSnippet(app, exitPoint)) {
            fprintf(stderr, "createAndInsertSnippet in %s failed\n",funcName);
            exit(1);
        }
        */
    }
    return err;
    
}


/**
 * Snippet from Dyninst API Manual - Starting instrumentation
 * */
BPatch_addressSpace *startInstrumenting(accessType_t accessType, const char **argv, const char** envp){
    BPatch_addressSpace *handle = NULL;
    switch(accessType){
        case create:
            handle = bpatch.processCreate(argv[1], argv+1,envp);
            break;
        case attach:
            fprintf(stderr, "Attach not supported\n");
            //handle = bpatch.processAttach(name, pid);
            break;
        case open:
            //handle = bpatch.openBinary(name, true);
            break;
    }
    return handle;
}


/*
 * Finish the Instrumentation
 */
void finishInstrumenting(BPatch_addressSpace* app, const char* newName)
{
    BPatch_process* appProc = dynamic_cast<BPatch_process*>(app);
    BPatch_binaryEdit* appBin = dynamic_cast<BPatch_binaryEdit*>(app);
    if (appProc) {
        if (!appProc->continueExecution()) {
            fprintf(stderr, "continueExecution failed\n");
        }
        while (!appProc->isTerminated()) {
            bpatch.waitForStatusChange();
        }
    }
    else if(appBin) {
        if(!appBin->writeFile(newName)) {
            fprintf(stderr,"WriteFile failed\n");
        }
    }
}


/*Creates and Inserts Snippet at the point passed in the argument
 * */
bool createAndInsertSnippet(BPatch_addressSpace* app, std::vector<BPatch_point*>* points) {

    vector<BPatch_snippet*> args;
    BPatch_snippet *fmt = new BPatch_constExpr(rand()%100);
    args.push_back(fmt);
    if(!app->insertSnippet(BPatch_funcCallExpr(*breaconDelay, args), *points)){
        fprintf(stderr, "insertSnippet failed\n");
        return false;
    }
    return true;
}


/**
 * Main Function - Breacon.cpp
 * */

int main(const int argc, const char** argv, const char** envp){   
    //Adding option support so that we only instrument what is asked by the user
    int argCount;
    int err = 0;
    if (argc > 5 || argc <=2) {
        printf("Breacon - Randomizing the Thread Scheduler\nUsage: %s <Pthread-Executable> [Options]\nOptions:\n-a => Atomicity Violation based Instrumentation\n-r => Race Condition based Instrumentation\n-all => Full Instrumentation (If you like it slow)\nPress[CTRL-C] to stop\n", argv[0]);
        exit(0);
    }

    for(argCount=2;argCount<argc;argCount++){
        if(!strcmp(argv[argCount],"-a")) {
            atomicityFlag = true;
        }
        else if(!strcmp(argv[argCount],"-r")) {
            raceFlag = true;
        }
        else if(!strcmp(argv[argCount],"-all")) {
            atomicityFlag = true;
            raceFlag = true;
        }
        else {
            printf("Invalid argument %s exiting...\n",argv[argCount]);
            exit(0);
        }
    }

    /*Initializing a PRG for future RAND calls in the RT API
     * The Numbers are passed to the library function for every invocation of the breacon program
     */
    unsigned int seed;
    FILE *fp = fopen("/dev/urandom", "r");
    fread((unsigned int*)(&seed),sizeof(seed), 1, fp);
    fclose(fp);
    printf("The seed is %u\n",seed);
    srand(seed);
    
    /*We need to run forever...press CTRL-C to exit*/
    
        /*Initializing the PRG with the runCount for easy reproducibility*/
        try {
            /*Creating and then attaching to the process. For debugging need to consider Binary Rewriting too*/
             BPatch_addressSpace *app = startInstrumenting(create,  argv,  envp);
             if(!app) {
                 printf("Error: startInstrumenting failed\n");
                 exit(0);
             }
                    
             /*Dyninst provides a loadlibrary interface :)*/
             char *RTLibpath="libbreacondelay.so";
             if(!app->loadLibrary(RTLibpath)) {
                 printf("Open Library Failed \n");
             }
        
             /*img - Handle to the executable file*/
             BPatch_image *img = app->getImage();

             /*Get Handle to the library functions in Advance*/
             BPatch_module *rtLibrary = img->findModule("libbreacondelay.so");
             if (rtLibrary == NULL){
                 printf("cannot find run time library module\n");
            }
             findLibFunction(rtLibrary);

            /*
             //funcVector - Vector containing the functions in the program
             const BPatch_Vector<BPatch_function*> *funcVector = img->getProcedures();
             for (BPatch_Vector<BPatch_function*>::const_iterator iterFuncVector = funcVector->begin(); iterFuncVector != funcVector->end(); ++iterFuncVector) {
                 (*iterFuncVector)->getName(bufferFuncName,1023);
                 //Some functions we can comfortably ignore
                 if (!strncmp(bufferFuncName, "std::",5)) continue;
                 if (!strncmp(bufferFuncName, "_",1)) continue;
                 if (!strncmp(bufferFuncName, "new(", 4)) continue;
                 if (!strncmp(bufferFuncName, "call_", 5)) continue;
                 if (!strncmp(bufferFuncName, "global con", 10)) continue;
                 if (!strcmp(bufferFuncName, "frame_dummy")) continue;
        
                 //Analyzing each function for Pthread calls and instrumenting after them
                 //printf("%s\n", bufferFuncName);
                 if (strcmp(bufferFuncName, "__pthread_mutex_unlock")) {
                    fuzzDriver(app, *iterFuncVector, bufferFuncName);
                 }
             }*/
          
            // Let the Randomization begin!
            err = fuzzDriver(app);
            if(!err) {
                finishInstrumenting(app,"JustForFun");
            }
            else {
                printf("Failed: Fuzz Driver has failed to his job\n");
            }


        }
        catch (exception& e) {
            printf("Exception occurred: %s",e.what());
        }
    return 0;
}
