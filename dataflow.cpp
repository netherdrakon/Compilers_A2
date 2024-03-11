
////////////////////////////////////////////////////////////////////////////////

#include "dataflow.h"

namespace llvm {

  BitVector DataFlow::meet(std::vector<BitVector>& inputarray, bool union_flag){
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
        result = result | inputarray[i];
      }
      else{
        result = result & inputarray[i];
      }
    }

    return result;

  }



  // Add code for your dataflow abstraction here.
}
