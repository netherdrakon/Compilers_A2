
////////////////////////////////////////////////////////////////////////////////

#include "dataflow.h"

namespace llvm {

  BitVector DataFlow::meet(std::vector<BitVector>& inputarray){
    BitVector result;
    int inputsize = input.size();
    if(inputsize==0){
      return result;
    }
    else if(inputsize==1){
      return inputarray[0];
    }
    result = inputarray[0];

    for(int i=1; i<inputsize; i++){
      //Meet Flag: Union = 0, Intersection = 1
      if(!meet_flag){
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
