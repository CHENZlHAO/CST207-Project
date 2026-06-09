/**
 * ============================================================================
 * StudySmart AI - Smart Academic Planning Application
 * Course: CST207 Design and Analysis of Algorithms
 * Group: [Your Group Name]
 * ============================================================================
 *
 * Description:
 *   StudySmart AI simulates a smart academic planning application. The system
 *   receives a set of study tasks and helps the student decide which tasks
 *   should be prioritized and which planning strategy is most suitable.
 *
 * Algorithms Implemented:
 *   1. Merge Sort (Divide & Conquer)    - Rank tasks by deadline/importance
 *   2. Greedy Algorithm                 - Select tasks by urgency/importance
 *   3. Dynamic Programming (Knapsack)   - Optimize task selection under time limit
 *   4. k-Nearest Neighbours (AI/ML)    - Recommend the best strategy
 *
 * Compilation:
 *   g++ -std=c++11 -o StudySmartAI StudySmartAI.cpp
 *
 * Execution:
 *   ./StudySmartAI
 * ============================================================================
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <limits>
#include <cstdlib>
#include <ctime>

using namespace std;

// ============================================================================
// Data Structures
// ============================================================================

/**
 * StudyTask - Represents a single study task with all relevant attributes.
 */
struct StudyTask {
    int    id;                // Unique task identifier
    string name;              // Task name / topic
    double studyTime;         // Estimated study time (hours)
    int    importance;        // Importance score (1-10)
    int    deadline;          // Deadline / urgency (days until due)
    int    difficulty;        // Difficulty level (1-5)
    string taskType;          // Lecture, Tutorial, Assignment, Practice, Revision

    // Computed attributes
    double urgencyRatio;      // importance / deadline
    double efficiency;        // importance / studyTime

    StudyTask() : id(0), name(""), studyTime(0), importance(0),
                  deadline(1), difficulty(1), taskType(""),
                  urgencyRatio(0), efficiency(0) {}

    StudyTask(int _id, string _name, double _time, int _imp,
              int _dead, int _diff, string _type)
        : id(_id), name(_name), studyTime(_time), importance(_imp),
          deadline(_dead), difficulty(_diff), taskType(_type) {
        urgencyRatio = (deadline > 0) ? (double)importance / deadline : importance;
        efficiency   = (studyTime > 0) ? (double)importance / studyTime : 0;
    }

    void display() const {
        cout << left << setw(5)  << id
             << setw(32) << name.substr(0, 30)
             << setw(10) << studyTime
             << setw(12) << importance
             << setw(10) << deadline
             << setw(12) << difficulty
             << setw(14) << taskType
             << setw(10) << fixed << setprecision(2) << urgencyRatio
             << setw(10) << fixed << setprecision(2) << efficiency
             << endl;
    }
};

/**
 * Scenario - Contains a set of tasks and total available study time.
 */
struct Scenario {
    string name;
    vector<StudyTask> tasks;
    double totalAvailableTime;

    Scenario(string _name, double _time) : name(_name), totalAvailableTime(_time) {}
};

/**
 * SelectionResult - Stores the output of a task selection algorithm.
 */
struct SelectionResult {
    string algorithmName;
    vector<int> selectedTaskIds;
    double totalStudyTime;
    int totalImportance;
    double executionTime;   // in milliseconds (simulated)
    string description;

    SelectionResult(string name) : algorithmName(name), totalStudyTime(0),
                                   totalImportance(0), executionTime(0) {}
};

// ============================================================================
// Module 1: Task Scenario Generation / Input
// ============================================================================

/**
 * Generates pre-defined study task scenarios.
 * Each group must test at least two different scenarios.
 */
vector<Scenario> generateScenarios() {
    vector<Scenario> scenarios;

    // ----------------------------------------------------------------
    // Scenario 1: Low-pressure - enough time for most tasks
    // ----------------------------------------------------------------
    Scenario lowPress("Low-Pressure Scenario (Enough Time)", 25.0);
    lowPress.tasks = {
        StudyTask(1,  "Revise Dynamic Programming",    3.0, 5, 2, 4, "Lecture"),
        StudyTask(2,  "Complete Greedy Algorithm HW",  2.0, 4, 3, 3, "Assignment"),
        StudyTask(3,  "Practice Sorting Problems",     2.5, 3, 5, 2, "Practice"),
        StudyTask(4,  "Read Chapter 7: Graph Theory",  4.0, 4, 4, 3, "Lecture"),
        StudyTask(5,  "Prepare Divide & Conquer Quiz", 1.5, 5, 1, 4, "Revision"),
        StudyTask(6,  "Finish DP Problem Set",         3.5, 5, 2, 5, "Assignment"),
        StudyTask(7,  "Review Lecture Notes Week 1-4", 2.0, 2, 7, 2, "Revision"),
        StudyTask(8,  "Tutorial on Complexity Analysis",1.5, 3, 3, 2, "Tutorial"),
        StudyTask(9,  "Practice Backtracking Problems", 3.0, 4, 6, 4, "Practice"),
        StudyTask(10, "Read Supplementary Materials",   2.0, 2, 8, 1, "Lecture"),
    };
    scenarios.push_back(lowPress);

    // ----------------------------------------------------------------
    // Scenario 2: High-pressure - limited time, must prioritise
    // ----------------------------------------------------------------
    Scenario highPress("High-Pressure Scenario (Limited Time)", 10.0);
    highPress.tasks = {
        StudyTask(1,  "Revise Dynamic Programming",    3.0, 5, 1, 4, "Lecture"),
        StudyTask(2,  "Complete Greedy Algorithm HW",  2.0, 4, 2, 3, "Assignment"),
        StudyTask(3,  "Practice Sorting Problems",     2.5, 3, 2, 2, "Practice"),
        StudyTask(4,  "Read Chapter 7: Graph Theory",  4.0, 4, 3, 3, "Lecture"),
        StudyTask(5,  "Prepare Divide & Conquer Quiz", 1.5, 5, 1, 4, "Revision"),
        StudyTask(6,  "Finish DP Problem Set",         3.5, 5, 2, 5, "Assignment"),
        StudyTask(7,  "Review Lecture Notes Week 1-4", 2.0, 2, 5, 2, "Revision"),
        StudyTask(8,  "Tutorial on Complexity Analysis",1.5, 3, 4, 2, "Tutorial"),
    };
    scenarios.push_back(highPress);

    return scenarios;
}

/**
 * Displays the task table for a given scenario.
 */
void displayScenario(const Scenario& sc) {
    cout << "\n================================================================================" << endl;
    cout << "  Scenario: " << sc.name << endl;
    cout << "  Total Available Study Time: " << sc.totalAvailableTime << " hours" << endl;
    cout << "================================================================================\n" << endl;

    cout << left << setw(5)  << "ID"
         << setw(32) << "Task Name"
         << setw(10) << "Time(h)"
         << setw(12) << "Importance"
         << setw(10) << "Deadline"
         << setw(12) << "Difficulty"
         << setw(14) << "Type"
         << setw(10) << "UrgencyR"
         << setw(10) << "Efficiency"
         << endl;
    cout << string(115, '-') << endl;

    for (const auto& task : sc.tasks) {
        task.display();
    }
    cout << string(115, '-') << endl;
}

// ============================================================================
// Module 2: Sorting / Divide-and-Conquer (Merge Sort)
// ============================================================================

/**
 * Merge Sort implementation to rank tasks by urgency ratio (importance / deadline)
 * in descending order. (Divide-and-Conquer approach)
 *
 * Time Complexity: O(n log n)
 * Space Complexity: O(n)
 */

void merge(vector<StudyTask>& tasks, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<StudyTask> L(tasks.begin() + left, tasks.begin() + mid + 1);
    vector<StudyTask> R(tasks.begin() + mid + 1, tasks.begin() + right + 1);

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i].urgencyRatio >= R[j].urgencyRatio) {
            tasks[k++] = L[i++];
        } else {
            tasks[k++] = R[j++];
        }
    }
    while (i < n1) tasks[k++] = L[i++];
    while (j < n2) tasks[k++] = R[j++];
}

void mergeSort(vector<StudyTask>& tasks, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(tasks, left, mid);
        mergeSort(tasks, mid + 1, right);
        merge(tasks, left, mid, right);
    }
}

/**
 * Sorts tasks by urgency ratio (importance / deadline) descending using Merge Sort.
 */
vector<StudyTask> rankTasksByMergeSort(const vector<StudyTask>& input) {
    vector<StudyTask> sorted = input;
    mergeSort(sorted, 0, sorted.size() - 1);
    return sorted;
}

/**
 * Displays the ranked task list.
 */
void displayRankedTasks(const vector<StudyTask>& tasks, const string& criterion) {
    cout << "\n--- Tasks Ranked by " << criterion << " (Merge Sort) ---\n" << endl;
    cout << left << setw(5)  << "Rank"
         << setw(5)  << "ID"
         << setw(32) << "Task Name"
         << setw(10) << "Time(h)"
         << setw(12) << "Importance"
         << setw(10) << "Deadline"
         << setw(10) << "UrgencyR"
         << endl;
    cout << string(84, '-') << endl;

    for (size_t i = 0; i < tasks.size(); ++i) {
        cout << left << setw(5)  << i + 1
             << setw(5)  << tasks[i].id
             << setw(32) << tasks[i].name.substr(0, 30)
             << setw(10) << tasks[i].studyTime
             << setw(12) << tasks[i].importance
             << setw(10) << tasks[i].deadline
             << setw(10) << fixed << setprecision(2) << tasks[i].urgencyRatio
             << endl;
    }
    cout << string(84, '-') << endl;
}

// ============================================================================
// Module 3: Greedy Algorithm - Task Selection by Efficiency
// ============================================================================

/**
 * Greedy algorithm that selects tasks based on efficiency (importance / time)
 * until available time is exhausted.
 *
 * Time Complexity: O(n log n) for sorting, O(n) for selection
 * Space Complexity: O(n)
 */
SelectionResult greedyTaskSelection(const vector<StudyTask>& tasks, double timeLimit) {
    SelectionResult result("Greedy (Efficiency-Based)");
    clock_t start = clock();

    // Sort tasks by efficiency descending
    vector<StudyTask> sorted = tasks;
    sort(sorted.begin(), sorted.end(), [](const StudyTask& a, const StudyTask& b) {
        return a.efficiency > b.efficiency;
    });

    double timeUsed = 0;
    for (const auto& task : sorted) {
        if (timeUsed + task.studyTime <= timeLimit) {
            result.selectedTaskIds.push_back(task.id);
            result.totalStudyTime += task.studyTime;
            result.totalImportance += task.importance;
            timeUsed += task.studyTime;
        }
    }

    clock_t end = clock();
    result.executionTime = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    result.description = "Selected tasks with highest importance-per-hour ratio";

    return result;
}

// ============================================================================
// Module 4: Dynamic Programming (0/1 Knapsack) - Optimised Task Selection
// ============================================================================

/**
 * Dynamic Programming (0/1 Knapsack) to select tasks maximising total importance
 * under a given study time constraint.
 *
 * Time Complexity: O(n * W) where n = number of tasks, W = time limit
 * Space Complexity: O(n * W)
 */
SelectionResult dpTaskSelection(const vector<StudyTask>& tasks, double timeLimit) {
    SelectionResult result("Dynamic Programming (0/1 Knapsack)");
    clock_t start = clock();

    int n = tasks.size();
    // Convert to integer "units" (multiply by 2 for 0.5-hour precision)
    int W = (int)(timeLimit * 2);
    vector<int> weights(n);
    for (int i = 0; i < n; ++i) {
        weights[i] = (int)(tasks[i].studyTime * 2);
    }

    // DP table: dp[i][w] = max importance using first i tasks with weight <= w
    vector<vector<int>> dp(n + 1, vector<int>(W + 1, 0));

    for (int i = 1; i <= n; ++i) {
        for (int w = 0; w <= W; ++w) {
            if (weights[i - 1] <= w) {
                dp[i][w] = max(dp[i - 1][w],
                               dp[i - 1][w - weights[i - 1]] + tasks[i - 1].importance);
            } else {
                dp[i][w] = dp[i - 1][w];
            }
        }
    }

    // Backtrack to find selected tasks
    int w = W;
    for (int i = n; i > 0 && w > 0; --i) {
        if (dp[i][w] != dp[i - 1][w]) {
            result.selectedTaskIds.push_back(tasks[i - 1].id);
            result.totalStudyTime += tasks[i - 1].studyTime;
            result.totalImportance += tasks[i - 1].importance;
            w -= weights[i - 1];
        }
    }

    clock_t end = clock();
    result.executionTime = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    result.description = "Selected tasks maximising total importance within time limit (0/1 Knapsack)";

    return result;
}

// ============================================================================
// Module 5: AI/ML Module - k-Nearest Neighbours (k-NN) Strategy Recommender
// ============================================================================

/**
 * Training data for the k-NN model.
 * Each entry represents a scenario with features:
 *   [totalTasks, avgImportance, avgDeadline, avgDifficulty, totalTimeRatio]
 * and the label is the best strategy: 0 = Greedy, 1 = DP
 */
struct TrainingSample {
    vector<double> features;
    int label;        // 0 = Greedy, 1 = DP
    string labelStr() const { return (label == 0) ? "Greedy" : "DP"; }
};

/**
 * k-Nearest Neighbours classifier.
 *
 * Features extracted from scenario:
 *   - Number of tasks
 *   - Average importance
 *   - Average deadline urgency
 *   - Average difficulty
 *   - Total required time / available time ratio
 */
class KNNClassifier {
private:
    vector<TrainingSample> trainingData;
    int k;

    /**
     * Euclidean distance between two feature vectors.
     */
    double euclideanDistance(const vector<double>& a, const vector<double>& b) const {
        double sum = 0;
        for (size_t i = 0; i < a.size(); ++i) {
            sum += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return sqrt(sum);
    }

public:
    KNNClassifier(int _k = 3) : k(_k) {}

    /**
     * Train the model by storing labelled samples.
     */
    void train(const vector<TrainingSample>& data) {
        trainingData = data;
    }

    /**
     * Predict the best strategy for a given feature vector.
     * Returns 0 for Greedy, 1 for DP.
     */
    int predict(const vector<double>& features) const {
        if (trainingData.empty()) return 0;

        // Calculate distances to all training samples
        vector<pair<double, int>> distances; // <distance, label>
        for (const auto& sample : trainingData) {
            double dist = euclideanDistance(features, sample.features);
            distances.push_back({dist, sample.label});
        }

        // Sort by distance (ascending)
        sort(distances.begin(), distances.end());

        // Vote among k nearest neighbours
        int voteGreedy = 0, voteDP = 0;
        int neighbours = min(k, (int)distances.size());
        for (int i = 0; i < neighbours; ++i) {
            if (distances[i].second == 0) voteGreedy++;
            else voteDP++;
        }

        return (voteGreedy >= voteDP) ? 0 : 1;
    }

    /**
     * Predict with confidence score (proportion of neighbours agreeing).
     */
    pair<int, double> predictWithConfidence(const vector<double>& features) const {
        if (trainingData.empty()) return {0, 1.0};

        vector<pair<double, int>> distances;
        for (const auto& sample : trainingData) {
            double dist = euclideanDistance(features, sample.features);
            distances.push_back({dist, sample.label});
        }

        sort(distances.begin(), distances.end());

        int voteGreedy = 0, voteDP = 0;
        int neighbours = min(k, (int)distances.size());
        for (int i = 0; i < neighbours; ++i) {
            if (distances[i].second == 0) voteGreedy++;
            else voteDP++;
        }

        int prediction = (voteGreedy >= voteDP) ? 0 : 1;
        double confidence = max(voteGreedy, voteDP) / (double)neighbours;

        return {prediction, confidence};
    }
};

/**
 * Extracts feature vector from a scenario for k-NN prediction.
 */
vector<double> extractFeatures(const Scenario& sc) {
    vector<double> features;
    int n = sc.tasks.size();
    if (n == 0) return {0, 0, 0, 0, 0};

    double totalTime = 0, totalImp = 0, totalDead = 0, totalDiff = 0;
    for (const auto& t : sc.tasks) {
        totalTime += t.studyTime;
        totalImp += t.importance;
        totalDead += t.deadline;
        totalDiff += t.difficulty;
    }

    features.push_back(n);                              // Number of tasks
    features.push_back(totalImp / n);                   // Average importance
    features.push_back(totalDead / n);                  // Average deadline
    features.push_back(totalDiff / n);                  // Average difficulty
    features.push_back(totalTime / sc.totalAvailableTime); // Time pressure ratio

    return features;
}

/**
 * Generates synthetic training data for the k-NN model.
 * Simulates known outcomes based on time pressure ratio.
 */
vector<TrainingSample> generateTrainingData() {
    vector<TrainingSample> data;

    // When time pressure is low (ratio < 1.5), DP is usually better (optimal)
    // When time pressure is high (ratio > 2.0), Greedy is faster and good enough
    for (int i = 0; i < 20; ++i) {
        TrainingSample s;
        s.features = {
            (double)(5 + rand() % 6),           // tasks: 5-10
            (double)(3 + (rand() % 4)),          // avg importance: 3-6
            (double)(1 + (rand() % 5)),          // avg deadline: 1-5
            (double)(2 + (rand() % 3)),          // avg difficulty: 2-4
            0.8 + (rand() % 5) * 0.3            // time ratio: 0.8-2.0
        };
        s.label = (s.features[4] < 1.5) ? 1 : 0; // DP for low pressure, Greedy for high
        data.push_back(s);
    }

    return data;
}

// ============================================================================
// Module 6: Performance Measurement & Strategy Comparison
// ============================================================================

/**
 * Displays the selection result and compares with alternatives.
 */
void displaySelectionResult(const SelectionResult& result, const vector<StudyTask>& allTasks) {
    cout << "\n--- " << result.algorithmName << " ---" << endl;
    cout << "  " << result.description << endl;
    cout << "  Execution Time: " << fixed << setprecision(3) << result.executionTime << " ms" << endl;
    cout << "  Tasks Selected: ";
    for (size_t i = 0; i < result.selectedTaskIds.size(); ++i) {
        cout << "T" << result.selectedTaskIds[i];
        if (i < result.selectedTaskIds.size() - 1) cout << ", ";
    }
    cout << endl;
    cout << "  Total Study Time: " << result.totalStudyTime << " hours" << endl;
    cout << "  Total Importance: " << result.totalImportance << endl;
}

/**
 * Displays the comparison table for all strategies.
 */
void displayComparisonTable(const Scenario& sc,
                            const SelectionResult& greedy,
                            const SelectionResult& dp) {
    cout << "\n================================================================================" << endl;
    cout << "  Strategy Comparison Table" << endl;
    cout << "  Scenario: " << sc.name << endl;
    cout << "================================================================================\n" << endl;

    cout << left << setw(30) << "Metric"
         << setw(25) << "Greedy"
         << setw(25) << "DP (Knapsack)"
         << endl;
    cout << string(80, '-') << endl;

    cout << left << setw(30) << "Tasks Selected"
         << setw(25) << greedy.selectedTaskIds.size()
         << setw(25) << dp.selectedTaskIds.size()
         << endl;

    cout << left << setw(30) << "Total Study Time (hours)"
         << setw(25) << fixed << setprecision(2) << greedy.totalStudyTime
         << setw(25) << fixed << setprecision(2) << dp.totalStudyTime
         << endl;

    cout << left << setw(30) << "Total Importance"
         << setw(25) << greedy.totalImportance
         << setw(25) << dp.totalImportance
         << endl;

    cout << left << setw(30) << "Execution Time (ms)"
         << setw(25) << fixed << setprecision(3) << greedy.executionTime
         << setw(25) << fixed << setprecision(3) << dp.executionTime
         << endl;

    // Determine actual best strategy (higher importance = better)
    string bestStrategy;
    if (greedy.totalImportance > dp.totalImportance) {
        bestStrategy = "Greedy";
    } else if (dp.totalImportance > greedy.totalImportance) {
        bestStrategy = "DP";
    } else {
        // Tie-break: if same importance, prefer shorter execution time
        bestStrategy = (greedy.executionTime <= dp.executionTime) ? "Greedy" : "DP";
    }

    cout << left << setw(30) << "Actual Best Strategy"
         << setw(25) << bestStrategy
         << endl;
    cout << string(80, '-') << endl;
}

// ============================================================================
// Main Function
// ============================================================================

int main() {
    srand((unsigned int)time(NULL));

    cout << "\n=====================================================================================" << endl;
    cout << "  StudySmart AI - Smart Academic Planning Application" << endl;
    cout << "  CST207 Design and Analysis of Algorithms" << endl;
    cout << "  Group: [Your Group Name]" << endl;
    cout << "=====================================================================================\n" << endl;

    // ------------------------------------------------------------------
    // Module 1: Generate and display scenarios
    // ------------------------------------------------------------------
    vector<Scenario> scenarios = generateScenarios();

    for (size_t s = 0; s < scenarios.size(); ++s) {
        Scenario& sc = scenarios[s];
        displayScenario(sc);

        // ------------------------------------------------------------------
        // Module 2: Merge Sort - Rank tasks by urgency ratio
        // ------------------------------------------------------------------
        vector<StudyTask> ranked = rankTasksByMergeSort(sc.tasks);
        displayRankedTasks(ranked, "Urgency Ratio (Importance / Deadline)");

        // ------------------------------------------------------------------
        // Modules 3 & 4: Greedy and DP task selection
        // ------------------------------------------------------------------
        SelectionResult greedyResult = greedyTaskSelection(sc.tasks, sc.totalAvailableTime);
        SelectionResult dpResult = dpTaskSelection(sc.tasks, sc.totalAvailableTime);

        displaySelectionResult(greedyResult, sc.tasks);
        displaySelectionResult(dpResult, sc.tasks);

        // ------------------------------------------------------------------
        // Module 6: Comparison table
        // ------------------------------------------------------------------
        displayComparisonTable(sc, greedyResult, dpResult);
    }

    // ------------------------------------------------------------------
    // Module 5: AI/ML - k-NN Strategy Recommendation
    // ------------------------------------------------------------------
    cout << "\n================================================================================" << endl;
    cout << "  AI/ML Module: k-Nearest Neighbours Strategy Recommendation" << endl;
    cout << "================================================================================\n" << endl;

    KNNClassifier knn(3);
    vector<TrainingSample> trainingData = generateTrainingData();
    knn.train(trainingData);

    cout << left << setw(5) << "Test"
         << setw(35) << "Scenario"
         << setw(20) << "Recommended"
         << setw(20) << "Actual Best"
         << setw(15) << "Match?"
         << setw(12) << "Confidence"
         << endl;
    cout << string(107, '-') << endl;

    int correctPredictions = 0;
    for (size_t s = 0; s < scenarios.size(); ++s) {
        Scenario& sc = scenarios[s];

        vector<double> features = extractFeatures(sc);
        auto [predicted, confidence] = knn.predictWithConfidence(features);
        string predictedStr = (predicted == 0) ? "Greedy" : "DP";

        // Determine actual best strategy
        SelectionResult gR = greedyTaskSelection(sc.tasks, sc.totalAvailableTime);
        SelectionResult dpR = dpTaskSelection(sc.tasks, sc.totalAvailableTime);

        string actualBest;
        if (gR.totalImportance > dpR.totalImportance) actualBest = "Greedy";
        else if (dpR.totalImportance > gR.totalImportance) actualBest = "DP";
        else actualBest = (gR.executionTime <= dpR.executionTime) ? "Greedy" : "DP";

        int actualLabel = (actualBest == "Greedy") ? 0 : 1;
        bool match = (predicted == actualLabel);
        if (match) correctPredictions++;

        cout << left << setw(5)  << (s + 1)
             << setw(35) << sc.name.substr(0, 33)
             << setw(20) << predictedStr
             << setw(20) << actualBest
             << setw(15) << (match ? "Yes" : "No")
             << setw(12) << fixed << setprecision(2) << confidence
             << endl;
    }
    cout << string(107, '-') << endl;
    cout << "  Prediction Accuracy: " << correctPredictions << "/" << scenarios.size()
         << " (" << fixed << setprecision(1) << (correctPredictions * 100.0 / scenarios.size()) << "%)" << endl;

    // ------------------------------------------------------------------
    // Feature summary for report
    // ------------------------------------------------------------------
    cout << "\n--- Extracted Features for Each Scenario ---\n" << endl;
    cout << left << setw(5)  << "No."
         << setw(35) << "Scenario"
         << setw(12) << "Tasks"
         << setw(16) << "Avg Importance"
         << setw(16) << "Avg Deadline"
         << setw(16) << "Avg Difficulty"
         << setw(16) << "Time Ratio"
         << endl;
    cout << string(111, '-') << endl;

    for (size_t s = 0; s < scenarios.size(); ++s) {
        vector<double> f = extractFeatures(scenarios[s]);
        cout << left << setw(5)  << (s + 1)
             << setw(35) << scenarios[s].name.substr(0, 33)
             << setw(12) << fixed << setprecision(1) << f[0]
             << setw(16) << fixed << setprecision(2) << f[1]
             << setw(16) << fixed << setprecision(2) << f[2]
             << setw(16) << fixed << setprecision(2) << f[3]
             << setw(16) << fixed << setprecision(2) << f[4]
             << endl;
    }
    cout << string(111, '-') << endl;

    cout << "\n=====================================================================================" << endl;
    cout << "  StudySmart AI - Execution Complete" << endl;
    cout << "=====================================================================================\n" << endl;

    return 0;
}
