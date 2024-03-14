
////////////////////////////////////////////////////////////////////////////////

#ifndef __CLASSICAL_DATAFLOW_H__
#define __CLASSICAL_DATAFLOW_H__

#include <stdio.h>
#include <iostream>
#include <queue>
#include <vector>


#include <unordered_map>
#include <string>
#include <iostream>

#include "llvm/IR/Instructions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/CFG.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/PostOrderIterator.h"


#include "available-support.h"

using namespace std;

namespace llvm
{
	enum MeetOperator
	{
		UNION,
	    INTERSECTION
	};
	enum Direction
	{	        	
		FORWARD,
    	BACKWARD	    	
	};
	struct transferVar // Includes the bit vectors that are inputs and the map of output sets of the block after the corresponding transfer function
	{
	    BitVector tfResult;
    	std::unordered_map<BasicBlock*, BitVector> next_values;
   	};
	struct BlockResult // Includes the bit vectors that are input and output sets of the block with the corresponding transfer function
	{
       	BitVector in;
      	BitVector out;
       	transferVar tfOutput;
   	};
	struct DataFlowResult // Includes the bitvector of the output result of the analysis, the variable value with its index in the doain and also the variable that indicates if the input values have been modified 
	{
		std::unordered_map<BasicBlock*, BlockResult> results;
		std::unordered_map<void*, int> ExprMap;
		bool modified;
	};
	class DataFlow // Includes the bit vector of meet operation, the set of all the generated variables and also the variables that are killed in the basic block. The method run is used to do the analysis
	{
		public:

	    	DataFlow(Direction direction, MeetOperator meet_op): direction(direction), meet_op(meet_op){

			}

	        DataFlowResult run(Function &F, vector<void*> domain,BitVector boundary_conditions, BitVector initial_interior_point);
			BitVector meet(vector<BitVector>& inputs);
			unordered_map<void*, int> InitializeExprMap(vector<void*> &domain);
			vector<BasicBlock*> InitializeBoundary(Function &F);
			// BitVector* InitInOutVals(BlockResult &boundary_result, BitVector boundary_condition, bool flag);
			vector<BasicBlock*> Traverse(Function &F);
			BitVector AssignValue(BlockResult &block_result, bool flag);
			BitVector* AssignValuePtr(BlockResult &block_result, bool flag);


	        std::unordered_map<BasicBlock*, BitVector> gen_set;
	        std::unordered_map<BasicBlock*, BitVector> kill_set;

		protected:
		virtual	transferVar TF(BitVector value,std::vector<void*> domain,std::unordered_map<void*, int> ExprMap, BasicBlock* BB) = 0;
		 private:
			Direction direction;
			MeetOperator meet_op;
	};
}
#endif
