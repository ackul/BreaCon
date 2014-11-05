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
using namespace std;

BPatch bpatch;
/* Dyninst provides three types of instrumentation.
   --create: dynamicly create a process of the mutatee program;
   --attach: dynamicly attach to a running process of the mutatee;
   --pen: staticly open the mutate program;
*/
typedef enum{create, attach, open} accessType_t;

/*Array for Function Names */
char bufferFuncName[1024];

/*Array for Call Site Function Name*/
char calledFuncName[1024];

/* handles of run-time library functions */
BPatch_function *breaconDelay;


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



/**
 * Fuzz Driver - This function finds call points and initiates instrumentation
 * */
void fuzzDriver(BPatch_addressSpace* app, BPatch_function *func, char *funcName) {

    /*Get all call instructions in the Function*/
    const BPatch_Vector<BPatch_point *>* callPoints = func->findPoint(BPatch_subroutine);

    /*Call site Enumeration and Display*/
    for (BPatch_Vector<BPatch_point*>::const_iterator iter = callPoints->begin(); iter != callPoints->end(); ++iter){
        BPatch_function* calledFuncs = (*iter)->getCalledFunction();
        if (calledFuncs != NULL){
            calledFuncs->getName(calledFuncName , 1023);
            //printf("In function %s, %s is called\n", funcName, calledFuncName);
            if (!strcmp(calledFuncName,"__pthread_mutex_unlock")) {
                printf("Found Unlock\n");
                vector<BPatch_snippet*> args;
                BPatch_snippet *fmt = new BPatch_constExpr(rand()%100);
                args.push_back(fmt);
                app->insertSnippet(BPatch_funcCallExpr(*breaconDelay, args), **iter, BPatch_callAfter);
            }
        }
    }
}



/**
 * Snippet from Dyninst API Manual - Starting instrumentation
 * */
BPatch_addressSpace *startInstrumenting(accessType_t accessType, const char *name, int pid){
    BPatch_addressSpace *handle = NULL;
    switch(accessType){
        case create:
            handle = bpatch.processCreate(name, NULL);
            break;
        case attach:
            handle = bpatch.processAttach(name, pid);
            break;
        case open:
            handle = bpatch.openBinary(name, true);
            break;
    }
    return handle;
}


/*
 * Finish the Instrumentation
 */
void finishInstrumenting(BPatch_addressSpace* app)
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
}




/**
 * Main Function - Breacon.cpp
 * */

int main(const int argc, const char** argv){   
    if (argc <= 1) {
        printf("Usage: %s <Pthread-Executable> (Press [CTRL-C] to Stop)\n", argv[0]);     
        exit(0);
    }
    /*Initializing a PRG for future RAND calls in the RT API
     * The Numbers are passed to the library function for every invocation
     */
    unsigned int seed;
    FILE *fp = fopen("/dev/urandom", "r");
    fread((unsigned int*)(&seed),sizeof(seed), 1, fp);
    fclose(fp);
    printf("The seed is %u\n",seed);
    srand(seed);
    
    /*We need to run forever...press CTRL-C to exit*/
    while(1){
        /*Initializing the PRG with the runCount for easy reproducibility*/
        try {
            /*Creating and then attaching to the process. For debugging need to consider Binary Rewriting too*/
             BPatch_addressSpace *app = startInstrumenting(create,  argv[1],  0);
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
             const BPatch_Vector<BPatch_module*> *funcVectorMod = img->getModules();
             for (BPatch_Vector<BPatch_module*>::const_iterator iterFuncVector = funcVectorMod->begin(); iterFuncVector != funcVectorMod->end(); ++iterFuncVector) {
                 (*iterFuncVector)->getName(bufferFuncName,1023);
                
                 printf("%s\n", bufferFuncName);
             }
            */

             /*funcVector - Vector containing the functions in the program*/
             const BPatch_Vector<BPatch_function*> *funcVector = img->getProcedures();
             for (BPatch_Vector<BPatch_function*>::const_iterator iterFuncVector = funcVector->begin(); iterFuncVector != funcVector->end(); ++iterFuncVector) {
                 (*iterFuncVector)->getName(bufferFuncName,1023);
                 /*Some functions we can comfortably ignore*/
                 if (!strncmp(bufferFuncName, "std::",5)) continue;
                 if (!strncmp(bufferFuncName, "_",1)) continue;
                 if (!strncmp(bufferFuncName, "new(", 4)) continue;
                 if (!strncmp(bufferFuncName, "call_", 5)) continue;
                 if (!strncmp(bufferFuncName, "global con", 10)) continue;
                 if (!strcmp(bufferFuncName, "frame_dummy")) continue;
        
                 /*Analyzing each function for Pthread calls and instrumenting after them*/
                 //printf("%s\n", bufferFuncName);
                 fuzzDriver(app, *iterFuncVector, bufferFuncName);
             }
             finishInstrumenting(app);
         
         
         }
        catch (exception& e) {
           printf("Exception occurred: %s",e.what());
        }
    }
    return 0;
}
