#!/usr/bin/env python3
from uvwBuilder import UVW
import string
import re
import dd
import sys
import copy
from dd.autoref import BDD

# ================
# Main -- Tool to compute a UVW from chains
# ================
if __name__ == '__main__':

    introLine = sys.stdin.readline()
    introLine = introLine.strip().split(" ")
    if len(introLine)!=3:
        sys.stderr.write("Error: Expected header line in the form output by a chain learner tool.\n")
        sys.exit(1)
    assert introLine[0]=="LEARNING"
    nofBits = int(introLine[1])
    nofLetters = int(introLine[2])
    
    # Prepare initial UVW, propositions, and letters
    uvw = UVW()
    for i in range(0,nofBits):
        uvw.propositions.append("v"+str(i))
        uvw.ddMgr.declare("v"+str(i))
        
    letters = []
    for i in range(0,nofLetters):
        thisOne = ~ uvw.ddMgr.false
        for j in range(0,nofBits):
            if (i & (1<< j))>0:
                thisOne = thisOne & uvw.ddMgr.var("v"+str(j))
            else:
                thisOne = thisOne & ~ uvw.ddMgr.var("v"+str(j))
        letters.append(thisOne)
                
    # Parse UVW
    ended = False
    for line in sys.stdin.readlines():
        line = line.strip()
        if line.startswith("CHAIN"):
            assert not ended
            chainParts = line.split(" ")
            
            parts = chainParts[1:]
            nofStates = (len(parts)+1)//2
            
            # Back to the front
            def parseLabel(thisPart):
                label = uvw.ddMgr.false
                for j,k in enumerate(thisPart):
                    if k=="1":
                        label |= letters[j]
                    elif k=="0":
                        pass
                    else:
                        assert False
                return label
            
            lastState = uvw.addStateButNotNewListOfTransitionsForTheNewState("",True)
            uvw.transitions.append([(lastState,parseLabel(parts[-1]))])

            for j in range(nofStates-2,-1,-1):
                nextState = uvw.addStateButNotNewListOfTransitionsForTheNewState("",False)         
                uvw.transitions.append([(lastState,parseLabel(parts[j*2+1])),(nextState,parseLabel(parts[j*2]))])
                lastState = nextState
            
            uvw.initialStates.append(lastState)
            uvw.removeStatesWithoutOutgoingTransitions()
    
            if len(uvw.transitions)>1:
                uvw.simulationBasedMinimization()

            if len(uvw.transitions)>1:
                uvw.mergeEquivalentlyReachableStates()

            if len(uvw.transitions)>1:
                uvw.removeStatesWithoutOutgoingTransitions()
                uvw.removeForwardReachableBackwardSimulatingStates()
            if len(uvw.transitions)>1:
                uvw.removeUnreachableStates()
            if len(uvw.transitions)>1:
                uvw.makeTransientStatesNonRejecting()

        elif line.startswith("END"):
            ended = True

    # Final expensive optimization
    uvw.bruteForceStateRemoval()
 

    # ==========================            
    # Print as graphviz
    # ==========================
    resultLines = ["digraph {"]

    # Assign state names
    neverClaimStateNames = ["T"+str(i) for i in range(0,len(uvw.transitions))]
    assert uvw.rejecting[0]
    neverClaimStateNames[0] = "all"
    for i in uvw.initialStates:
        neverClaimStateNames[i] = neverClaimStateNames[i]+"_init"
    for i in range(0,len(uvw.transitions)):
        if uvw.rejecting[i]:
            # neverClaimStateNames[i] = "reject_"+neverClaimStateNames[i]
            resultLines.append(" \""+neverClaimStateNames[i]+"\" [shape=doublecircle];")

    # Other states
    for i in range(1,len(uvw.transitions)):
        
        for (a,b) in uvw.transitions[i]:
            while b!=uvw.ddMgr.false:
                # Generate minterm for transition
                rest = b
                conditionParts = []
                for p in uvw.propositions:
                    if uvw.ddMgr.exist([p],(rest & uvw.ddMgr.var(p)))!=rest:
                        if (rest & uvw.ddMgr.var(p))==uvw.ddMgr.false:
                            conditionParts.append("!"+p)
                            rest = rest & ~(uvw.ddMgr.var(p))
                        else:
                            conditionParts.append(p)
                            rest = rest & (uvw.ddMgr.var(p))
                b = b & ~rest
                if len(conditionParts)>0:
                    resultLines.append(" \""+neverClaimStateNames[i]+"\" -> \""+neverClaimStateNames[a]+"\" [label=\""+" && ".join(conditionParts)+"\"];")
                else:
                    resultLines.append(" \""+neverClaimStateNames[i]+"\" -> \""+neverClaimStateNames[a]+"\" [label=\"true\"];")
                    
    # First state is special case
    resultLines.append(" \""+neverClaimStateNames[0]+"\" -> \""+neverClaimStateNames[0]+"\" [label=\"true\"];")

    resultLines.append("}")
    
    print("\n".join(resultLines))

