// ECE/CS 5544 S24 Assignment 2: liveness.cpp
// Group: Akhilesh Marathe, Alexander Owens

////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"

using namespace llvm;

namespace {

      class LivenessAnalysis : public DataFlow
    {
    public:
        LivenessAnalysis(Direction direction, MeetOperator meet_op) : DataFlow(direction, meet_op) { }
    protected:
        transferVar TF(BitVector value,  std::vector<void*> domain, std::unordered_map<void*, int> domainVal_index, BasicBlock* BB)
        {

            transferVar transfer;
            int domain_size = domainVal_index.size();
            BitVector def_set(domain_size);
            BitVector use_set(domain_size);
            for (Instruction &I: *BB)
            {
              if (PHINode* phi_ins = dyn_cast<PHINode>(&I)) // checking if the current instruction is a phi nodeinstruction
              {
                for (int i = 0; i < phi_ins->getNumIncomingValues(); i++) //loops over the number of operands in the phinode instructions
                {
                  Value* value = phi_ins->getIncomingValue(i);
                  if (isa<Instruction>(value) || isa<Argument>(value)) // checking if the value is an instructions is an instruction or an argument
                  {   
                      int value_index = domainVal_index[(void*)value]; // value_index is initialised to the index in the domain value index map
                      BasicBlock* value_block = phi_ins->getIncomingBlock(i); // value_block is initialised to the Basic Block
                      if (transfer.next_values.find(value_block) == transfer.next_values.end()) // checking if the basic block contains the bit vector in the upcoming flow path
                      {
                          transfer.next_values[value_block] = BitVector(domain_size);// If not, adds a bit vector to that block
                      }
                      transfer.next_values[value_block].set(value_index);// updates the bit vector to index of the updated value of the index

                  }
                }
              }
		        else
		        {
              for (Use& operand : I.operands()) 
              {
                Value* value = operand.get();
                if (isa<Instruction>(value) || isa<Argument>(value)) // checking if the valueis an instructions is an instruction or an argument
                {
                    int value_index = domainVal_index[value];// value_index is initialised to the index in the domain value index map
                    if (!def_set[value_index]) // Checking if the value index in the def set bit vector is set 
                    {
                        use_set.set(value_index);// The value index bit vector is set in use set
                    }
                }
              }
            }
                auto i = domainVal_index.find(static_cast<void*>(&I)); // checking if i is there in domainVal_index
                if (i != domainVal_index.end()) // If i is found in domainVal_index
                {
                    def_set.set((*i).second);// i th bit in def_set is set
                }
            }
            if(gen_set.find(BB) == gen_set.end()) //updating the genSet for each block.
            {
                gen_set[BB] = use_set;
            }
	        if(kill_set.find(BB) == kill_set.end())//updating the killSet for each block.
		    {
                kill_set[BB] = def_set;
            } // Transfer function =(input - def_set) U use_set
		    transfer.tfResult = def_set;
            transfer.tfResult.flip();// Complement of def_set
            transfer.tfResult &= value; // input - def_set
            transfer.tfResult |= use_set;
            return transfer;
        }

    };

  class Liveness : public FunctionPass {
  public:
    static char ID;

    Liveness() : FunctionPass(ID) { }

    virtual void getAnalysisUsage(AnalysisUsage& AU) const {
      AU.setPreservesAll();
    }

  private:
    bool runOnFunction(Function &F)
      {
        outs() << "FUNCTION :: " << F.getName()  << "\n";
        std::vector<void*> domain; // Initialising the domain vector
        for(auto& I :instructions(F)) // Iterating through the instructions in the function
          {
            for (Use&U : I.operands()) // Iterating through the use objects of the operand
            {
                Value *value = U.get();
                if (isa<Instruction>(value) || isa<Argument>(value)) // checking if the valueis an instructions is an instruction or an argument
                {
                    if(std::count(domain.begin(),domain.end(),value) == 0) // checking if the instruction or the argument is alrady there in the domain
                    {
                        domain.push_back((void*)value); // adding the instruction or the argument in the domain if its not already there
                    }
                }
            }
        }
        BitVector boundary_conditions(domain.size(), false); //The boundary_condition is set to phi
        BitVector initial_interior_point(domain.size(), false); //initial condition is set of all variables
        LivenessAnalysis pass(Direction::BACKWARD, MeetOperator::UNION); // The Pass
        DataFlowResult output = pass.run(F, domain, boundary_conditions, initial_interior_point);// Apply pass
        for (Function::iterator BI = F.begin(), BE = F.end(); BI != BE; ++BI) //Print the output for iterating over each basic block.
        {
            BasicBlock* block = &*BI;
            outs() << "[BB Name]"<< block->getName() << "\n";
            outs() << "IN ["<<block->getName() <<"]:"<<  formatSet(domain, output.results[block].in, 0) << "\n";
            outs() << "OUT["<<block->getName() <<"]:"<<  formatSet(domain, output.results[block].out, 0) << "\n";
            outs() << "Use["<<block->getName() <<"]:"<<   formatSet(domain, pass.gen_set[block], 0) << "\n";
            outs() << "Def["<<block->getName() <<"]:"<<   formatSet(domain, pass.kill_set[block], 0) << "\n\n";
        }
        return false;
      }
  };

  char Liveness::ID = 0;
  RegisterPass<Liveness> X("liveness", "ECE/CS 5544 Liveness");
}
