#include <iostream>
#include <vector>
#include <ctime>
#include <map>
#include <functional>
#include <set>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>
using namespace std;

#define MAX_PARTITION_LENGTH 10
#define MAX_SIZE 10
#define PRECISE 0.0001

class PartitionVote;

// Element
struct Candidate
{
    int votes;
    string id;
};

struct NEResult
{
    int partyAStrategy;
    int partyBStrategy;
    double partyAValue;
    double partyBValue;
};

struct Strategy
{
    vector<vector<int>*>* partition;
    PartitionVote* strategy;
};

// Global Variable
vector<Strategy*> g_strategiesA;
vector<Strategy*> g_strategiesB;
vector<NEResult> g_results;

// Party A
map<int, Candidate> g_partyA;

// Party B
map<int, Candidate> g_partyB;

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
        remainers = new map<int, int, greater<int>>();
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

            fixedSeats += voteSum / quota;
            int remainer = voteSum % quota;

            if (remainers->count(remainer) == 0)
                (*remainers)[remainer] = 0;

            (*remainers)[remainer]++;
        }

        // Only seats - fixedSeats are valid.
        int maxValidRemainers = seats - validRemainers;
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

// Quota
int g_quota;

// Seats
int g_seats;


int GetQuotas(const int seats, map<int, Candidate>& partyA, map<int, Candidate>& partyB);
void OutputResult();
void OutputPartition(vector<Strategy*>& strategies, int strategy, map<int, Candidate>& party);

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

void PartitionTwoParts(const int* pSrc, const int iSrcLength, PartitionTuple* &pPartitions, int& iPartitionsLength);
void PrintPartitions(const PartitionTuple* pPartitions, const int iLength);
void GetAllPartitions(vector<vector<int>* >& prefix, const int* pSuffix, const int iSuffixLength, map<int, Candidate>& party, vector<Strategy*>& strategies);
void PrintPartitions(vector<vector<vector<int>* > *>& partitions);
void ClearPartitions(vector<vector<vector<int>* > *>& partitions);
void FindNE();
void ReadFile(const string& fileName);

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

    FindNE();

    OutputResult();
    return 0;
}

void OutputResult()
{
    for (vector<NEResult>::iterator iter = g_results.begin(); iter != g_results.end(); iter++)
    {
        cout << "(";
        OutputPartition(g_strategiesA, iter->partyAStrategy, g_partyA);
        cout << ",";
        OutputPartition(g_strategiesB, iter->partyBStrategy, g_partyB);
        cout << ")";
        cout << " payoff (" << iter->partyAValue << "," << iter->partyBValue << ")" << endl;
    }
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

    regex split_pattern("\\)|,|\\(");

    int lineNumber = 0;
    while (!inFile.eof())
    {
        getline(inFile, line);
        line = regex_replace(line, split_pattern, " ");
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
    valueA = pStrategyA->strategy->fixedSeats;
    valueB = pStrategyB->strategy->fixedSeats;

    int leftSeats = g_seats - valueA - valueB;
    map<int, int>::iterator iterA = pStrategyA->strategy->remainers->begin();
    map<int, int>::iterator iterB = pStrategyB->strategy->remainers->begin();

    while (leftSeats > 0)
    {
        if (iterA == pStrategyA->strategy->remainers->end())
        {
            valueB += leftSeats;
            break;
        }

        if (iterB == pStrategyB->strategy->remainers->end())
        {
            valueA += leftSeats;
            break;
        }

        if (iterA->first > iterB->first)
        {
            int newSeats = min(iterA->second, leftSeats);
            valueA += newSeats;
            leftSeats -= newSeats;

            iterA++;
        }
        else if (iterA->first < iterB->first)
        {
            int newSeats = min(iterB->second, leftSeats);
            valueB += newSeats;
            leftSeats -= newSeats;

            iterB++;
        }
        else
        {
            double totalNumber = iterA->second + iterB->second;

            if ((int)totalNumber <= leftSeats)
            {
                valueA += iterA->second;
                valueB += iterB->second;
            }
            else
            {
                valueA += iterA->second / totalNumber * leftSeats;
                valueB += iterB->second / totalNumber * leftSeats;
            }

            leftSeats -= totalNumber;
            iterA++;
            iterB++;
        }
    }
}

void FindNE()
{
    map<int, set<int>*> maxPartyAStrategy;

    for (int i = 0; i < g_strategiesB.size(); i++)
    {
        maxPartyAStrategy[i] = new set<int>();
    }

    map<int, set<int>*> maxPartyBStrategy;

    for (int i = 0; i < g_strategiesA.size(); i++)
    {
        maxPartyBStrategy[i] = new set<int>();
    }

    double* maxPartyA = new double[g_strategiesB.size()]{ 0 };
    double* maxPartyB = new double[g_strategiesA.size()]{ 0 };

    double valueA = 0;
    double valueB = 0;

    for (int i = 0; i < g_strategiesA.size(); i++)
    {
        for (int j = 0; j < g_strategiesB.size(); j++)
        {
            PayOff(g_strategiesA[i], g_strategiesB[j], valueA, valueB);

            if (fabs(valueA - maxPartyA[j]) < PRECISE )
            {
                maxPartyAStrategy[j]->insert(i);
            }
            else if (valueA > (maxPartyA[j] + PRECISE))
            {
                maxPartyA[j] = valueA;
                maxPartyAStrategy[j]->clear();
                maxPartyAStrategy[j]->insert(i);
            }

            if (fabs(valueB - maxPartyB[i]) < PRECISE)
            {
                maxPartyBStrategy[i]->insert(j);
            }
            else if (valueB > (maxPartyB[i] + PRECISE))
            {
                maxPartyB[i] = valueB;
                maxPartyBStrategy[i]->clear();
                maxPartyBStrategy[i]->insert(j);
            }
        }
    }

    for (map<int, set<int>*>::iterator iterPartyA = maxPartyAStrategy.begin(); iterPartyA != maxPartyAStrategy.end(); iterPartyA++)
    {
        int strategyBId = iterPartyA->first;
        for (set<int>::iterator iterPartyAStrategy = iterPartyA->second->begin(); iterPartyAStrategy != iterPartyA->second->end(); iterPartyAStrategy++)
        {
            int strategyAId = *iterPartyAStrategy;

            if (maxPartyBStrategy[strategyAId]->count(strategyBId) != 0)
            {
                NEResult result;
                result.partyAStrategy = strategyAId;
                result.partyBStrategy = strategyBId;
                result.partyAValue = maxPartyA[strategyBId];
                result.partyBValue = maxPartyB[strategyAId];

                g_results.push_back(result);
            }
        }
    }
    delete maxPartyA;
    delete maxPartyB;
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
    pNewStrategy->strategy = pStrategy;
    strategies.push_back(pNewStrategy);
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

int GetQuotas(const int seats, map<int, Candidate>& partyA, map<int, Candidate>& partyB)
{
    int voteSum = 0;

    for (map<int, Candidate>::iterator iter = partyA.begin(); iter != partyA.end(); iter++)
    {
        voteSum += iter->second.votes;
    }

    for (map<int, Candidate>::iterator iter = partyB.begin(); iter != partyB.end(); iter++)
    {
        voteSum += iter->second.votes;
    }

    return voteSum / seats;
}










