#include <iostream>
#include <bitset>
#include <list>
#include <vector>
#include <sstream>
#include <fstream>
#include <set>
#include "tools.hpp"
#include "pareto_enumerator.hpp"
#include "learningProblem.hpp"



class Learner {
private:
    const LearningProblem &problem;
    unsigned int maxUVWLength;
    std::list<std::vector<int> > pastChains;
public:
    Learner(const LearningProblem &_problem, unsigned int _maxUVWLength) : problem(_problem), maxUVWLength(_maxUVWLength) {}
    void learn(unsigned int uvwChainLength);
    void learn() {
        for (unsigned int i=1;i<=maxUVWLength;i++) learn(i);
    }
};


void Learner::learn(unsigned int uvwChainLength) {

    if (uvwChainLength>63) throw "Error: Due to optimziations in the code, no UVW chain lengths >63 are supported.";

    unsigned int nofBitsPerChain = problem.getNofLetters()*(2*uvwChainLength-1);
    std::vector<std::pair<int,int> > limits(nofBitsPerChain);
    for (unsigned int i=0;i<nofBitsPerChain;i++) limits[i] = std::pair<int,int>(0,1);

    // Safety case: Modify limits so that the "end of word" character is the only one in the loop, and only there.
    if (problem.getSafetyMode()!=LIVENESS) {
        for (unsigned int i=0;i<2*uvwChainLength-1;i++) {
            limits[i*problem.getNofLetters()+problem.getNofLetters()-1] = std::pair<int,int>(1,1);
        }
        for (unsigned int i=0;i<problem.getNofLetters()-1;i++) {
            limits[problem.getNofLetters()*(2*uvwChainLength-2)+i] = (problem.getSafetyMode()==FINITEWORDS)?std::pair<int,int>(1,1):std::pair<int,int>(0,0);
        }
        limits[limits.size()-1] = std::pair<int,int>(0,0);
    }

    // Define model checking function
    std::function<bool(const std::vector<int> &)> modelCheckingFn = [this,uvwChainLength](std::vector<int> chain) {

#ifndef NDEBUG
        std::cerr << "Call: ";
        for (auto it : chain) std::cerr << (it?"1":"0");
        std::cerr << " ";
#endif

        // No that having chain[...] = FALSE represents a transition in the UVW chain.

#ifndef NDEBUG
        int rejectingLine = 0;
#endif

        for (auto it = problem.begin();it!=problem.end();it++) {
            bool reject = true;

            // std::cerr << "X" << it->second.size() << it->first.size();

            // First check if the final state of this chain would accept
            for (unsigned int ltrSuffix : it->second) {
                if (chain[(2*uvwChainLength-2)*problem.getNofLetters()+ltrSuffix]) reject = false;
            }

            // std::cerr << "M" << reject;

            if (reject) {
                // So the cycle would be rejected. Ok, then let's see if it's reachable!

                // First, prefix
                uint64_t reachableLast = 1;
                for (unsigned int character : it->first) {
                    uint64_t reachable = 0;
                    for (unsigned int state=0;state<uvwChainLength;state++) {
                        if (((reachableLast & (1<<state))>0)) {
                            // Self-cycle
                            if (!chain[2*state*problem.getNofLetters()+character])  {
                                reachable |= 1 << (state);
                            }
                            // Forward
                            if (state+1<uvwChainLength) {
                                if (!chain[2*state*problem.getNofLetters()+character+problem.getNofLetters()])  {
                                    reachable |= 1 << (state+1);
                                }
                            }
                        }
                    }
                    reachableLast = reachable;
                }

                uint64_t reachableLoop = reachableLast;
                uint64_t reachableLastLoop = 0;

                // Saturate
                while (reachableLoop!=reachableLastLoop) {

                    for (unsigned int character : it->second) {
                        uint64_t reachable = 0;
                        for (unsigned int state=0;state<uvwChainLength;state++) {
                            if (((reachableLast & (1<<state))>0)) {
                                // Self-cycle
                                if (!chain[2*state*problem.getNofLetters()+character])  {
                                    reachable |= 1 << (state);
                                }
                                // Forward
                                if (state+1<uvwChainLength) {
                                    if (!chain[2*state*problem.getNofLetters()+character+problem.getNofLetters()])  {
                                        reachable |= 1 << (state+1);
                                    }
                                }
                            }
                        }
                        reachableLast = reachable;
                    }

                    reachableLastLoop = reachableLoop;
                    reachableLoop |= reachableLast;
                }

                // Model checking?
                if (reachableLoop & (1<<(uvwChainLength-1))) {
#ifndef NDEBUG
                    std::cerr << " reject" << rejectingLine << "\n";
#endif
                    return false;
                }

            }
#ifndef NDEBUG
            rejectingLine++;
#endif
        }
#ifndef NDEBUG
        std::cerr << " accept\n";
#endif
        return true;
    };

    // Define callback function
    std::function<void(const std::vector<int> &)> callbackFn = [this,uvwChainLength](std::vector<int> chain) {

        // Test if any of the parts is the empty set
        bool nonEmpty = true;
        for (unsigned int i=1;i<2*uvwChainLength-1;i+=2) {
            bool thisOne = false;
            for (unsigned int j=0;j<problem.getNofLetters();j++) {
                thisOne |= chain[i*problem.getNofLetters()+j]==0;
            }
            nonEmpty &= thisOne;
        }

        // Loop empty?
        bool thisOne = false;
        for (unsigned int j=0;j<problem.getNofLetters();j++) {
            thisOne |= chain[(uvwChainLength-1)*2*problem.getNofLetters()+j]==0;
        }
        nonEmpty &= thisOne;

        if (nonEmpty) {

            // Check simulation by an older chain
            unsigned int nof= 0;
            for (auto &it : pastChains) {
                nof++;

                // Simulation at the last element
                bool sim = true;
                for (unsigned int j=0;j<problem.getNofLetters();j++) {
                    sim &= (chain[chain.size()-problem.getNofLetters()+j]>0) || it[it.size()-problem.getNofLetters()+j]==0;
                    //std::cerr << "p: " << chain.size()-problem.getNofLetters()+j << std::endl;
                    //std::cerr << "Q: " << it.size()-problem.getNofLetters()+j << std::endl;
                }

                // Other simulation elements
                if (sim) {
                    std::list<std::pair<unsigned int,unsigned int> > todo; todo.push_back(std::pair<int,int>(0,0));
                    std::set<std::pair<unsigned int,unsigned int> > done; done.insert(std::pair<int,int>(0,0));
                    while (todo.size()!=0) {
                        std::pair<int,int> thisOne = todo.front();
                        todo.pop_front();
                        //std::cerr << "M " << thisOne.first << "," << thisOne.second << "\n";

                        // Check if staying is possible.
                        bool stay = true;
                        for (unsigned int j=0;j<problem.getNofLetters();j++) {
                            stay &= chain[thisOne.first*problem.getNofLetters()*2+j] || !it[(thisOne.second*2)*problem.getNofLetters()+j];
                        }
                        //std::cerr << "S" << stay << std::endl;

                        if ((stay) && (thisOne.first < (int)uvwChainLength-1)) {

                            // Check if we can move one forward
                            bool front = true;
                            for (unsigned int j=0;j<problem.getNofLetters();j++) {
                                front &= chain[(thisOne.first*2+1)*problem.getNofLetters()+j] || !it[(thisOne.second*2)*problem.getNofLetters()+j];
                            }
                            //std::cerr << "F" << front << std::endl;

                            if (front) {
                                std::pair<int,int> next(thisOne.first+1,thisOne.second);
                                if (done.count(next)==0) {
                                    todo.push_back(next);
                                    done.insert(next);
                                }
                            }

                            // Check if we can move two forward
                            if (thisOne.second < (int)(it.size()/problem.getNofLetters()/2)) {

                                bool both = true;
                                for (unsigned int j=0;j<problem.getNofLetters();j++) {
                                    both &= chain[(thisOne.first*2+1)*problem.getNofLetters()+j] || !it[(thisOne.second*2+1)*problem.getNofLetters()+j];
                                }
                                if (both) {
                                    std::pair<int,int> next(thisOne.first+1,thisOne.second+1);
                                    if (done.count(next)==0) {
                                        todo.push_back(next);
                                        done.insert(next);
                                    }
                                }
                            }
                        }

                    }

                    if (done.count(std::pair<int,int>(uvwChainLength-1,it.size()/problem.getNofLetters()/2))>0) {

                        /*std::cout << "NOCHAIN(" << nof << ")";
                        for (unsigned int i=0;i<2*uvwChainLength-1;i++) {
                            std::cout << " ";
                            for (unsigned int j=0;j<problem.getNofLetters();j++) {
                                std::cout << (chain[i*problem.getNofLetters()+j]?'0':'1');
                            }
                        }
                        std::cout << "\n";*/


                        return;
                    }
                }

            }

            std::cout << "CHAIN";
            for (unsigned int i=0;i<2*uvwChainLength-1;i++) {
                std::cout << " ";
                for (unsigned int j=0;j<problem.getNofLetters();j++) {
                    std::cout << (chain[i*problem.getNofLetters()+j]?'0':'1');
                }
            }
            std::cout << "\n";
        }

        pastChains.push_back(chain);
    };

    paretoenumerator::enumerateParetoFront(callbackFn,modelCheckingFn,limits);

}



/**
 * @brief Program entry point
 * @param nofArgs
 * @param args
 * @return
 */
int main(int nofArgs, const char **args) {
    try {
        std::string inputFilename = "";
        int nofLines = -1;
        unsigned int uvwChainLength = 2;
        SafetyMode safetyMode = LIVENESS;
        for (int i=1;i<nofArgs;i++) {
            std::string thisArg = args[i];
            if (thisArg.substr(0,1)=="-") {
                // Parameter
                if (thisArg=="-c") {
                    if (i==nofArgs-1) throw "Error: Require a number after '-c'";
                    std::istringstream cl(args[++i]);
                    cl >> uvwChainLength;
                    if (cl.fail()) throw"Error: Required a valid number after '-c'";
                }
                else if (thisArg=="-l") {
                    if (i==nofArgs-1) throw "Error: Require a number after '-l'";
                    std::istringstream cl(args[++i]);
                    cl >> nofLines;
                    if (cl.fail()) throw"Error: Required a valid number after '-l'";
                }
                else if (thisArg=="-s") {
                    safetyMode = SAFETY;
                }
                else if (thisArg=="-f") {
                    safetyMode = FINITEWORDS;
                }

                else {
                    throw std::string("Error: Did not understand parameter'")+thisArg+"'";
                }
            } else {
                if (inputFilename.length()>0) throw "Error: More than one input file name given.";
                inputFilename = thisArg;
            }
        }
        if (inputFilename.length()==0) throw "Error: No input file name given.";

        // Start the learner
        LearningProblem learningProblem(inputFilename, nofLines, safetyMode);
        Learner learner(learningProblem,uvwChainLength);
        std::cout << "LEARNING " << learningProblem.getNofBitsPerLetter() << " " << learningProblem.getNofLetters() << std::endl;

        for (unsigned int l=1;l<=uvwChainLength;l++) {
            learner.learn(l);
        }
        std::cout << "END\n";


    } catch (const char *error) {
        std::cerr << error << std::endl;
        return 1;
    } catch (std::string error) {
        std::cerr << error << std::endl;
        return 1;
    }

}
