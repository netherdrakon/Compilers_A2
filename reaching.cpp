// ECE/CS 5544 Assignment 2: reaching.cpp
// Group: Akhilesh Marathe, Alexander owens
////////////////////////////////////////////////////////////////////////////////

#include "dataflow.h"

using namespace llvm;

namespace {


    // reaching definition Analysis class
    class ReachingDefinitionsAnalysis : public DataFlow {
       public:

            ReachingDefinitionsAnalysis(Direction direction, MeetOperator meet_op) : DataFlow(direction, meet_op) { }

      protected:
        //Apply Transfer function 
            transferVar TF(BitVector value,  std::vector<void*> domain, std::unordered_map<void*, int> domainVal_index, BasicBlock* BB){

                transferVar transfer;
                //To find the downward exposed definitions and killed definitions in each block
                int domain_Size = domainVal_index.size();
                BitVector GenSet(domain_Size);
                BitVector KillSet(domain_Size);

                  for (Instruction &I: *BB ){

                    auto curr_itr = domainVal_index.find(&I); //Itereate over each definition in the domain of instructions

                    if (curr_itr != domainVal_index.end()) {

                        //Kill prior definitions for the same variable (including those in this block's gen set)
                        for (void* tfResult : domain) {
                            std::string prevDef = valueToDefinitionVarStr((Value*)tfResult);
                            std::string currDef = valueToDefinitionVarStr((Value*)curr_itr->first);
                            if (prevDef == currDef) { //Check if the previous definitions for a variable is same as current definition in the block
                                KillSet.set(domainVal_index[tfResult]); //Kill the prior definitions including the block's gen set
                                GenSet.reset(domainVal_index[tfResult]);
                            }
                        }
                        //Add this new definition to gen set
                        GenSet.set(curr_itr->second);
                    }
                }

                //update the genSet and killSet for each block.
                if(gen_set.find(BB) == gen_set.end()){
                    gen_set[BB] = GenSet;
                }

                if(kill_set.find(BB) == kill_set.end()){
                    kill_set[BB] = KillSet;
                }

                //Then, apply transfer function: Y = GenSet union (X - KillSet)
                transfer.tfResult = KillSet;
                    // Complement of KillSet
                transfer.tfResult.flip();
                    // X - KillSet
                transfer.tfResult &= value;
                transfer.tfResult |= GenSet;

                return transfer;
            }
    };


    class ReachingDefinitions : public FunctionPass {
    public:
        static char ID;

        ReachingDefinitions() : FunctionPass(ID) {
        }

        void getAnalysisUsage(AnalysisUsage& AU) const {
            AU.setPreservesAll();
        }

  private:
       bool runOnFunction(Function &F) {
            // Get Function name
            outs() << "FUNCTION :: " << F.getName()  << "\n";

            // Setup the pass
            std::vector<void*> domain;

            // Compute domain for function
           for (auto arg = F.arg_begin(); arg != F.arg_end(); ++arg) {
                  domain.push_back(&*arg);
            }

            // Check for other instructions
           for (auto I = inst_begin(F); I != inst_end(F); ++I) {
                if (Value* val = dyn_cast<Value> (&*I)) {
                    if (!valueToDefinitionStr(val).empty()){
                        if (std::find(domain.begin(), domain.end(), val) == domain.end()) {
                            domain.push_back(val);
                        }
                    }
                }
            }

            BitVector boundary_Condition(domain.size(), false);
            //Initial boundary condition is set of input arguments
            for (int i = 0; i < domain.size(); i++){
                if (isa<Argument>((Value*)domain[i]))
                    boundary_Condition.set(i);
            }

            //Interior initial values = empty sets
            BitVector initial_Interior_point(domain.size(), false);

            //Initialize the ReachingDefinitionsAnalysis.
            ReachingDefinitionsAnalysis pass(Direction::FORWARD, MeetOperator::UNION);

            // Apply pass
            DataFlowResult output = pass.run(F, domain, boundary_Condition, initial_Interior_point);

            //Print the output for iterating over each basic block.
            for(BasicBlock& BI : F){
                BasicBlock* BB = &BI;

                outs() << "BB Name"<< BB->getName() << "\n";
                outs() << "IN["<<BB->getName()<<"]: " << setToStr(domain, output.results[BB].in) << "\n";
                outs() << "OUT["<<BB->getName()<<"]: "<< setToStr(domain, output.results[BB].out) << "\n";
                outs() << "Gen["<<BB->getName()<<"]: "<< setToStr(domain, pass.gen_set[BB]) << "\n";
                outs() << "Kill["<<BB->getName()<<"]: "<< setToStr(domain, pass.kill_set[BB]) << "\n\n";
              }

            return false;
        }

    };

    char ReachingDefinitions::ID = 0;
    RegisterPass<ReachingDefinitions> X("reaching", " ",false);
}
