/**
 * ============================================================================
 * StudySmart AI - Smart Academic Planning Application
 * Course: CST207 Design and Analysis of Algorithms
 * Group: 不知道啊
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
 *   3. Dynamic Programming (Knapsack)   - Optimise task selection under time limit
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

    /*
     * Computed attributes (to be implemented):
     *   - urgencyRatio  = importance / deadline
     *   - efficiency    = importance / studyTime
     */
};

/**
 * Scenario - Contains a set of tasks and total available study time.
 */
struct Scenario {
    string name;
    vector<StudyTask> tasks;
    double totalAvailableTime;
};

/**
 * SelectionResult - Stores the output of a task selection algorithm.
 */
struct SelectionResult {
    string algorithmName;
    vector<int> selectedTaskIds;
    double totalStudyTime;
    int totalImportance;
    double executionTime;
    string description;
};

// ============================================================================
// Module 1: Task Scenario Generation / Input
// ============================================================================

/*
 * Study-task base list (15 tasks covering 5 course areas):
 *
 * Task ID | Task Name                                       | Course/Area                | Type
 * --------+-------------------------------------------------+----------------------------+-----------
 * T1      | Attend Divide-and-Conquer Lecture               | Design and Analysis of ... | Lecture
 * T2      | Complete Greedy and DP Tutorial                 | Design and Analysis of ... | Tutorial
 * T3      | Practice Algorithm Midterm Questions             | Design and Analysis of ... | Practice
 * T4      | Finish StudySmart AI Algorithm Module            | Design and Analysis of ... | Assignment
 * T5      | Review ERD and Normalization Concepts            | Database                   | Revision
 * T6      | Complete SQL Access Control Lab                  | Database                   | Practice
 * T7      | Finish Database Project Report Section           | Database                   | Assignment
 * T8      | Attend Database Group Project Meeting            | Database                   | Assignment
 * T9      | Attend C++ File Handling Lecture                  | C++ Programming            | Lecture
 * T10     | Debug C++ Menu and Input Handling                | C++ Programming            | Practice
 * T11     | Complete C++ Pointer and Vector Exercises         | C++ Programming            | Tutorial
 * T12     | Revise Software Engineering Design Principles    | Software Engineering       | Revision
 * T13     | Practice Use Case and Class Diagram Questions     | Software Engineering       | Practice
 * T14     | Complete FYP Thesis Submission Draft             | FYP                        | Assignment
 * T15     | Prepare FYP Presentation and Supervisor Meeting  | FYP                        | Assignment
 */

/*
 * Scenario 1: Low-pressure / Balanced
 *   Available Time: 28h | Total Required: 32h | Time Pressure Ratio: 1.14
 *   Deadline Tightness: 0.00 | Importance Variation: 0.12 | Avg Difficulty: 2.8
 *
 * Task ID | Study Time | Importance | Deadline | Difficulty
 * --------+------------+------------+----------+------------
 * T1      | 3          | 7          | 6        | 3
 * T2      | 2          | 6          | 5        | 3
 * T3      | 2          | 7          | 5        | 3
 * T4      | 3          | 8          | 7        | 3
 * T5      | 2          | 7          | 6        | 3
 * T6      | 2          | 7          | 5        | 3
 * T7      | 2          | 7          | 6        | 3
 * T8      | 1          | 6          | 7        | 2
 * T9      | 2          | 7          | 7        | 3
 * T10     | 2          | 7          | 6        | 3
 * T11     | 2          | 7          | 8        | 3
 * T12     | 2          | 7          | 7        | 3
 * T13     | 2          | 7          | 6        | 3
 * T14     | 3          | 8          | 5        | 3
 * T15     | 2          | 6          | 8        | 2
 */

/*
 * Scenario 2: High-pressure
 *   Available Time: 22h | Total Required: 53h | Time Pressure Ratio: 2.41
 *   Deadline Tightness: 0.07 | Importance Variation: 0.12 | Avg Difficulty: 4.27
 *
 * Task ID | Study Time | Importance | Deadline | Difficulty
 * --------+------------+------------+----------+------------
 * T1      | 4          | 6          | 5        | 4
 * T2      | 3          | 7          | 3        | 4
 * T3      | 3          | 6          | 1        | 4
 * T4      | 4          | 8          | 4        | 5
 * T5      | 3          | 7          | 5        | 4
 * T6      | 4          | 6          | 5        | 4
 * T7      | 4          | 7          | 5        | 4
 * T8      | 2          | 5          | 5        | 4
 * T9      | 3          | 7          | 5        | 4
 * T10     | 4          | 7          | 4        | 5
 * T11     | 3          | 6          | 5        | 4
 * T12     | 4          | 7          | 5        | 4
 * T13     | 4          | 7          | 5        | 5
 * T14     | 5          | 8          | 4        | 5
 * T15     | 3          | 7          | 5        | 4
 */

/*
 * Scenario 3: Deadline-focused
 *   Available Time: 24h | Total Required: 41h | Time Pressure Ratio: 1.71
 *   Deadline Tightness: 0.73 | Importance Variation: 0.12 | Avg Difficulty: 3.2
 *
 * Task ID | Study Time | Importance | Deadline | Difficulty
 * --------+------------+------------+----------+------------
 * T1      | 3          | 7          | 2        | 3
 * T2      | 3          | 8          | 2        | 4
 * T3      | 2          | 7          | 1        | 3
 * T4      | 3          | 8          | 3        | 4
 * T5      | 3          | 7          | 1        | 3
 * T6      | 2          | 7          | 2        | 3
 * T7      | 3          | 7          | 3        | 3
 * T8      | 2          | 5          | 2        | 2
 * T9      | 2          | 6          | 4        | 3
 * T10     | 3          | 7          | 3        | 3
 * T11     | 2          | 6          | 5        | 3
 * T12     | 3          | 7          | 4        | 3
 * T13     | 3          | 8          | 2        | 4
 * T14     | 4          | 8          | 3        | 4
 * T15     | 3          | 7          | 5        | 3
 */

/*
 * Scenario 4: Importance-focused
 *   Available Time: 26h | Total Required: 41h | Time Pressure Ratio: 1.58
 *   Deadline Tightness: 0.00 | Importance Variation: 0.40 | Avg Difficulty: 3.27
 *
 * Task ID | Study Time | Importance | Deadline | Difficulty
 * --------+------------+------------+----------+------------
 * T1      | 3          | 4          | 6        | 3
 * T2      | 2          | 5          | 5        | 3
 * T3      | 3          | 10         | 5        | 4
 * T4      | 3          | 6          | 7        | 3
 * T5      | 2          | 4          | 6        | 3
 * T6      | 2          | 5          | 8        | 3
 * T7      | 3          | 3          | 7        | 3
 * T8      | 2          | 4          | 8        | 2
 * T9      | 2          | 5          | 9        | 3
 * T10     | 3          | 6          | 6        | 3
 * T11     | 2          | 4          | 7        | 3
 * T12     | 3          | 5          | 9        | 3
 * T13     | 3          | 10         | 5        | 4
 * T14     | 5          | 10         | 6        | 5
 * T15     | 3          | 9          | 7        | 4
 */

// ============================================================================
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
    vector<int> selectedTaskIds;
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
// Module 5: AI/ML Module - k-Nearest Neighbours (k-NN)
// ============================================================================

/*
 * TODO: Implement a k-NN classifier to predict the best planning
 *       strategy based on scenario features:
 *         - Number of tasks
 *         - Average importance
 *         - Average deadline
 *         - Average difficulty
 *         - Time pressure ratio (required time / available time)
 *
 * Training data should be generated or provided based on simulated
 * scenario outcomes.
 */

// ============================================================================
// Module 6: Performance Measurement & Strategy Comparison
// ============================================================================

/*
 * TODO: Measure and compare the results of all algorithms:
 *         - Execution time (ms)
 *         - Tasks selected
 *         - Total study time used
 *         - Total importance achieved
 *         - Prediction correctness (k-NN vs actual best)
 *
 * Display results in a structured comparison table.
 */

// ============================================================================
// Main Function
// ============================================================================

int main() {
    cout << "\n=====================================================================================" << endl;
    cout << "  StudySmart AI - Smart Academic Planning Application" << endl;
    cout << "  CST207 Design and Analysis of Algorithms" << endl;
    cout << "  Group: [Your Group Name]" << endl;
    cout << "=====================================================================================\n" << endl;

    // TODO: Call scenario generation / input module
    // TODO: Call Divide-and-Conquer / Sorting module
    // TODO: Call Greedy algorithm module
    // TODO: Call Dynamic Programming module
    // TODO: Call AI/ML recommendation module
    // TODO: Call performance measurement and comparison module

    cout << "\n=====================================================================================" << endl;
    cout << "  StudySmart AI - Execution Complete" << endl;
    cout << "=====================================================================================\n" << endl;

    return 0;
}
