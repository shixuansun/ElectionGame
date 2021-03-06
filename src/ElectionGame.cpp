#include <iostream>
#include <vector>
#include <ctime>
#include <map>
#include <functional>
#include <set>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string.h>
using namespace std;

#define MAX_PARTITION_LENGTH 10
#define MAX_SIZE 10
#define PRECISE 0.0001
#define BYTE_SIZE 8

class PartitionVote;

// Element
struct Candidate
{
    int votes;
    string id;
};

struct Strategy
{
    vector<vector<int>*>* partition;

    short fixedSeats;
    short validNumber;

    int* remainers;
    int* remainersCount;

    ~Strategy()
    {
        delete remainers;
        delete remainersCount;
    }
};


class PartitionVote
{
public:
    // Fixed Seats
    int fixedSeats;
    // key: vote number, value: the number of remainers with same value
    map<int, int, greater<int> >* remainers;
    // The number of valid remainers
    int validRemainers;

    PartitionVote()
    {
        fixedSeats = 0;
        validRemainers = 0;
        remainers = new map<int, int, greater<int> >();
    }

    void UpdatePartitionVote(vector<vector<int>*>& partition, const int seats, const int quota, map<int, Candidate>& party)
    {
        for (vector<vector<int>*>::iterator iterList = partition.begin(); iterList != partition.end(); iterList++)
        {
            int voteSum = 0;

            for (vector<int>::iterator iterCandidate = (*iterList)->begin(); iterCandidate != (*iterList)->end(); iterCandidate++)
            {
                voteSum += party[*iterCandidate].votes;
            }

            int tempFixedSeats = min(voteSum / quota, (int)(*iterList)->size());
            fixedSeats += tempFixedSeats;

            if (tempFixedSeats < (*iterList)->size())
            { 
                int remainer = voteSum % quota;

                if (remainers->count(remainer) == 0)
                    (*remainers)[remainer] = 0;

                (*remainers)[remainer]++;
            }
            
        }

        // Only seats - fixedSeats are valid.
        int maxValidRemainers = seats - fixedSeats;

        for (map<int, int>::iterator iter = remainers->begin(); iter != remainers->end(); iter++)
        {
            if (validRemainers >= maxValidRemainers)
            {
                remainers->erase(iter, remainers->end());
                break;
            }

            validRemainers += iter->second;
        }
    }

    ~PartitionVote()
    {
        delete remainers;
    }
};

// Global Variable
vector<Strategy*> g_strategiesA;
vector<Strategy*> g_strategiesB;

// Party A
map<int, Candidate> g_partyA;

// Party B
map<int, Candidate> g_partyB;

// Quota
int g_quota;

// Seats
int g_seats;

// Max Seats for party A
double g_currentMax;

// size
size_t g_partyA_size;
size_t g_partyB_size;

struct PartitionTuple
{
    int* pPartitions[2];
    int aLength[2];

    ~PartitionTuple()
    {
        delete pPartitions[0];
        delete pPartitions[1];
    }
};

int GetQuotas(const int seats, map<int, Candidate>& partyA, map<int, Candidate>& partyB);
int GetVoteSum(map<int, Candidate>& party);
void OutputPartition(vector<Strategy*>& strategies, int strategy, map<int, Candidate>& party);
void PartitionTwoParts(const int* pSrc, const int iSrcLength, PartitionTuple* &pPartitions, int& iPartitionsLength);
void PrintPartitions(const PartitionTuple* pPartitions, const int iLength);
void GetAllPartitions(vector<vector<int>* >& prefix, const int* pSuffix, const int iSuffixLength, map<int, Candidate>& party, vector<Strategy*>& strategies);
void PrintPartitions(vector<vector<vector<int>* > *>& partitions);
void ClearPartitions(vector<vector<vector<int>* > *>& partitions);
void ReadFile(const string& fileName);
void MiniMaxAlgorithm(vector<Strategy*> &party1, vector<Strategy*> &party2, char* & result, double& currentMax, bool swapIndex);
void OutputResult(char* partyResultA, char* partyResultB);
void OutputResult(int i, int j, double result);
void OutputResult(char* partyResult, double result);

void ConstructStrategy(PartitionVote* pPartitionVote, int* remainers, int* remainersCount, short& validNumber, short& fixedSeats);

int main(int argc, char* argv[])
{
    string fileName = argv[1];

    ReadFile(fileName);

    g_quota = GetQuotas(g_seats, g_partyA, g_partyB);

    vector<vector<int>* > prefixA;

    int* pPartyA = new int[g_partyA.size()];
    for (int i = 0; i < g_partyA.size(); i++)
    {
        pPartyA[i] = i;
    }

    GetAllPartitions(prefixA, pPartyA, g_partyA.size(), g_partyA, g_strategiesA);

    int* pPartyB = new int[g_partyB.size()];
    for (int i = 0; i < g_partyB.size(); i++)
    {
        pPartyB[i] = i;
    }

    vector<vector<int>* > prefixB;
    GetAllPartitions(prefixB, pPartyB, g_partyB.size(), g_partyB, g_strategiesB);

    g_partyA_size = g_strategiesA.size();
    g_partyB_size = g_strategiesB.size();

    int voteSumA = GetVoteSum(g_partyA);
    int voteSumB = GetVoteSum(g_partyB);

    char* possibleResultA = NULL;

    if (g_seats != 1 || (g_seats == 1 && voteSumA >= voteSumB))
    {
        MiniMaxAlgorithm(g_strategiesA, g_strategiesB, possibleResultA, g_currentMax, false);
    }

    double tempMax = 0;
    char* possibleResultB = NULL;

    if (g_seats != 1 || (g_seats == 1 && voteSumB >= voteSumA))
    {
        MiniMaxAlgorithm(g_strategiesB, g_strategiesA, possibleResultB, tempMax, true);
    }

    if (g_seats == 1 && (voteSumA != voteSumB))
    {
        char* resultArray = possibleResultA == NULL ? possibleResultB : possibleResultA;
        double tempResult = possibleResultA == NULL ? (g_seats - tempMax) : g_currentMax;
        OutputResult(resultArray, tempResult);
    }
    else
    {
        OutputResult(possibleResultA, possibleResultB);
    }


    for (int i = 0; i < g_strategiesA.size(); i++)
    {
        delete g_strategiesA[i];
    }

    for (int i = 0; i < g_strategiesB.size(); i++)
    {
        delete g_strategiesB[i];
    }

    delete possibleResultA;
    delete possibleResultB;

    return 0;
}

void OutputResult(char* partyResult, double result)
{
    size_t size = g_partyA_size * g_partyB_size;

    for (size_t i = 0; i < size; i++)
    {
        size_t array_index = i / 8;
        size_t bit_index = i % 8;

        bool isResult = partyResult[array_index] & (1 << bit_index);

        if (isResult)
        {
            int strategyA = i / g_partyB_size;
            int strategyB = i % g_partyB_size;
            OutputResult(strategyA, strategyB, result);
        }
    }
}

void OutputResult(char* partyResultA, char* partyResultB)
{
    size_t size = g_partyA_size * g_partyB_size;

    for (size_t i = 0; i < size; i++)
    {
        size_t array_index = i / 8;
        size_t bit_index = i % 8;

        bool isResult = partyResultA[array_index] & (1 << bit_index);
        isResult = isResult && partyResultB[array_index] & (1 << bit_index);
        if (isResult)
        {
            int strategyA = i / g_partyB_size;
            int strategyB = i % g_partyB_size;
            OutputResult(strategyA, strategyB, g_currentMax);
        }
    }
}

void OutputResult(int i, int j, double result)
{
        cout << "(";
        OutputPartition(g_strategiesA, i, g_partyA);
        cout << ",";
        OutputPartition(g_strategiesB, j, g_partyB);
        cout << ")";
        cout << " payoff (" << result << "," << g_seats - result << ")" << endl;
}

void OutputPartition(vector<Strategy*>& strategies, int strategy, map<int, Candidate>& party)
{
    vector<vector<int>*>* partition = strategies[strategy]->partition;

    cout << "{";
    for (int i = 0; i < partition->size(); i++)
    {
        cout << "{";
        for (int j = 0; j < (*partition)[i]->size(); j++)
        {
            int index = (*(*partition)[i])[j];
            cout << party[index].id;

            if (j != (*partition)[i]->size() - 1)
                cout << ",";
        }
        if (i == partition->size() - 1)
            cout << "}";
        else
            cout << "},";
    }
    cout << "}";
}
void UpdateParty(stringstream &strStream, map<int, Candidate>& party) {
    int i = 0;
    while (!strStream.eof())
    {
        Candidate candidate;
        strStream >> candidate.id >> candidate.votes;
        party[i++] = candidate;
    }
}

void trim(string& str)
{
    string::size_type pos1 = str.find_first_not_of(' ');
    string::size_type pos2 = str.find_last_not_of(' ');
    str = str.substr(pos1 == string::npos ? 0 : pos1,
        pos2 == string::npos ? str.length() - 1 : pos2 - pos1 + 1);
}

void ReadFile(const string& fileName)
{
    ifstream inFile(fileName.c_str(), ios::in);
    string line;
    stringstream strStream;

    int lineNumber = 0;
    while (!inFile.eof())
    {
        getline(inFile, line);
        replace(line.begin(), line.end(), '(', ' ');
        replace(line.begin(), line.end(), ')', ' ');
        replace(line.begin(), line.end(), ',', ' ');

        trim(line);
        strStream.clear();
        strStream.str(line);

        if (lineNumber == 0)
        {
            g_seats = atoi(line.c_str());
        }
        else if (lineNumber == 1)
        {
            UpdateParty(strStream, g_partyA);
        }
        else if (lineNumber == 2)
        {
            UpdateParty(strStream, g_partyB);
            return;
        }
        lineNumber++;
    }
}

void PayOff(Strategy* pStrategyA, Strategy* pStrategyB, double& valueA, double& valueB)
{
    valueA = pStrategyA->fixedSeats;
    valueB = pStrategyB->fixedSeats;

    double leftSeats = g_seats - valueA - valueB;
    short validNumberA = pStrategyA->validNumber;
    short validNumberB = pStrategyB->validNumber;
    int* remainerA = pStrategyA->remainers;
    int* remainerB = pStrategyB->remainers;
    int* remainerCountA = pStrategyA->remainersCount;
    int* remainerCountB = pStrategyB->remainersCount;

    short currentNumberA = 0;
    short currentNumberB = 0;

    while (leftSeats > 0 + PRECISE)
    {
        if (currentNumberA >= validNumberA)
        {
            valueB += leftSeats;
            break;
        }

        if (currentNumberB >= validNumberB)
        {
            valueA += leftSeats;
            break;
        }

        if (remainerA[currentNumberA] > remainerB[currentNumberB])
        {
            double newSeats = min((double)remainerCountA[currentNumberA], leftSeats);
            valueA += newSeats;
            leftSeats -= newSeats;

            currentNumberA++;
        }
        else if (remainerA[currentNumberA] < remainerB[currentNumberB])
        {
            double newSeats = min((double)remainerCountB[currentNumberB], leftSeats);
            valueB += newSeats;
            leftSeats -= newSeats;

            currentNumberB++;
        }
        else
        {
            double totalNumber = remainerCountA[currentNumberA] + remainerCountB[currentNumberB];

            if ((int)totalNumber <= leftSeats)
            {
                valueA += remainerCountA[currentNumberA];
                valueB += remainerCountB[currentNumberB];
            }
            else
            {
                valueA += remainerCountA[currentNumberA] / totalNumber * leftSeats;
                valueB += remainerCountB[currentNumberB] / totalNumber * leftSeats;
            }

            leftSeats -= totalNumber;
            currentNumberA++;
            currentNumberB++;
        }
    }
}

void ConstructResult(vector<vector<int>* >& prefix, const int* pSuffix, const int iSuffixLength, map<int, Candidate>& party, vector<Strategy*>& strategies)
{
    vector<vector<int>* >* pNewPartition = new vector<vector<int>* >();

    for (vector<vector<int>* >::iterator iter = prefix.begin(); iter != prefix.end(); iter++)
    {
        pNewPartition->push_back(new vector<int>(*(*iter)));
    }

    pNewPartition->push_back(new vector<int>(&pSuffix[0], &pSuffix[0] + iSuffixLength));

    PartitionVote* pStrategy = new PartitionVote();
    pStrategy->UpdatePartitionVote(*pNewPartition, g_seats, g_quota, party);

    
    Strategy* pNewStrategy = new Strategy();
    pNewStrategy->partition = pNewPartition;
    pNewStrategy->remainers = new int[MAX_PARTITION_LENGTH];
    pNewStrategy->remainersCount = new int[MAX_PARTITION_LENGTH];
    ConstructStrategy(pStrategy, pNewStrategy->remainers, pNewStrategy->remainersCount, pNewStrategy->validNumber, pNewStrategy->fixedSeats);
    strategies.push_back(pNewStrategy);

    delete pStrategy;
}

void GetAllPartitions(vector<vector<int>* >& prefix, const int* pSuffix, const int iSuffixLength, map<int, Candidate>& party, vector<Strategy*>& strategies)
{
    if (prefix.size() >= MAX_SIZE)
        return;

    ConstructResult(prefix, pSuffix, iSuffixLength, party, strategies);

    if (iSuffixLength < 2)
        return;

    PartitionTuple* pPartitionTuple = NULL;
    int iPartitionsLength = 0;

    PartitionTwoParts(pSuffix, iSuffixLength, pPartitionTuple, iPartitionsLength);

    for (int i = 0; i < iPartitionsLength; ++i)
    {
        vector<int>* pFirstPart = new vector<int>(&pPartitionTuple[i].pPartitions[0][0],
            &pPartitionTuple[i].pPartitions[0][0] + pPartitionTuple[i].aLength[0]);

        prefix.push_back(pFirstPart);

        GetAllPartitions(prefix, pPartitionTuple[i].pPartitions[1], pPartitionTuple[i].aLength[1], party, strategies);

        prefix.pop_back();

        delete pFirstPart;
    }
}

void PartitionTwoParts(const int* pSrc, const int iSrcLength, PartitionTuple* &pPartitions, int& iPartitionsLength)
{
    iPartitionsLength = (1 << (iSrcLength - 1)) - 1;
    pPartitions = new PartitionTuple[iPartitionsLength];

    for (int i = 1; i <= iPartitionsLength; ++i)
    {
        PartitionTuple* pCurrentPartition = &pPartitions[i - 1];
        pCurrentPartition->pPartitions[0] = new int[MAX_PARTITION_LENGTH];
        pCurrentPartition->pPartitions[1] = new int[MAX_PARTITION_LENGTH];

        pCurrentPartition->aLength[0] = 0;
        pCurrentPartition->aLength[1] = 0;

        pCurrentPartition->pPartitions[0][pCurrentPartition->aLength[0]++] = pSrc[0];

        for (int j = 1; j < iSrcLength; j++)
        {
            int iNumber = (i >> (j - 1)) & 1;
            pCurrentPartition->pPartitions[iNumber][pCurrentPartition->aLength[iNumber]++] = pSrc[j];
        }
    }
}

int GetVoteSum(map<int, Candidate>& party)
{
    int voteSum = 0;

    for (map<int, Candidate>::iterator iter = party.begin(); iter != party.end(); iter++)
    {
        voteSum += iter->second.votes;
    }

    return voteSum;
}

int GetQuotas(const int seats, map<int, Candidate>& partyA, map<int, Candidate>& partyB)
{
    return (GetVoteSum(partyA) + GetVoteSum(partyB)) / seats;
}

void MiniMaxAlgorithm(vector<Strategy*> &party1, vector<Strategy*> &party2,
    char* & result, double& currentMax, bool swapIndex)
{
    currentMax = -1;

    double valueA = 0;
    double valueB = 0;

    size_t size = party1.size() * party2.size() / BYTE_SIZE + 1;

    int* tempResult = new int[party2.size()];
    int tempResultCount = 0;

    result = new char[size];
    memset(result, 0, size);

    for (int i = 0; i < party1.size(); i++)
    {
        bool bChanged = false;
        double currentMin = g_seats + 1;

        tempResultCount = 0;

        for (int j = 0; j < party2.size(); j++)
        {
            PayOff(party1[i], party2[j], valueA, valueB);

            if (valueA - currentMin < -PRECISE)
            {
                currentMin = valueA;
                tempResultCount = 0;
            }

            if (currentMin < currentMax - PRECISE)
            {
                bChanged = true;
                break;
            }

            if (fabs(valueA - currentMin) < PRECISE)
            {
                tempResult[tempResultCount++] = j;
            }
        }

        if (!bChanged)
        {
            if (currentMin > currentMax + PRECISE)
            {
                currentMax = currentMin;
                memset(result, 0, size);
            }

            for (int k = 0; k < tempResultCount; k++)
            {
                size_t index = !swapIndex ? (i * party2.size() + (tempResult[k])) : (tempResult[k] * party1.size() + i);
                size_t array_index = index / 8;
                size_t bit_index = index % 8;

                result[array_index] |= (1 << bit_index);
            }
        }
    }
}

void ConstructStrategy(PartitionVote* pPartitionVote, int* remainers, int* remainersCount, short& validNumber, short& fixedSeats)
{
    fixedSeats = pPartitionVote->fixedSeats;

    for (map<int, int>::iterator iter = pPartitionVote->remainers->begin(); iter != pPartitionVote->remainers->end(); iter++)
    {
        remainers[validNumber] = iter->first;
        remainersCount[validNumber++] = iter->second;
    }
}






