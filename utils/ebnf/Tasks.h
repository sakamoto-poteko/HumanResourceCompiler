#ifndef TASKS_H
#define TASKS_H

#include <string>

class DependencyGraphAnalyzer;

void calculate_first_follow_set(DependencyGraphAnalyzer &checker, bool check_conflicts, const std::string &output_path);
void check_unreachable(DependencyGraphAnalyzer &checker);
void check_non_left_circular(DependencyGraphAnalyzer &checker);
void check_left_recursion(DependencyGraphAnalyzer &checker);

#endif