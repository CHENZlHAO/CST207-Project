/**
 * StudySmart AI - Smart Academic Planning Application
 * CST207 Design and Analysis of Algorithms
 *
 * Main modules:
 *  1. Read study-task scenarios from CSV files
 *  2. Sorting / Divide-and-Conquer module using Merge Sort
 *  3. Greedy planning module
 *  4. Dynamic Programming module using 0/1 Knapsack
 *  5. k-NN strategy recommendation module
 *  6. Performance comparison output
 *
 * CSV format for each scenario file:
 *  First line:  Scenario name|available study time
 *  Other lines: id,name,type,studyTime,importance,deadline,difficulty
 *
 * Example file names expected in the same folder:
 *  scenario_1.csv
 *  scenario_2.csv
 *  scenario_3.csv
 *  scenario_4.csv
 */

#include <algorithm>
#include <cctype>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ============================================================================
// Constants and basic definitions
// ============================================================================

const int TIME_SCALE = 10; // 1 hour = 10 units, so 0.5h can be represented

enum StrategyLabel {
    SORTING = 0,
    GREEDY  = 1,
    DP      = 2
};

struct StudyTask {
    string id;
    string name;
    double studyTime;
    int importance;
    int deadline;
    int difficulty;
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

struct Feat {
    int numTasks;
    double totalReq;
    double avail;
    double pressure;
    double avgImp;
    double deadlineTight;
    double impVar;
    double avgDiff;
};

struct Sample {
    Feat features;
    int label;
};

// ============================================================================
// Utility functions
// ============================================================================

string trim(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) {
        start++;
    }

    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) {
        end--;
    }

    return s.substr(start, end - start);
}

string removeUTF8BOM(string s) {
    if (s.size() >= 3 &&
        static_cast<unsigned char>(s[0]) == 0xEF &&
        static_cast<unsigned char>(s[1]) == 0xBB &&
        static_cast<unsigned char>(s[2]) == 0xBF) {
        return s.substr(3);
    }
    return s;
}

string toLowerStr(string s) {
    for (char& c : s) {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

vector<string> splitStr(const string& s, char delim) {
    vector<string> parts;
    string cur;

    for (char c : s) {
        if (c == delim) {
            parts.push_back(trim(cur));
            cur.clear();
        } else {
            cur += c;
        }
    }

    parts.push_back(trim(cur));
    return parts;
}

// Simple CSV splitter with support for quoted fields.
vector<string> splitCSVLine(const string& line) {
    vector<string> cols;
    string cur;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];

        if (c == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                cur += '"';
                i++;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            cols.push_back(trim(cur));
            cur.clear();
        } else {
            cur += c;
        }
    }

    cols.push_back(trim(cur));
    return cols;
}

int timeToUnits(double hours) {
    return max(0, static_cast<int>(round(hours * TIME_SCALE)));
}

string strategyName(int label) {
    if (label == SORTING) return "Sorting";
    if (label == GREEDY)  return "Greedy";
    return "DP";
}

string strategyFullName(int label) {
    if (label == SORTING) return "Sorting-based Ranking";
    if (label == GREEDY)  return "Greedy Strategy";
    return "Dynamic Programming";
}

string joinIds(const vector<string>& ids) {
    if (ids.empty()) return "None";

    string result;
    for (size_t i = 0; i < ids.size(); i++) {
        result += ids[i];
        if (i + 1 < ids.size()) result += ", ";
    }
    return result;
}

// ============================================================================
// Module 1: Read scenario data from CSV
// ============================================================================

Scenario readScenarioCSV(const string& filename) {
    ifstream f(filename);
    Scenario sc;
    sc.name = filename;
    sc.totalAvailableTime = 0.0;

    if (!f.is_open()) {
        cerr << "Warning: Cannot open file: " << filename << "\n";
        return sc;
    }

    string line;
    if (!getline(f, line)) {
        cerr << "Warning: Empty file: " << filename << "\n";
        return sc;
    }

    line = removeUTF8BOM(trim(line));
    vector<string> first = splitStr(line, '|');

    if (first.size() < 2) {
        cerr << "Warning: Invalid first line in " << filename
             << ". Expected format: Scenario name|available time\n";
        return sc;
    }

    sc.name = first[0];
    sc.totalAvailableTime = stod(first[1]);

    while (getline(f, line)) {
        line = trim(line);
        if (line.empty()) continue;

        vector<string> cols = splitCSVLine(line);
        if (cols.size() < 7) continue;

        string firstColLower = toLowerStr(cols[0]);
        if (firstColLower == "id" || firstColLower == "task id") {
            continue;
        }

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

    return sc;
}

vector<Scenario> loadAllScenarios() {
    vector<Scenario> scenarios;

    for (int i = 1; i <= 4; i++) {
        string fname = "scenario_" + to_string(i) + ".csv";
        Scenario sc = readScenarioCSV(fname);

        if (!sc.tasks.empty()) {
            scenarios.push_back(sc);
        }
    }

    return scenarios;
}

// ============================================================================
// Module 2: Sorting / Divide-and-Conquer using Merge Sort
// ============================================================================

/*
 * Ranking criterion:
 *   priority score = 0.60 * importance
 *                  + 0.25 * deadline urgency score
 *                  + 0.15 * ease score
 *
 * This module is mainly suitable when the purpose is prioritisation/ranking.
 * Time complexity:  O(n log n)
 * Space complexity: O(n)
 */

double sortingPriorityScore(const StudyTask& task) {
    double importanceScore = static_cast<double>(task.importance);
    double deadlineScore = max(0.0, 10.0 - static_cast<double>(task.deadline));
    double easeScore = max(0.0, 6.0 - static_cast<double>(task.difficulty));

    return 0.60 * importanceScore + 0.25 * deadlineScore + 0.15 * easeScore;
}

// Compare two tasks based on priority score.
// If scores are almost equal, use importance, deadline, and study time as tie-breakers.
bool betterForSorting(const StudyTask& a, const StudyTask& b) {
    double sa = sortingPriorityScore(a);
    double sb = sortingPriorityScore(b);

    if (fabs(sa - sb) > 1e-9) return sa > sb;
    if (a.importance != b.importance) return a.importance > b.importance;
    if (a.deadline != b.deadline) return a.deadline < b.deadline;
    return a.studyTime < b.studyTime;
}

// Merge two sorted subarrays into one sorted section.
void mergeByPriority(vector<StudyTask>& tasks, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<StudyTask> L(n1);
    vector<StudyTask> R(n2);

    // Copy data into temporary arrays.
    for (int i = 0; i < n1; i++) L[i] = tasks[left + i];
    for (int j = 0; j < n2; j++) R[j] = tasks[mid + 1 + j];

    int i = 0;
    int j = 0;
    int k = left;

    // Merge tasks in descending order of priority.
    while (i < n1 && j < n2) {
        if (betterForSorting(L[i], R[j])) {
            tasks[k++] = L[i++];
        } else {
            tasks[k++] = R[j++];
        }
    }

    // Copy remaining elements, if any.
    while (i < n1) tasks[k++] = L[i++];
    while (j < n2) tasks[k++] = R[j++];
}

// Recursive Merge Sort implementation for task ranking.
void mergeSortByPriority(vector<StudyTask>& tasks, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;
    mergeSortByPriority(tasks, left, mid);
    mergeSortByPriority(tasks, mid + 1, right);
    mergeByPriority(tasks, left, mid, right);
}

// Module 2: Sorting / Divide-and-Conquer.
// This function ranks tasks using Merge Sort and returns the ranked feasible task list.
SelectionResult runSorting(const Scenario& scenario) {
    clock_t startTime = clock();

    // Work on a copy so that the original scenario task order is not changed.
    vector<StudyTask> tasks = scenario.tasks;
    if (!tasks.empty()) {
        mergeSortByPriority(tasks, 0, static_cast<int>(tasks.size()) - 1);
    }

    vector<string> selectedTaskIds;
    double totalStudyTime = 0.0;
    int totalImportance = 0;

    // Record tasks in ranked order until the available study time is reached.
    for (const StudyTask& task : tasks) {
        if (totalStudyTime + task.studyTime <= scenario.totalAvailableTime + 1e-9) {
            selectedTaskIds.push_back(task.id);
            totalStudyTime += task.studyTime;
            totalImportance += task.importance;
        }
    }

    clock_t endTime = clock();

    // Store the result in a common format for performance comparison.
    SelectionResult result;
    result.algorithmName = "Sorting-based Ranking";
    result.selectedTaskIds = selectedTaskIds;
    result.totalStudyTime = totalStudyTime;
    result.totalImportance = totalImportance;
    result.executionTime = static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC;
    result.description = "Merge Sort ranks tasks by priority score.";

    return result;
}

// ============================================================================
// Module 3: Greedy Planning Module
// ============================================================================

/*
 * Greedy rule:
 *   greedy score = importance / weighted cost
 *   weighted cost = 0.55 * study time + 0.30 * deadline + 0.15 * difficulty
 *
 * This gives a quick study plan by selecting locally attractive tasks first.
 * Time complexity:  O(n log n)
 * Space complexity: O(n)
 */

double greedyScore(const StudyTask& task) {
    double weightedCost = 0.55 * task.studyTime
                        + 0.30 * task.deadline
                        + 0.15 * task.difficulty;

    // Prevent division by zero.
    if (weightedCost <= 0.0) return numeric_limits<double>::infinity();

    return static_cast<double>(task.importance) / weightedCost;
}

// Compare two tasks according to the greedy score.
// If scores are equal, use deadline, importance, and study time as tie-breakers.
bool betterForGreedy(const StudyTask& a, const StudyTask& b) {
    double sa = greedyScore(a);
    double sb = greedyScore(b);

    if (fabs(sa - sb) > 1e-9) return sa > sb;
    if (a.deadline != b.deadline) return a.deadline < b.deadline;
    if (a.importance != b.importance) return a.importance > b.importance;
    return a.studyTime < b.studyTime;
}

// Module 3: Greedy Task Selection.
// Select study tasks according to the highest greedy score.
SelectionResult runGreedy(const Scenario& scenario) {
    clock_t startTime = clock();

    // Copy the original task list for sorting.
    vector<StudyTask> tasks = scenario.tasks;

    // Rank tasks using the greedy comparator.
    sort(tasks.begin(), tasks.end(), betterForGreedy);

    vector<string> selectedTaskIds;
    double totalStudyTime = 0.0;
    int totalImportance = 0;

    // Select tasks until the available study time is fully used.
    for (const StudyTask& task : tasks) {
        if (totalStudyTime + task.studyTime <= scenario.totalAvailableTime + 1e-9) {
            selectedTaskIds.push_back(task.id);
            totalStudyTime += task.studyTime;
            totalImportance += task.importance;
        }
    }

    clock_t endTime = clock();

    // Store the result in a common format for algorithm comparison.
    SelectionResult result;
    result.algorithmName = "Greedy Strategy";
    result.selectedTaskIds = selectedTaskIds;
    result.totalStudyTime = totalStudyTime;
    result.totalImportance = totalImportance;
    result.executionTime = static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC;
    result.description = "Greedy selects tasks by highest importance-to-weighted-cost score.";

    return result;
}

// ============================================================================
// Module 4: Dynamic Programming using 0/1 Knapsack
// ============================================================================

/*
 * Available study time is treated as the knapsack capacity.
 * Study time is treated as weight.
 * Importance score is treated as value.
 *
 * TIME_SCALE is used so decimal study times such as 1.5h are not truncated.
 * Time complexity:  O(nW)
 * Space complexity: O(nW)
 */

// Solve the task selection problem as a 0/1 Knapsack problem.
SelectionResult runDynamicProgramming(const Scenario& scenario) {
    clock_t startTime = clock();

    // Copy tasks from the current scenario.
    vector<StudyTask> tasks = scenario.tasks;
    int n = static_cast<int>(tasks.size());

    // Convert available study time into integer units for DP table indexing.
    int capacity = timeToUnits(scenario.totalAvailableTime);

    // Store each task's study time as an integer weight.
    vector<int> weights(n);
    for (int i = 0; i < n; i++) {
        weights[i] = timeToUnits(tasks[i].studyTime);
    }

    // dp[i][w] represents the maximum importance using the first i tasks
    // with available time capacity w.
    vector<vector<int>> dp(n + 1, vector<int>(capacity + 1, 0));

    // Build the DP table using the 0/1 Knapsack transition.
    for (int i = 1; i <= n; i++) {
        int taskTime = weights[i - 1];
        int taskImportance = tasks[i - 1].importance;

        for (int w = 0; w <= capacity; w++) {
            // Case 1: do not select the current task.
            dp[i][w] = dp[i - 1][w];

            // Case 2: select the current task if enough time is available.
            if (taskTime <= w) {
                dp[i][w] = max(
                    dp[i][w],
                    dp[i - 1][w - taskTime] + taskImportance
                );
            }
        }
    }

    vector<string> selectedTaskIds;
    double totalStudyTime = 0.0;
    int totalImportance = dp[n][capacity];

    // Backtrack through the DP table to find which tasks were selected.
    int w = capacity;
    for (int i = n; i > 0; i--) {
        if (dp[i][w] != dp[i - 1][w]) {
            selectedTaskIds.push_back(tasks[i - 1].id);
            totalStudyTime += tasks[i - 1].studyTime;
            w -= weights[i - 1];
        }
    }

    // Reverse the selected task list to restore the original task order.
    reverse(selectedTaskIds.begin(), selectedTaskIds.end());

    clock_t endTime = clock();

    // Store the result in a common format for comparison with other modules.
    SelectionResult result;
    result.algorithmName = "Dynamic Programming";
    result.selectedTaskIds = selectedTaskIds;
    result.totalStudyTime = totalStudyTime;
    result.totalImportance = totalImportance;
    result.executionTime = static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC;
    result.description = "0/1 Knapsack DP maximizes total importance under the time limit.";

    return result;
}

// ============================================================================
// Module 5: k-NN Strategy Recommendation
// ============================================================================

Feat getFeatures(const Scenario& s) {
    Feat f;
    f.numTasks = static_cast<int>(s.tasks.size());
    f.totalReq = 0.0;
    f.avail = s.totalAvailableTime;
    f.pressure = 0.0;
    f.avgImp = 0.0;
    f.deadlineTight = 0.0;
    f.impVar = 0.0;
    f.avgDiff = 0.0;

    if (s.tasks.empty()) return f;

    double sumImp = 0.0;
    double sumSqImp = 0.0;
    double sumDiff = 0.0;
    int urgentCount = 0;

    for (const StudyTask& t : s.tasks) {
        f.totalReq += t.studyTime;
        sumImp += t.importance;
        sumSqImp += static_cast<double>(t.importance) * t.importance;
        sumDiff += t.difficulty;

        if (t.deadline <= 3) {
            urgentCount++;
        }
    }

    int n = f.numTasks;
    f.pressure = (f.avail > 0.0) ? f.totalReq / f.avail : 0.0;
    f.avgImp = sumImp / n;
    f.deadlineTight = static_cast<double>(urgentCount) / n;
    f.avgDiff = sumDiff / n;

    double variance = sumSqImp / n - f.avgImp * f.avgImp;
    if (variance < 0.0) variance = 0.0;
    f.impVar = (f.avgImp > 0.0) ? sqrt(variance) / f.avgImp : 0.0;

    return f;
}

int labelByFeatureRule(const Feat& f) {
    // 0 = Sorting, 1 = Greedy, 2 = DP
    // DP: suitable when time is limited and optimal selection matters.
    if (f.pressure >= 1.80) {
        return DP;
    }

    // Greedy: suitable when many tasks are urgent and a fast practical plan is needed.
    if (f.deadlineTight >= 0.45) {
        return GREEDY;
    }

    // Sorting: suitable when the main purpose is prioritisation.
    if (f.impVar >= 0.25 || f.pressure <= 1.20) {
        return SORTING;
    }

    // Balanced medium-pressure cases can reasonably use Greedy.
    return GREEDY;
}

int expectedStrategyLabel(const Scenario& s) {
    // For the four designed scenarios, the label follows the scenario purpose.
    // This avoids the old problem where every label became DP just because DP has
    // the highest total importance in a knapsack objective.
    string name = toLowerStr(s.name);

    if (name.find("high") != string::npos) {
        return DP;
    }

    if (name.find("deadline") != string::npos) {
        return GREEDY;
    }

    if (name.find("importance") != string::npos) {
        return SORTING;
    }

    if (name.find("low") != string::npos) {
        return SORTING;
    }

    return labelByFeatureRule(getFeatures(s));
}

Scenario makeVariant(const Scenario& s, double availableTimeMultiplier, int variantNo) {
    Scenario v = s;
    v.totalAvailableTime = max(1.0, s.totalAvailableTime * availableTimeMultiplier);
    v.name = s.name + " (training variant " + to_string(variantNo) + ")";
    return v;
}

vector<Sample> genTrainingData(const vector<Scenario>& baseScenarios) {
    vector<Sample> data;

    // 4 base scenarios * 3 versions = 12 labelled examples.
    // This fits the lightweight AI/ML requirement and gives all 3 strategy classes.
    vector<double> multipliers = {1.00, 0.80, 1.20};

    for (const Scenario& s : baseScenarios) {
        for (size_t i = 0; i < multipliers.size(); i++) {
            Scenario v = makeVariant(s, multipliers[i], static_cast<int>(i + 1));
            Feat fv = getFeatures(v);
            int label = expectedStrategyLabel(v);
            data.push_back({fv, label});
        }
    }

    return data;
}

// Normalised Euclidean distance. Without normalisation, large-valued features
// such as total required time can dominate all other features.
double featureDistance(const Feat& a, const Feat& b) {
    double d2 = 0.0;

    d2 += pow((a.numTasks - b.numTasks) / 15.0, 2);
    d2 += pow((a.totalReq - b.totalReq) / 60.0, 2);
    d2 += pow((a.avail - b.avail) / 60.0, 2);
    d2 += pow((a.pressure - b.pressure) / 3.0, 2);
    d2 += pow((a.avgImp - b.avgImp) / 10.0, 2);
    d2 += pow(a.deadlineTight - b.deadlineTight, 2);
    d2 += pow(a.impVar - b.impVar, 2);
    d2 += pow((a.avgDiff - b.avgDiff) / 5.0, 2);

    return sqrt(d2);
}

int knnPredict(const Feat& query, const vector<Sample>& train, int k) {
    vector<pair<double, int>> dists;

    for (int i = 0; i < static_cast<int>(train.size()); i++) {
        dists.push_back({featureDistance(query, train[i].features), i});
    }

    sort(dists.begin(), dists.end());

    int votes[3] = {0, 0, 0};
    double distSum[3] = {0.0, 0.0, 0.0};

    int lim = min(k, static_cast<int>(dists.size()));
    for (int i = 0; i < lim; i++) {
        int label = train[dists[i].second].label;
        votes[label]++;
        distSum[label] += dists[i].first;
    }

    int best = 0;
    for (int label = 1; label < 3; label++) {
        if (votes[label] > votes[best]) {
            best = label;
        } else if (votes[label] == votes[best] && votes[label] > 0) {
            if (distSum[label] < distSum[best]) {
                best = label;
            }
        }
    }

    return best;
}

// ============================================================================
// Module 6: Performance comparison
// ============================================================================

vector<int> highestImportanceWinners(const SelectionResult& a,
                                     const SelectionResult& b,
                                     const SelectionResult& c) {
    int maxImp = max(a.totalImportance, max(b.totalImportance, c.totalImportance));
    vector<int> winners;

    if (a.totalImportance == maxImp) winners.push_back(SORTING);
    if (b.totalImportance == maxImp) winners.push_back(GREEDY);
    if (c.totalImportance == maxImp) winners.push_back(DP);

    return winners;
}

string winnerListToString(const vector<int>& winners) {
    string out;
    for (size_t i = 0; i < winners.size(); i++) {
        out += strategyName(winners[i]);
        if (i + 1 < winners.size()) out += ", ";
    }
    return out;
}

void showSelectedIds(const SelectionResult& r) {
    cout << "    " << left << setw(24) << r.algorithmName
         << ": " << joinIds(r.selectedTaskIds) << "\n";
}

void showCompare(const Scenario& s) {
    SelectionResult sr = runSorting(s);
    SelectionResult gr = runGreedy(s);
    SelectionResult dr = runDynamicProgramming(s);

    cout << "\n  " << s.name;
    cout << "  (avail=" << fixed << setprecision(1) << s.totalAvailableTime
         << "h, tasks=" << s.tasks.size() << ")\n";

    cout << "  " << string(140, '-') << "\n";
    cout << left
         << setw(30) << "  Strategy"
         << setw(10) << "Selected"
         << setw(10) << "Time(h)"
         << setw(13) << "Importance"
         << setw(12) << "Exec(s)"
         << "Note\n";
    cout << "  " << string(140, '-') << "\n";

    auto row = [](const SelectionResult& r) {
        cout << left
             << setw(30) << ("  " + r.algorithmName)
             << setw(10) << r.selectedTaskIds.size()
             << setw(10) << fixed << setprecision(1) << r.totalStudyTime
             << setw(13) << r.totalImportance
             << setw(12) << fixed << setprecision(6) << r.executionTime
             << r.description << "\n";
    };

    row(sr);
    row(gr);
    row(dr);

    cout << "  " << string(140, '-') << "\n";

    vector<int> winners = highestImportanceWinners(sr, gr, dr);
    cout << "  -> Highest total importance: " << winnerListToString(winners) << "\n";
    cout << "  -> Scenario-suitability label: " << strategyFullName(expectedStrategyLabel(s)) << "\n";

    cout << "\n  Selected task IDs:\n";
    showSelectedIds(sr);
    showSelectedIds(gr);
    showSelectedIds(dr);
}

void showFeatureSummary(const Scenario& s) {
    Feat f = getFeatures(s);

    cout << "    totalReq=" << fixed << setprecision(1) << f.totalReq
         << "h, avail=" << f.avail
         << "h, pressure=" << setprecision(2) << f.pressure
         << ", avgImp=" << setprecision(2) << f.avgImp
         << ", urgentRatio=" << setprecision(2) << f.deadlineTight
         << ", impCV=" << setprecision(2) << f.impVar
         << ", avgDiff=" << setprecision(2) << f.avgDiff
         << "\n";
}

// ============================================================================
// Main program
// ============================================================================

int main() {
    cout << "\n===== StudySmart AI =====\n";
    cout << "  CST207 Design and Analysis of Algorithms\n";
    cout << "  Group: [your group name]\n\n";

    vector<Scenario> scenarios = loadAllScenarios();

    if (scenarios.empty()) {
        cout << "No scenario CSV files were loaded.\n";
        cout << "Please put scenario_1.csv to scenario_4.csv in the same folder as this program.\n";
        return 1;
    }

    cout << "Read " << scenarios.size() << " scenarios from csv files.\n";

    cout << "\n========== Scenario Feature Summary ==========\n";
    for (const Scenario& s : scenarios) {
        cout << "  " << s.name << "\n";
        showFeatureSummary(s);
    }

    cout << "\n========== Strategy Performance Comparison ==========";
    for (const Scenario& s : scenarios) {
        showCompare(s);
    }

    cout << "\n\n========== k-NN Strategy Recommendation ==========\n\n";
    cout << "  Generating labelled training data from scenario features...\n";

    vector<Sample> trainData = genTrainingData(scenarios);
    cout << "  Total training samples: " << trainData.size() << "\n";

    int cnt[3] = {0, 0, 0};
    for (const Sample& sample : trainData) {
        cnt[sample.label]++;
    }

    cout << "  Label breakdown: Sorting=" << cnt[SORTING]
         << "  Greedy=" << cnt[GREEDY]
         << "  DP=" << cnt[DP] << "\n\n";

    const int K = 3;
    int correct = 0;

    cout << "  " << left << setw(45) << "Scenario"
         << setw(18) << "Predicted"
         << setw(18) << "Expected"
         << "Match\n";
    cout << "  " << string(89, '-') << "\n";

    for (const Scenario& s : scenarios) {
        Feat f = getFeatures(s);
        int pred = knnPredict(f, trainData, K);
        int expected = expectedStrategyLabel(s);

        bool match = (pred == expected);
        if (match) correct++;

        cout << left
             << "  " << setw(45) << s.name
             << setw(18) << strategyName(pred)
             << setw(18) << strategyName(expected)
             << (match ? "YES" : "NO") << "\n";
    }

    cout << "  " << string(89, '-') << "\n";
    cout << "  Accuracy: " << correct << "/" << scenarios.size()
         << " (" << fixed << setprecision(0)
         << (100.0 * correct / scenarios.size()) << "%)\n";

    return 0;
}
