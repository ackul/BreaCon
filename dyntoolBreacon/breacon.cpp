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

// Find a point at which to insert instrumentation
std::vector<BPatch_point*>* findPoint(BPatch_addressSpace* app, const char* name, BPatch_procedureLocation loc) {
 std::vector<BPatch_function*> functions;
 std::vector<BPatch_point*>* points;
 // Scan for functions named "name"
 BPatch_image* appImage = app->getImage();
 appImage->findFunction(name, functions);

 printf("Found Function %s\n",name);
 if (functions.size() == 0) {
 fprintf(stderr, "No function %s\n", name);
 return points;
 } else if (functions.size() > 1) {
 fprintf(stderr, "More than one %s; using the first one\n", name);
 }
 // Locate the relevant points
 points = functions[0]->findPoint(loc);
 return points;
}


/**
 * Fuzz Driver - This function finds call points and initiates instrumentation
 * */
void fuzzDriver(BPatch_addressSpace* app, BPatch_function *func, char *funcName) {





/*

    //Get all call instructions in the Function
    const BPatch_Vector<BPatch_point *>* callPoints = func->findPoint(BPatch_subroutine);

   //Call site Enumeration and Display
    for (BPatch_Vector<BPatch_point*>::const_iterator iter = callPoints->begin(); iter != callPoints->end(); ++iter){
        BPatch_function* calledFuncs = (*iter)->getCalledFunction();
        if (calledFuncs != NULL){
            calledFuncs->getName(calledFuncName , 1023);
            //printf("In function %s, %s is called\n", funcName, calledFuncName);
            if (!strcmp(calledFuncName,"__pthread_mutex_unlock")) {
                printf("In function %s, %s is called\n", funcName, calledFuncName);
                vector<BPatch_snippet*> args;
                BPatch_snippet *fmt = new BPatch_constExpr(rand()%100);
                args.push_back(fmt);
                app->insertSnippet(BPatch_funcCallExpr(*breaconDelay, args), **iter, BPatch_callAfter);
            }
         if (!strcmp(calledFuncName,"pthread_create")) {
                printf("In function %s, %s is called\n", funcName, calledFuncName);
                vector<BPatch_snippet*> args;
                BPatch_snippet *fmt = new BPatch_constExpr(rand()%100);
                args.push_back(fmt);
                app->insertSnippet(BPatch_funcCallExpr(*breaconDelay, args), **iter, BPatch_callAfter);
            }

        }
    }
*/
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
             const BPatch_Vector<BPatch_module*> *funcVectorMod = img->getModules();
             for (BPatch_Vector<BPatch_module*>::const_iterator iterFuncVector = funcVectorMod->begin(); iterFuncVector != funcVectorMod->end(); ++iterFuncVector) {
                 (*iterFuncVector)->getName(bufferFuncName,1023);
                
                 printf("%s\n", bufferFuncName);
             }
            */
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
          
// Find the entry point for function InterestingProcedure
 const char* interestingFuncName = "__pthread_mutex_unlock";
 std::vector<BPatch_point*>* exitPoint =  findPoint(app, interestingFuncName, BPatch_entry);
 if (!exitPoint || exitPoint->size() == 0) {
 fprintf(stderr, "No entry points for %s\n", interestingFuncName);
exit(1);
}
// Create and insert instrumentation snippet 2
  if (!createAndInsertSnippet(app, exitPoint)) {
   fprintf(stderr, "createAndInsertSnippet2 failed\n");
    exit(1);
     }

  finishInstrumenting(app,"instrumentationName");
         
         
         }
        catch (exception& e) {
           printf("Exception occurred: %s",e.what());
        }
    
    return 0;
}
