
////////////////////////////////////////////////////////////////////////////////

#include "dataflow.h"
using namespace std;

namespace llvm {

  BitVector DataFlow::meet(vector<BitVector>& inputarray, bool union_flag){
    BitVector result;
    int inputsize = inputarray.size();
    if(inputsize==0){
      return result;
    }
    else if(inputsize==1){
      return inputarray[0];
    }
    result = inputarray[0];

    for(int i=1; i<inputsize; i++){
      //Meet Flag: Union = 1, Intersection = 0
      if(union_flag){
        result |= inputarray[i];
      }
      else{
        result &= inputarray[i];
      }
    }

    return result;

  }

  void DataFlow::InitializeExpr(Function &F){
    bool expr_exists = false;

    for (BasicBlock& BB : F){
      for (Instruction& I : BB){
        
        if(BinaryOperator * BI = dyn_cast<BinaryOperator>(&I)) {

          Expression *temp = new Expression(BI);

          for(int i = 0; i<ExprList.size(); i++){
            if(*temp == *ExprList[i]){
              expr_exists = 1;
              break;
            }
          }

          if(!expr_exists){
            ExprList.push_back(temp);
          }
          else{
            delete temp;
          }

        }
      }
    }
  }

  void DataFlow::CustomInit(Function &F){

    

  }


  // Add code for your dataflow abstraction here.
}
