// ECE/CS 5544 S24 Assignment 2: available.cpp
// Group: Akhilesh Marathe, Alexander Owens

////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"
// #include "available-support.h"

using namespace llvm;
using namespace std;

namespace {
  class AvailableExprAnalysis : public DataFlow{
    public:

              AvailableExprAnalysis(Direction direction, MeetOperator meet_op) : DataFlow(direction, meet_op) { }

      protected:
            //Apply Transfer function
            transferVar TF(BitVector value, std::vector<void*> domain, std::unordered_map<void*, int> ExprMap, BasicBlock* BB)
            {
                transferVar tfOutput;

                // Finding GenSet and KillSet in each BasicBlock
                int domain_Size = ExprMap.size();
                BitVector GenSet(domain_Size);
                BitVector KillSet(domain_Size);

                for(Instruction& Ins : *BB){
                    Instruction * I = &Ins;
                    // Available expressions is checked only for BinaryOperators
                    if (BinaryOperator * BI = dyn_cast<BinaryOperator>(I)) {
                        // Store the RHS of expression in a newly created expression
                        Expression *expr = new Expression(BI);
                        Expression *check = NULL;
                        bool foundinmap = false;

                        for(void* iter : domain)
                        {
                            if((*expr) == *((Expression *) iter))  //At each stage in the data flow check if the expression evaluates to same value
                            {
                                foundinmap = true;
                                check = (Expression *) iter;
                                break;
                            }
                        }

                        // Generated expression
                        if(foundinmap)  //Evaluate the expressions and check if the operands are being redefined in any of the block
                        {
                            int value_index = ExprMap[(void*)check];
                            GenSet.set(value_index);
                        }
                    }

                    // Killed expressions

                    StringRef insn  =  I->getName();
                    if(!insn.empty())
                    {
                          for(auto domain_itr = domain.begin() ; domain_itr != domain.end() ; domain_itr++)
                        {
                            Expression* expr = (Expression*) (*domain_itr);

                            StringRef op1 = expr->v1->getName();  //Get the instruction name for the operands
                            StringRef op2 = expr->v2->getName();

                            if(op1.equals(insn) || op2.equals(insn))
                            {
                                // Kill if either operand 1 or 2 match the variable assigned
                                std::unordered_map<void*, int>::iterator iter = ExprMap.find((void*) expr);

                                if (iter != ExprMap.end())
                                {
                                    KillSet.set((*iter).second);
                                    GenSet.reset((*iter).second);
                                }
                            }
                        }
                    }

                }


                //update the genSet and killSet for each block.
                if(gen_set.find(BB) == gen_set.end()){
                    gen_set[BB] = GenSet;
                }

                if(kill_set.find(BB) == kill_set.end()){
                    kill_set[BB] = KillSet;
                }

                // Transfer function = GenSet U (input - KillSet)

                tfOutput.tfResult = KillSet;
                // Complement of KillSet
                tfOutput.tfResult.flip();
                // input - KillSet
                tfOutput.tfResult &= value;
                tfOutput.tfResult |= GenSet;

                return tfOutput;
            }

  };




  class AvailableExpressions : public FunctionPass {
    
  public:
    static char ID;
    
    AvailableExpressions() : FunctionPass(ID) { }


 private:

             bool runOnFunction(Function &F) {
                outs() << "FUNCTION :: " << F.getName()  << "\n";


                // Setup the pass
                std::vector<void*> domain;
                // Compute the domain

                for(BasicBlock& B : F){
                    for(Instruction& I : B){
                      // Available expressions is checked only for BinaryOperators
                        if (BinaryOperator * BI = dyn_cast<BinaryOperator>(&I)) {

                            // Create a new Expression to capture the RHS of the BinaryOperator
                            Expression *expr = new Expression(BI);
                            bool found = false;

                            for(void* tfResult : domain)
                            {
                                if((*expr) == *((Expression *) tfResult)) //At each stage in the data flow check if the expression evaluates to same value
                                {
                                    found = true;
                                    break;
                                }
                            }

                            if(found == false)
                                domain.push_back(expr);
                            else
                                delete expr;
                        }
                    }
                }


                //The boundary_condition is set to phi
                //initial condition is set of all variables
                BitVector boundary_Condition(domain.size(), false);
                BitVector initial_Interior_point(domain.size(), true);

                // The pass
                AvailableExprAnalysis pass(Direction::FORWARD,  MeetOperator::INTERSECTION);

                // Apply pass
                DataFlowResult output = pass.run(F, domain, boundary_Condition, initial_Interior_point);


                // Map domain values to index in bitvector
                std::unordered_map<void*, int> ExprMap;
                for (int i = 0; i < domain.size(); i++)
                    ExprMap[(void*)domain[i]] = i;

                //Print the output for iterating over each basic block.
                for(BasicBlock& BL : F){
                    BasicBlock* BB = &BL;

                    outs() << "[BB Name]"<< BB->getName() << "\n";
                   outs() << "IN["<<BB->getName()<<"]: " << formatSet(domain, output.results[BB].in, 1) << "\n";
                   outs() << "OUT["<<BB->getName()<<"]: " << formatSet(domain, output.results[BB].out, 1) << "\n";
                   outs() << "Gen["<<BB->getName()<<"]: " << formatSet(domain, pass.gen_set[BB], 1) << "\n";
                   outs() << "Kill["<<BB->getName()<<"]: " << formatSet(domain, pass.kill_set[BB], 1) << "\n\n";

                }

                return false;
            }

    };
  
  char AvailableExpressions::ID = 0;
  RegisterPass<AvailableExpressions> X("available",
				       "ECE/CS 5544 Available Expressions");
}
