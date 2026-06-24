/**
 * StudySmart AI - Smart Academic Planning Application
 * CST207 Design and Analysis of Algorithms
 * Group: [your group name]
 *
 * 功能: 从csv读取scenario数据 -> merge sort排序 -> greedy选择 -> DP背包 -> k-NN策略推荐
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <random>

using namespace std;

// ============================================================================
// Data Structures (from original code)
// ============================================================================

struct StudyTask {
    string id;
    string name;
    double studyTime;
    int    importance;
    int    deadline;
    int    difficulty;
    string taskType;
};

struct Scenario {
    string name;
    vector<StudyTask> tasks;
    double totalAvailableTime;
};

struct SelectionResult {
    string algorithmName;
    vector<string> selectedTaskIds;
    double totalStudyTime;
    int totalImportance;
    double executionTime;
    string description;
};

// ============================================================================
// Module 1: read scenario data from csv
// ============================================================================
// csv exported from StudySmartAI_Scenario_Settings.xlsx sheets 4-7
// format: first line "name|availTime", then "id,name,type,studyTime,imp,deadline,diff"

vector<string> splitStr(const string& s, char delim) {
    vector<string> parts;
    string cur;
    for (char c : s) {
        if (c == delim) { parts.push_back(cur); cur = ""; }
        else cur += c;
    }
    parts.push_back(cur);
    return parts;
}

Scenario readScenarioCSV(const string& filename) {
    ifstream f(filename);
    Scenario sc;
    string line;

    if (!getline(f, line)) return sc;
    auto p = splitStr(line, '|');
    sc.name = p[0];
    sc.totalAvailableTime = stod(p[1]);

    while (getline(f, line)) {
        auto cols = splitStr(line, ',');
        if (cols.size() >= 7) {
            StudyTask t;
            t.id         = cols[0];
            t.name       = cols[1];
            t.taskType   = cols[2];
            t.studyTime  = stod(cols[3]);
            t.importance = stoi(cols[4]);
            t.deadline   = stoi(cols[5]);
            t.difficulty = stoi(cols[6]);
            sc.tasks.push_back(t);
        }
    }
    return sc;
}

vector<Scenario> loadAllScenarios() {
    vector<Scenario> scenarios;
    for (int i = 1; i <= 4; i++) {
        string fname = "scenario_" + to_string(i) + ".csv";
        scenarios.push_back(readScenarioCSV(fname));
    }
    return scenarios;
}

// Module 2: Sorting / Divide-and-Conquer
// ============================================================================

/*
 * TODO: Implement Merge Sort (or Quick Sort) to rank study tasks
 *       based on a selected criterion (e.g., deadline, importance,
 *       urgency ratio = importance / deadline).
 *
 * Justify your choice of sorting algorithm in the report.
 *
 * Time Complexity: O(n log n)
 * Space Complexity: O(n)
 */
 // Calculate efficiency score = importance / study time
double getEfficiencyScore(const StudyTask& task)
{
    return (task.importance * 0.7) + ((10 - task.difficulty) * 0.3); 
}

// Merge two sorted subarrays based on efficiency score
void mergeByEfficiency(vector<StudyTask>& tasks, int left, int mid, int right)
{
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<StudyTask> L(n1);
    vector<StudyTask> R(n2);

    // Copy data into temporary arrays
    for (int i = 0; i < n1; i++)
        L[i] = tasks[left + i];

    for (int j = 0; j < n2; j++)
        R[j] = tasks[mid + 1 + j];

    int i = 0, j = 0, k = left;

    // Merge arrays in descending order of efficiency
    while (i < n1 && j < n2)
    {
        double scoreL = getEfficiencyScore(L[i]);
        double scoreR = getEfficiencyScore(R[j]);

        // If efficiency is equal, higher importance gets priority
        if (scoreL > scoreR ||
            (scoreL == scoreR && L[i].importance > R[j].importance))
        {
            tasks[k++] = L[i++];
        }
        else
        {
            tasks[k++] = R[j++];
        }
    }

    // Copy remaining elements
    while (i < n1)
        tasks[k++] = L[i++];

    while (j < n2)
        tasks[k++] = R[j++];
}

// Recursive Merge Sort
void mergeSortByEfficiency(vector<StudyTask>& tasks, int left, int right)
{
    if (left >= right)
        return;

    int mid = left + (right - left) / 2;

    mergeSortByEfficiency(tasks, left, mid);
    mergeSortByEfficiency(tasks, mid + 1, right);

    mergeByEfficiency(tasks, left, mid, right);
}

// Rank study tasks using Merge Sort and efficiency score
SelectionResult runSorting(const Scenario& scenario)
{
    clock_t startTime = clock();

    // Create a local copy so original scenario data is not modified
    vector<StudyTask> tasks = scenario.tasks;

    // Sort all tasks by efficiency
    if (!tasks.empty())
    {
        mergeSortByEfficiency(tasks, 0, tasks.size() - 1);
    }

    vector<string> selectedTaskIds;
    double totalStudyTime = 0;
    int totalImportance = 0;

    // Select tasks according to ranking until study time limit is reached
    for (const StudyTask& task : tasks)
    {
        if (totalStudyTime + task.studyTime <= scenario.totalAvailableTime)
        {
            selectedTaskIds.push_back(task.id);
            totalStudyTime += task.studyTime;
            totalImportance += task.importance;
        }
    }

    clock_t endTime = clock();

    SelectionResult result;

    result.algorithmName = "Sorting-based Ranking";
    result.selectedTaskIds = selectedTaskIds;
    result.totalStudyTime = totalStudyTime;
    result.totalImportance = totalImportance;
    result.executionTime =
        static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC;

    result.description =
        "Merge Sort ranks tasks by efficiency score.";

    return result;
}

// ============================================================================
// Module 3: Greedy Algorithm - Task Selection
// ============================================================================

/*
 * TODO: Implement a greedy algorithm to select tasks within the
 *       available study time. Choose an appropriate greedy criterion
 *       (e.g., highest importance, earliest deadline, best efficiency).
 *
 * Time Complexity: O(n log n)
 * Space Complexity: O(n)
 */
bool compareByEfficiency(const StudyTask& a, const StudyTask& b)
{
    double ratioA = a.importance / (a.studyTime * 0.4 + a.deadline * 0.6); 
    double ratioB = b.importance / (b.studyTime * 0.4 + b.deadline * 0.6); 
    //If the efficiency scores are equal, prioritize the task with the shorter 
    if (ratioA == ratioB)
    {
       return a.studyTime < b.studyTime; 
//Sort in descending order cased on efficiency 
    }
    return ratioA > ratioB;
} 

SelectionResult runGreedy(const Scenario& scenario)
{
    clock_t startTime = clock();

    vector<StudyTask> tasks = scenario.tasks;

//Step 1:Sort all tasks
    sort(tasks.begin(), tasks.end(), compareByEfficiency);

    vector<string> selectedTaskIds;
    double totalStudyTime = 0;
    int totalImportance = 0;

//Step 2:Iterate starting
    for (const StudyTask& task : tasks)
    {
        if (totalStudyTime + task.studyTime <= scenario.totalAvailableTime)
        {
            selectedTaskIds.push_back(task.id);
            totalStudyTime += task.studyTime;
            totalImportance += task.importance;
        }
    }

    clock_t endTime = clock();

    SelectionResult result;
    result.algorithmName = "Greedy Strategy";
    result.selectedTaskIds = selectedTaskIds;
    result.totalStudyTime = totalStudyTime;
    result.totalImportance = totalImportance;
    result.executionTime = static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC;
    result.description = "Greedy strategy selects tasks by highest importance-to-study-time ratio first.";

    return result;
}

// ============================================================================
// Module 4: Dynamic Programming (0/1 Knapsack)
// ============================================================================

/*
 * TODO: Implement a Dynamic Programming solution (0/1 Knapsack) to
 *       select tasks maximising total importance under the study time
 *       constraint.
 *
 * Time Complexity: O(n * W)  where W = time limit (in suitable units)
 * Space Complexity: O(n * W)
 */
 SelectionResult runDynamicProgramming(const Scenario& scenario)
{
    clock_t startTime = clock();

    vector<StudyTask> tasks = scenario.tasks;
    int n = tasks.size();

    // Available study time = Knapsack Capacity
    int capacity = static_cast<int>(scenario.totalAvailableTime);

    // DP Table
    vector<vector<int>> dp(n + 1, vector<int>(capacity + 1, 0));

    // Build DP table
    for (int i = 1; i <= n; i++)
    {
        int taskTime = static_cast<int>(tasks[i - 1].studyTime);
        int taskImportance = tasks[i - 1].importance;

        for (int w = 0; w <= capacity; w++)
        {
            if (taskTime <= w)
            {
                dp[i][w] = max(
                    dp[i - 1][w],
                    dp[i - 1][w - taskTime] + taskImportance
                );
            }
            else
            {
                dp[i][w] = dp[i - 1][w];
            }
        }
    }

    // Backtracking: find selected tasks
    vector<string> selectedTaskIds;
    double totalStudyTime = 0;
    int totalImportance = dp[n][capacity];

    int w = capacity;

    for (int i = n; i > 0; i--)
    {
        if (dp[i][w] != dp[i - 1][w])
        {
            selectedTaskIds.push_back(tasks[i - 1].id);

            totalStudyTime += tasks[i - 1].studyTime;

            w -= static_cast<int>(tasks[i - 1].studyTime);
        }
    }

    reverse(selectedTaskIds.begin(), selectedTaskIds.end());

    clock_t endTime = clock();

    double executionTime =
        static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC;

    SelectionResult result;

    result.algorithmName = "Dynamic Programming";

    result.selectedTaskIds = selectedTaskIds;

    result.totalStudyTime = totalStudyTime;

    result.totalImportance = totalImportance;

    result.executionTime = executionTime;

    result.description =
        "0/1 Knapsack DP selects the combination of study tasks "
        "with maximum total importance under the available study time.";

    return result;
}

// ============================================================================

// ============================================================================
// Module 5: k-NN strategy recommendation
// ============================================================================
// 从excel的4个scenario出发，对每个scenario改变availTime产生变体作为训练样本
// 特征：total required time, avail time, pressure ratio, avg importance,
//        deadline tightness (deadline<=3的比例), importance variation (CV)

struct Feat {
    double totalReq;
    double avail;
    double pressure;
    double avgImp;
    double deadlineTight;
    double impVar;
};

Feat getFeatures(const Scenario& s) {
    Feat f;
    f.avail = s.totalAvailableTime;
    f.totalReq = 0;
    double sumImp = 0, sumSq = 0;
    int urgentCnt = 0;

    for (const auto& t : s.tasks) {
        f.totalReq += t.studyTime;
        sumImp     += t.importance;
        sumSq      += (double)t.importance * t.importance;
        if (t.deadline <= 3) urgentCnt++;
    }

    int n = (int)s.tasks.size();
    f.pressure      = f.totalReq / f.avail;
    f.avgImp        = sumImp / n;
    f.deadlineTight = (double)urgentCnt / n;

    double variance = sumSq / n - f.avgImp * f.avgImp;
    if (variance < 0) variance = 0;
    f.impVar = (f.avgImp > 0) ? sqrt(variance) / f.avgImp : 0;

    return f;
}

int bestStrategy(const SelectionResult& a, const SelectionResult& b, const SelectionResult& c) {
    // dp 理论上最优，在同分的情况下优先选dp
    double scoreA = a.totalImportance; 
    double scoreB = b.totalImportance;
    double scoreC = c.totalImportance;
     if (a.selectedTaskIds.size() > b.selectedTaskIds.size()) 
     {
        scoreA += 0.5;
    }
    if (b.totalStudyTime < c.totalStudyTime) 
    {
        scoreB += 0.5; 
    }
    int bestIdx = 2;
    double maxScore = scoreC;
    if (scoreB > maxScore)
    {
        maxScore = scoreB; 
        bestIdx = 1; 
    }
    if (scoreA > maxScore)
    {
        maxScore = scoreA; 
        bestIdx = 0; 
    }
    return bestIdx;// 0=sorting, 1=greedy, 2=dp 
}

double euclidean(const Feat& a, const Feat& b) {
    double d2 = 0;
    d2 += pow(a.totalReq     - b.totalReq, 2);
    d2 += pow(a.avail        - b.avail, 2);
    d2 += pow(a.pressure     - b.pressure, 2);
    d2 += pow(a.avgImp       - b.avgImp, 2);
    d2 += pow(a.deadlineTight - b.deadlineTight, 2);
    d2 += pow(a.impVar       - b.impVar, 2);
    return sqrt(d2);
}

struct Sample {
    Feat features;
    int  label;
};

vector<Sample> genTrainingData(const vector<Scenario>& baseScenarios) {
    vector<Sample> data;
    mt19937 rng(123);
    uniform_real_distribution<double> ratio(0.5, 1.6);

    for (const auto& s : baseScenarios) {
        
        SelectionResult rs = runSorting(s);
        SelectionResult rg = runGreedy(s);
        SelectionResult rd = runDynamicProgramming(s);
        data.push_back({getFeatures(s), bestStrategy(rs, rg, rd)});

        
        for (int k = 0; k < 3; k++) {
            Scenario v = s;
            v.totalAvailableTime = s.totalAvailableTime * ratio(rng);
            v.name = s.name + " (var" + to_string(k+1) + ")";

            SelectionResult r2 = runSorting(v);
            SelectionResult g2 = runGreedy(v);
            SelectionResult d2 = runDynamicProgramming(v);
            data.push_back({getFeatures(v), bestStrategy(r2, g2, d2)});
        }
    }
    return data;
}

int knnPredict(const Feat& query, const vector<Sample>& train, int k) {
    vector<pair<double, int>> dists;
    for (int i = 0; i < (int)train.size(); i++)
        dists.push_back({euclidean(query, train[i].features), i});

    sort(dists.begin(), dists.end());

    int votes[3] = {0, 0, 0};
    int lim = min(k, (int)dists.size());
    for (int i = 0; i < lim; i++)
        votes[train[dists[i].second].label]++;

    int best = 0;
    if (votes[2] > votes[best]) best = 2;  // prefer dp
    if (votes[1] > votes[best]) best = 1;
    return best;
}

// ============================================================================
// Module 6: performance comparison
// ============================================================================

void showCompare(const Scenario& s) {
    SelectionResult sr = runSorting(s);
    SelectionResult gr = runGreedy(s);
    SelectionResult dr = runDynamicProgramming(s);

    cout << "\n  " << s.name;
    cout << "  (avail=" << s.totalAvailableTime << "h, tasks=" << s.tasks.size() << ")\n";
    cout << "  " << string(78, '-') << "\n";
    cout << left
         << setw(26) << "  Strategy"
         << setw(9)  << "Selected"
         << setw(9)  << "Time(h)"
         << setw(12) << "Importance"
         << setw(12) << "Exec(s)"
         << "Note\n";
    cout << "  " << string(78, '-') << "\n";

    auto row = [](const SelectionResult& r) {
        cout << left
             << setw(26) << ("  " + r.algorithmName)
             << setw(9)  << r.selectedTaskIds.size()
             << setw(9)  << fixed << setprecision(1) << r.totalStudyTime
             << setw(12) << r.totalImportance
             << setw(12) << fixed << setprecision(6) << r.executionTime
             << r.description << "\n";
    };

    row(sr); row(gr); row(dr);

    cout << "  " << string(78, '-') << "\n";
    int best = bestStrategy(sr, gr, dr);
    string names[] = {"Sorting", "Greedy", "DP"};
    cout << "  -> Best for this scenario: " << names[best] << "\n";
}

// ============================================================================
// main
// ============================================================================

int main() {
    cout << "\n===== StudySmart AI =====\n";
    cout << "  CST207 Design and Analysis of Algorithms\n";
    cout << "  Group: [your group name]\n\n";

    // ---- part 1: load scenarios from csv ----
    vector<Scenario> scenarios = loadAllScenarios();
    cout << "Read " << scenarios.size() << " scenarios from csv files.\n";

    // ---- part 2: run all 3 strategies on each scenario ----
    cout << "\n========== Strategy Performance Comparison ==========";
    for (const auto& s : scenarios) {
        showCompare(s);
    }

    // ---- part 3: k-NN training and prediction ----
    cout << "\n\n========== k-NN Strategy Recommendation ==========\n\n";

    cout << "  Generating training data from excel scenarios...\n";
    vector<Sample> trainData = genTrainingData(scenarios);
    cout << "  Total training samples: " << trainData.size() << "\n";

    int cnt[3] = {0, 0, 0};
    for (const auto& t : trainData) cnt[t.label]++;
    cout << "  Label breakdown: Sorting=" << cnt[0]
         << "  Greedy=" << cnt[1]
         << "  DP=" << cnt[2] << "\n\n";

    const int K = 3;
    int correct = 0;
    string algoNames[] = {"Sorting", "Greedy", "DP"};

    cout << "  " << left << setw(34) << "Scenario"
         << setw(16) << "Predicted"
         << setw(16) << "Actual Best"
         << "Match\n";
    cout << "  " << string(70, '-') << "\n";

    for (const auto& s : scenarios) {
        Feat f = getFeatures(s);
        int pred = knnPredict(f, trainData, K);

        SelectionResult rs = runSorting(s);
        SelectionResult rg = runGreedy(s);
        SelectionResult rd = runDynamicProgramming(s);
        int actual = bestStrategy(rs, rg, rd);

        bool match = (pred == actual);
        if (match) correct++;

        cout << left
             << "  " << setw(34) << s.name
             << setw(16) << algoNames[pred]
             << setw(16) << algoNames[actual]
             << (match ? "YES" : "NO") << "\n";
    }

    cout << "  " << string(70, '-') << "\n";
    cout << "  Accuracy: " << correct << "/" << scenarios.size()
         << " (" << fixed << setprecision(0)
         << (100.0 * correct / scenarios.size()) << "%)\n";

    return 0;
}
