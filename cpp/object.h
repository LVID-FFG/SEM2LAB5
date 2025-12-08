#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <iostream>
#include <algorithm>
#include <utility>
#include <iomanip>

using namespace std;

// КЛАСС ПРЕДМЕТА
class Subject {
private:
    string name;                           // Название предмета
    string code;                           // Код предмета
    int professorId;                       // ID преподавателя, ведущего предмет
    set<int> enrolledStudentIds;           // ID зачисленных студентов
    map<int, map<string, double>> assignmentGrades; // Оценки за задания
    map<int, map<string, double>> reportGrades;     // Оценки за доклады
    vector<string> assignments;            // Список названий заданий
    vector<string> reports;                // Список тем докладов
    
public:
    // КОНСТРУКТОР
    Subject(const string& name, const string& code, int professorId);
    
    void enrollStudent(int studentId);                     // Зачислить студента
    bool isStudentEnrolled(int studentId) const;          // Проверить зачисление
    
    void addAssignment(const string& assignmentName);     // Добавить задание
    void addReport(const string& reportName);            // Добавить доклад
    bool removeReport(const string& reportName);         // Удалить доклад
    
    // ВЫСТАВЛЕНИЕ ОЦЕНОК
    void gradeAssignment(int studentId, const string& assignmentName, double grade);  // Оценка за задание
    void gradeReport(int studentId, const string& reportName, double grade);          // Оценка за доклад
    void gradeAllReports(const string& reportName, double grade, const vector<int>& participants = {});  // Оценка всем за доклад
    // ПОЛУЧЕНИЕ ОЦЕНОК
    double getStudentAssignmentGrade(int studentId, const string& assignmentName) const;  // Оценка студента за задание
    double getStudentReportGrade(int studentId, const string& reportName) const;          // Оценка студента за доклад
    
    vector<int> getEnrolledStudents() const;              // Список ID зачисленных студентов
    vector<string> getAssignments() const;                // Список заданий
    vector<string> getReports() const;                    // Список докладов
    bool isProfessor(int professorId) const { return this->professorId == professorId; }  // Проверка преподавателя
    
    string getName() const { return name; }
    string getCode() const { return code; }
    int getProfessorId() const { return professorId; }
    
    void generateFinalReport(const map<int, string>& studentNames) const;  // Подробный отчет по предмету
    
    bool hasAssignment(const string& assignmentName) const;  // Проверить наличие задания
    bool hasReport(const string& reportName) const;          // Проверить наличие доклада
    
    vector<pair<string, double>> getStudentGradesSummary(int studentId) const; // получение сводци оценок студента
    
    const map<int, map<string, double>>& getAllAssignmentGrades() const { return assignmentGrades; }
    const map<int, map<string, double>>& getAllReportGrades() const { return reportGrades; }
    void setAssignmentGrades(const map<int, map<string, double>>& grades) { assignmentGrades = grades; }
    void setReportGrades(const map<int, map<string, double>>& grades) { reportGrades = grades; }
    vector<int> getEnrolledStudentIds() const { return vector<int>(enrolledStudentIds.begin(), enrolledStudentIds.end()); }
    vector<string> getAssignmentList() const { return assignments; }
    vector<string> getReportList() const { return reports; }
};

// КЛАСС ЗАДАНИЯ
class Assignment {
private:
    string name;         // Название задания
    string subjectName;  // Название предмета
    double maxScore;     // Максимальный балл за задание
    
public:
    Assignment(const string& name, const string& subjectName, double maxScore = 100.0)
        : name(name), subjectName(subjectName), maxScore(maxScore) {}
    
    string getName() const { return name; }
    double getMaxScore() const { return maxScore; }
    string getSubjectName() const { return subjectName; }
    void setMaxScore(double score) { maxScore = score; }
    void setSubjectName(const string& name) { subjectName = name; }
};

// КЛАСС ДОКЛАДА/ПРОЕКТА
class Report {
private:
    string topic;                     // Тема доклада
    string subjectName;               // Название предмета
    set<int> signedUpStudentIds;      // Множество ID записавшихся студентов
    time_t date;                      // Дата создания
    int maxParticipants;              // Максимальное количество участников
    bool isCompleted;                 // Флаг завершения (после выставления оценок)
    
public:
    Report(const string& topic, const string& subjectName, int maxParticipants = 5);
    
    bool addStudent(int studentId);    // Добавить студента
    bool removeStudent(int studentId); // Удалить студента
    bool isFull() const;               // Проверить заполненность
    bool hasStudent(int studentId) const;  // Проверить наличие студента
    
    string getTopic() const { return topic; }
    time_t getDate() const { return date; }
    bool getIsCompleted() const { return isCompleted; }
    int getSignedUpCount() const { return signedUpStudentIds.size(); }
    int getMaxParticipants() const { return maxParticipants; }
    string getSubjectName() const { return subjectName; }
    
    vector<int> getSignedUpStudents() const; // получение списка участников
    
    void markAsCompleted() { isCompleted = true; }  // Отметить как завершенный
};