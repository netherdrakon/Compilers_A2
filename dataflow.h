
////////////////////////////////////////////////////////////////////////////////

#ifndef __CLASSICAL_DATAFLOW_H__
#define __CLASSICAL_DATAFLOW_H__

#include <stdio.h>
#include <iostream>
#include <queue>
#include <vector>

#include "llvm/IR/Instructions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/CFG.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/PostOrderIterator.h"
#include <algorithm>
#include <stack>
#include <map>
#include <string>

#include "available-support.h"

using namespace std;

namespace llvm {

// Add definitions (and code, depending on your strategy) for your dataflow
// abstraction here:

    class DataFlow : public FunctionPass{
        
        public:

            vector<Expression* > ExprList;
            unordered_map<Expression* , int> ExprIndex;

            bool backward_flag = 0; //Forward = 0, Backward = 1
            // bool union_flag = 0; //Union = 0, Intersection = 1

            
            BitVector meet(std::vector<BitVector>& inputarray, bool union_flag);

            void InitializeExpr(Function &F);

            virtual void CustomInit(Function &F);

    
    };

    typedef struct BlockData{
        BlockData(int size){
            BitVector in[size];
            BitVector out[size];
        }
    }BlockData;

    

}

#endif
