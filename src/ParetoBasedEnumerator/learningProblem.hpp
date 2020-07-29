#ifndef __LEARNING_PROBLEM_HPP__
#define __LEARNING_PROBLEM_HPP__

#include <list>
#include <vector>

class LearningProblem {
private:
    unsigned int nofBitsPerLetter;
    unsigned int nofLetters;
    std::list<std::pair<std::vector<unsigned int>,std::vector<unsigned int> > > positiveExamples;
public:
    LearningProblem(std::string &inputFileName, int numberOfLines);
    inline unsigned int getNofLetters() const { return nofLetters; }
    inline unsigned int getNofBitsPerLetter() const { return nofBitsPerLetter; }
    inline std::list<std::pair<std::vector<unsigned int>,std::vector<unsigned int> > >::const_iterator begin() const { return positiveExamples.begin(); }
    inline std::list<std::pair<std::vector<unsigned int>,std::vector<unsigned int> > >::const_iterator end() const { return positiveExamples.end(); }
};


/**
 * @brief Loads a learning problem from disk
 * @param inputFileName
 */
inline LearningProblem::LearningProblem(std::string &inputFileName, int numberOfLines) {

    std::ifstream inFile(inputFileName);
    if (inFile.fail()) throw "Error opening input file";

    // Read first line with the number of bits
    {
        std::string firstLine;
        std::getline(inFile,firstLine);
        if (inFile.fail()) throw "Error reading first line from input file.";
        if (firstLine.substr(0,39)!="Learning problem with character width: ") throw "First line does not have a learning problem signature.";
        std::istringstream nofBitsParser(firstLine.substr(39));
        nofBitsParser >> nofBitsPerLetter;
        if (nofBitsParser.fail()) throw "Cannot read number of bits from the first line.";
    }

    // Read second line with the number of characters
    {
        std::string secondLine;
        std::getline(inFile,secondLine);
        if (inFile.fail()) throw "Error reading second line from input file.";
        if (secondLine.substr(0,16)!="Nof characters: ") throw "Second line does not have a learning problem signature.";
        std::istringstream nofLettersParser(secondLine.substr(16));
        nofLettersParser >> nofLetters;
        if (nofLettersParser.fail()) throw "Cannot read number of letters from the first line.";
    }

    // Sanity check: Number of letters
    if ((1UL<<nofBitsPerLetter)<nofLetters) throw "Error: More letters declared than can be formed with the given number of bits.";

    // Read positive example lines
    std::string positiveExample;
    int nofLine = 2;
    while (std::getline(inFile,positiveExample)) {

        nofLine++;

        // checks if the number of lines parameter (numberOfLines) was supplied.
        // If so, it only reads up to numberOfLines lines
        if (numberOfLines != -1 && nofLine > numberOfLines){
            break;
        }
        trim(positiveExample);
        if (positiveExample.length()>0) {

            // Find separating space
            size_t spacePos = positiveExample.find(" ");
            if (spacePos==std::string::npos) {
                std::ostringstream error;
                error << "Cannot find space in positive example in line " << nofLine << "!";
                throw error.str();
            }

            // Size sanity checks
            if ((spacePos % nofBitsPerLetter)!=0) {
                std::ostringstream error;
                error << "Space found at a non-multiple of the number of bits in line " << nofLine << "!";
                throw error.str();
            }

            unsigned int nofBitsCycle = positiveExample.length()-spacePos-1;

            if ((nofBitsCycle % nofBitsPerLetter)!=0) {
                std::ostringstream error;
                error << "Cycle length is a non-multiple of the number of bits in line " << nofLine << "!";
                throw error.str();
            }

            positiveExamples.push_back(std::pair<std::vector<unsigned int>,std::vector<unsigned int> >(std::vector<unsigned int >(),std::vector<unsigned int>()));
            std::vector<unsigned int> &prefix = positiveExamples.back().first;
            std::vector<unsigned int> &cycle = positiveExamples.back().second;

            // Parse prefix
            unsigned int letter = 0;
            for (unsigned int i=0;i<spacePos;i++) {
                if (positiveExample[i]=='0') {
                    // Nothing to add
                } else if (positiveExample[i]=='1') {
                    letter |= (1 << ((i % nofBitsPerLetter)));
                } else {
                    std::ostringstream error;
                    error << "Non-binary literal in line " << nofLine << ", character " << i+1 << ".";;
                    throw error.str();
                }
                if ((i%nofBitsPerLetter)==(nofBitsPerLetter-1)) {
                    prefix.push_back(letter);
                    letter = 0;
                }
            }

            // Parse suffix
            for (unsigned int i=0;i<nofBitsCycle;i++) {
                if (positiveExample[i+spacePos+1]=='0') {
                    // Letter should not be changed.
                } else if (positiveExample[i+spacePos+1]=='1') {
                    letter |= (1 << ((i % nofBitsPerLetter)));
                } else {
                    std::ostringstream error;
                    error << "Non-binary literal in line " << nofLine << ", character " << i+spacePos+2 << ".";;
                    throw error.str();
                }
                if ((i%nofBitsPerLetter)==(nofBitsPerLetter-1)) {
                    cycle.push_back(letter);
                    letter = 0;
                }
            }


        }
    }
}






#endif
