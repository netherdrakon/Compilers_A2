// ECE/CS 5544 S24 Assignment 2: available.cpp
// Group:

////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"
#include "available-support.h"

using namespace llvm;
using namespace std;

namespace {
  class AvailableExpressions : public FunctionPass {
    
  public:
    static char ID;
    int bbctr = 0;
    int instctr = 0;
    
    AvailableExpressions() : FunctionPass(ID) { }
    
    // virtual bool runOnFunction(Function& F) {
    bool runOnFunction(Function& F) {
      outs() << "Function: " << F.getName() << "\n";



      for(BasicBlock& B : F){
        bbctr++;
        outs() << "Basic Block: " << bbctr << "\n";
        for(Instruction& I : B){
          instctr++;
          outs() << "Basic Block: " << bbctr << "\n";
        }
        instctr=0;

      }
    

      
      
      // Here's some code to familarize you with the Expression
      // class and pretty printing code we've provided:
      
      // vector<Expression> expressions;
      // for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
	    //    BasicBlock* block = &*FI;
	    //    for (BasicBlock::iterator i = block->begin(), e = block->end(); i!=e; ++i) {
            
	    //      Instruction* I = &*i;
	    //      // We only care about available expressions for BinaryOperators
	    //      if (BinaryOperator *BI = dyn_cast<BinaryOperator>(I)) {
	           
	    //        expressions.push_back(Expression(BI));
	    //      }
	    //    }
      // }
      
      // Print out the expressions used in the function
      // outs() << "Expressions used by this function:\n";
      // printSet(&expressions);
      
      // Did not modify the incoming Function.
      return false;
    }
    
    virtual void getAnalysisUsage(AnalysisUsage& AU) const {
      AU.setPreservesAll();
    }
    
  private:
  };
  
  char AvailableExpressions::ID = 0;
  RegisterPass<AvailableExpressions> X("available",
				       "ECE/CS 5544 Available Expressions");
}
