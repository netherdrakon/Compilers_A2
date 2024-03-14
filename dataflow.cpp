#include "dataflow.h"

using namespace std;
namespace llvm 
{
    BitVector DataFlow::meet(vector<BitVector>& inputarray)
    {
        BitVector result;
        int inputsize = inputarray.size();

        if(inputsize<1){
            return result;
        }
        else if (inputsize == 1){
            return inputarray[0];
        }

        result = inputarray[0];

            for (int i = 1; i < inputsize; i++) 
            {

                if (meet_op == MeetOperator::UNION) 
                {
                    result |= inputarray[i];
                }
                else if (meet_op == MeetOperator::INTERSECTION) 
                {
                    result &= inputarray[i];
                }

            }
        return result;

    }

    //Initializes an index map from Expressions/Variables etc (depending on analysis) to index.
    unordered_map<void*, int> DataFlow::InitializeExprMap(std::vector<void*> &domain){

        unordered_map<void*, int> res;

        for (int i = 0; i < domain.size(); i++){
            res[(void*)domain[i]] = i;
        }

        return res;
        

    }

    //Initializes the boundary depending on forward or back propagation
    vector<BasicBlock*> DataFlow::InitializeBoundary(Function &F){
        vector<BasicBlock*> boundary;
        if (direction == Direction::FORWARD){
            boundary.push_back(&F.front());
            
        }
        else{
            for(Function::iterator I = F.begin(), E = F.end(); I != E; ++I){
                if (isa<ReturnInst>(I->getTerminator())){
                        boundary.push_back(&*I);   
                }
            }
        }
        return boundary;
    }

    // BitVector* DataFlow::InitInOutVals(BlockResult &boundary_result, BitVector boundary_condition, bool flag){
    //     BitVector* value;
    //     if (direction == Direction::FORWARD){
    //         if(!flag){
    //             value = &boundary_result.in;
    //         }
    //         else{
    //             value = &boundary_result.out;
    //         }
    //     }
    //     else{
    //         if(!flag){
    //             value = &boundary_result.out;
    //         }
    //         else{
    //             value = &boundary_result.in;
    //         }
    //     }

    //     BitVector* value = AssignValuePtr(boundary_result, flag);

    //     *value = boundary_condition; 
    //     boundary_result.tfOutput.tfResult = boundary_condition;

    //     return value;
    // }

    //Traverses the Function to get the Basic Blocks in required order to perform the meet operation
    vector<BasicBlock*> DataFlow::Traverse(Function &F){
        vector<BasicBlock*> result;// Initialising a vector to store the order of traversal
        if(direction == Direction::FORWARD)
        {
            ReversePostOrderTraversal<Function*> rev_traverse(&F);
            for (ReversePostOrderTraversal<Function*>::rpo_iterator I = rev_traverse.begin(), E = rev_traverse.end(); I != E; ++I) 
            {
                result.push_back(*I); // Storing basic blocks in forward propagation analysis in reverse post order tranversal
            }
        }
        else if(direction == Direction::BACKWARD)
        {
            for (po_iterator<BasicBlock*> I = po_begin(&F.getEntryBlock()), E = po_end(&F.getEntryBlock()); I != E; ++I) 
            {
                result.push_back(*I); // Storing basic blocks in back propagation analysis in post order tranversal
            }
        }

        return result;
    }

    //Helper function to simplify assigning values based on direction
    BitVector DataFlow::AssignValue(BlockResult &block_result, bool flag){
        BitVector value;
        if (direction == Direction::FORWARD) 
        {
            if(!flag){
                value = block_result.out;
            }
            else{
                value = block_result.in;
            }
            
        } 
        else 
        {
            if(!flag){
                value = block_result.in;
            }
            else{
                value = block_result.out;
            }
            
        }
        return value;

    }

    //Another helper function for pointers
    BitVector* DataFlow::AssignValuePtr(BlockResult &block_result, bool flag){
        BitVector* value;
        if (direction == Direction::FORWARD) 
        {
            if(flag){
                value = &block_result.out; // if Direction is Forward, then in block is intialised
            }
            else{
                value = &block_result.in;
            }
            
        } 
        else 
        {
            if(flag){
                value = &block_result.in; // if Direction is Backward, then out block is intialised
            }
            else{
                value = &block_result.out;
            }
            
        }
        return value;

    }

    //Actual driver code
    DataFlowResult DataFlow::run(Function &F, std::vector<void*> domain,BitVector boundary_condition, BitVector initial_interior_point) 
    {
        std::unordered_map<BasicBlock*, BlockResult> output;// Initialising the output to store the result of the analysis
        bool modified = false;// Initialising the variable to check if there is any modification to the in and out of the basic block

        std::unordered_map<void*, int> ExprMap = InitializeExprMap(domain);


        vector<BasicBlock*> boundary = InitializeBoundary(F);


        BlockResult boundary_result = BlockResult();
        BlockResult block_init_result = BlockResult(); 


        BitVector* value = AssignValuePtr(boundary_result, 0);
        *value = boundary_condition;
        boundary_result.tfOutput.tfResult = boundary_condition;

        BitVector* initial_value = AssignValuePtr(block_init_result, 1);
        *initial_value = initial_interior_point; 
        block_init_result.tfOutput.tfResult = initial_interior_point;


        for (vector<BasicBlock*>::iterator I = boundary.begin(), E = boundary.end(); I != E; ++I) 
        {
            output[*I] = boundary_result;
        }

        
        for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; ++BB) 
        {
            BasicBlock* block = &*BB;
            if (std::find(boundary.begin(),boundary.end(), block) == boundary.end()) 
            {
                output[block] = block_init_result;// Storing the results after checking if it is not a boundary block in the output vector
            }
        }


        unordered_map<BasicBlock*, vector<BasicBlock*>> next_block;// Initialising the map to store the blocks leading to the basic block or leading from the basic block based on the direction of analysis
        for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; ++BB) 
        {
            vector<BasicBlock*> next_block_list;
            BasicBlock* block = &*BB;
            switch (direction) 
            {
                case Direction::FORWARD:
                    for (pred_iterator pi = pred_begin(block), E = pred_end(block); pi != E; ++pi)
                        next_block_list.push_back(*pi);// if Direction is Forward, then the predecessors are stored 

                    break;
                case Direction::BACKWARD:
                    for (succ_iterator si = succ_begin(block), E = succ_end(block); si != E; ++si)
                        next_block_list.push_back(*si); // if Direction is Backward, then the sucessors are stored
                    break;
                default:
                    break;
            }
            next_block[block] = next_block_list;
        }


        vector<BasicBlock*> traverse = Traverse(F);


        bool fixedpoint = false;
        while (!fixedpoint) // Checking for convergence if old_in = new_in and old_out = new_out
        {
            // fixedpoint = true;
            for (vector<BasicBlock*>::iterator BB = traverse.begin(), BE = traverse.end(); BB != BE; ++BB) 
            {
                BlockResult& block_result = output[*BB]; // Current state of the variables are stored in block_result
                BlockResult old_block_result = block_result; // Current state stored in old_block_result 
                
                BitVector value_old = AssignValue(old_block_result, 0);


                vector<BitVector> meet_inputs; // Bitvector initialisation to perform the meet operator
                for (vector<BasicBlock*>::iterator NB = next_block[*BB].begin(), NB_End = next_block[*BB].end(); NB != NB_End; ++NB) 
                {
                    BlockResult& next_block_result = output[*NB];
                    BitVector next_value = next_block_result.tfOutput.tfResult;
                    std::unordered_map<BasicBlock*, BitVector>::iterator check = next_block_result.tfOutput.next_values.find(*BB);
                    if (check != next_block_result.tfOutput.next_values.end()) 
                    {
                        next_value |= check->second;// Checking if there are multiple predecessors and performing union operation on them
                    }
                    meet_inputs.push_back(next_value);
                }

                BitVector* blockInput = AssignValuePtr(block_result, 0);

                if (!meet_inputs.empty())
                    *blockInput = meet(meet_inputs);
                block_result.tfOutput = TF(*blockInput, domain, ExprMap, *BB);

                BitVector* blockOutput = AssignValuePtr(block_result, 1);

                *blockOutput = block_result.tfOutput.tfResult;
                // if (fixedpoint) // If there is a change in the in or out values, fixed point is set as false and the iteration loop is entered again
                // {
                    // if (*blockOutput != value_old ||block_result.tfOutput.next_values.size() != old_block_result.tfOutput.next_values.size())
                if (*blockOutput != value_old ||block_result.tfOutput.next_values.size() != old_block_result.tfOutput.next_values.size()){

                    fixedpoint = true;
                }
                // }
            }
        }
        DataFlowResult result; // Output is intialised and updated
        result.ExprMap = ExprMap;
        result.results = output;
        result.modified = modified;

        return result;
    }


}
