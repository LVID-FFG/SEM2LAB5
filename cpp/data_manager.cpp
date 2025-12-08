#include "data_manager.h"
#include "user.h"
#include "student.h"
#include "professor.h"
#include "object.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <filesystem>
#include <algorithm>

using namespace std;

const string DataManager::DATA_DIR = "data";

void DataManager::initDataDirectory() {
    filesystem::create_directory(DATA_DIR);
}

void DataManager::saveNextUserId(int nextId) {
    ofstream file(DATA_DIR + "/next_id.txt");
    if (file.is_open()) {
        file << nextId;
        file.close();
    }
}

int DataManager::loadNextUserId() {
    ifstream file(DATA_DIR + "/next_id.txt");
    if (file.is_open()) {
        int nextId = 1;
        file >> nextId;
        file.close();
        return nextId;
    }
    return 1;
}

void DataManager::saveUsers(const map<string, shared_ptr<User>>& users) {
    ofstream file(DATA_DIR + "/users.txt");
    if (!file.is_open()) {
        return;
    }
    
    for (const auto& [name, user] : users) {
        file << user->getId() << "," << name << "," 
             << user->getPasswordHash() << "," 
             << static_cast<int>(user->getRole()) << "\n";
    }
    file.close();
}

void DataManager::saveSubjects(const vector<shared_ptr<Subject>>& subjects) {
    ofstream file(DATA_DIR + "/subjects.txt");
    if (!file.is_open()) {
        return;
    }
    
    for (const auto& subject : subjects) {
        file << subject->getName() << "," << subject->getCode() << "," 
             << subject->getProfessorId() << "\n";
    }
    file.close();
}

void DataManager::saveSubjectGrades(const vector<shared_ptr<Subject>>& subjects) {
    ofstream file(DATA_DIR + "/subject_grades.txt");
    if (!file.is_open()) {
        return;
    }
    
    for (const auto& subject : subjects) {
        string subjectName = subject->getName();
        if (subjectName.empty()) continue;
        
        // Сохраняем оценки за задания
        auto assignmentGrades = subject->getAllAssignmentGrades();
        for (const auto& [studentId, grades] : assignmentGrades) {
            for (const auto& [assignmentName, grade] : grades) {
                file << "ASSIGNMENT," << subjectName << "," 
                     << studentId << "," << assignmentName << "," << grade << "\n";
            }
        }
        
        // Сохраняем оценки за доклады
        auto reportGrades = subject->getAllReportGrades();
        for (const auto& [studentId, grades] : reportGrades) {
            for (const auto& [reportName, grade] : grades) {
                file << "REPORT," << subjectName << "," 
                     << studentId << "," << reportName << "," << grade << "\n";
            }
        }
    }
    file.close();
}

void DataManager::saveAssignments(const vector<shared_ptr<Assignment>>& assignments) {
    ofstream file(DATA_DIR + "/assignments.txt");
    if (!file.is_open()) {
        return;
    }
    
    for (const auto& assignment : assignments) {
        file << assignment->getName() << "," 
             << "" << "," 
             << assignment->getMaxScore() << "," << assignment->getSubjectName() << "\n";
    }
    file.close();
}

void DataManager::saveReports(const vector<shared_ptr<Report>>& reports) {
    ofstream file(DATA_DIR + "/reports.txt");
    if (!file.is_open()) {
        return;
    }
    
    for (const auto& report : reports) {
        file << report->getTopic() << "," << report->getSubjectName() << ","
             << report->getMaxParticipants() << "," 
             << (report->getIsCompleted() ? "1" : "0");
        
        auto students = report->getSignedUpStudents();
        for (int studentId : students) {
            file << "," << studentId;
        }
        file << "\n";
    }
    file.close();
}

void DataManager::saveEnrollments(const map<int, vector<string>>& studentEnrollments) {
    ofstream file(DATA_DIR + "/enrollments.txt");
    if (!file.is_open()) {
        return;
    }
    
    for (const auto& [studentId, subjects] : studentEnrollments) {
        file << studentId;
        for (const auto& subject : subjects) {
            file << "," << subject;
        }
        file << "\n";
    }
    file.close();
}

void DataManager::saveSubmissions(const vector<DataSubmission>& submissions) {
    ofstream file(DATA_DIR + "/submissions.txt");
    if (!file.is_open()) {
        return;
    }
    
    for (const auto& submission : submissions) {
        file << submission.studentId << "," << submission.subjectName << ","
             << submission.assignmentName << "," << submission.status << ","
             << submission.timestamp << "\n";
    }
    file.close();
}

void DataManager::saveGrades(const vector<DataGrade>& grades) {
    ofstream file(DATA_DIR + "/grades.txt");
    if (!file.is_open()) {
        return;
    }
    
    for (const auto& grade : grades) {
        file << grade.studentId << "," << grade.subjectName << ","
             << grade.assignmentName << "," << grade.score << ","
             << grade.type << "," << grade.timestamp << "\n";
    }
    file.close();
}

void DataManager::saveAllData(const map<string, shared_ptr<User>>& users,
                             const vector<shared_ptr<Subject>>& subjects,
                             const vector<shared_ptr<Assignment>>& assignments,
                             const vector<shared_ptr<Report>>& reports,
                             const map<int, vector<string>>& studentEnrollments,
                             const vector<DataSubmission>& submissions,
                             const vector<DataGrade>& grades) {
    initDataDirectory();
    
    saveUsers(users);
    saveSubjects(subjects);
    saveAssignments(assignments);
    saveReports(reports);
    saveEnrollments(studentEnrollments);
    saveSubmissions(submissions);
    saveGrades(grades);
    saveSubjectGrades(subjects);
    saveNextUserId(User::getNextId());
}

map<string, shared_ptr<User>> DataManager::loadUsers() {
    map<string, shared_ptr<User>> users;
    ifstream file(DATA_DIR + "/users.txt");
    if (!file.is_open()) {
        return users;
    }
    
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string idStr, name, passwordHash, roleStr;
        
        if (getline(ss, idStr, ',') &&
            getline(ss, name, ',') &&
            getline(ss, passwordHash, ',') &&
            getline(ss, roleStr, ',')) {
            
            int id = stoi(idStr);
            User::Role role = static_cast<User::Role>(stoi(roleStr));
            
            shared_ptr<User> user;
            switch (role) {
                case User::Role::STUDENT:
                    user = make_shared<Student>(name, passwordHash, id);
                    break;
                case User::Role::PROFESSOR:
                    user = make_shared<Professor>(name, passwordHash, id);
                    break;
            }
            
            if (user) {
                users[name] = user;
            }
        }
    }
    file.close();
    return users;
}

vector<shared_ptr<Subject>> DataManager::loadSubjects() {
    vector<shared_ptr<Subject>> subjects;
    ifstream file(DATA_DIR + "/subjects.txt");
    if (!file.is_open()) {
        return subjects;
    }
    
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string name, code, profIdStr;
        
        if (getline(ss, name, ',') &&
            getline(ss, code, ',') &&
            getline(ss, profIdStr, ',')) {
            
            int profId = stoi(profIdStr);
            subjects.push_back(make_shared<Subject>(name, code, profId));
        }
    }
    file.close();
    return subjects;
}

map<string, DataSubjectGrades> DataManager::loadSubjectGrades() {
    map<string, DataSubjectGrades> subjectGradesMap;
    ifstream file(DATA_DIR + "/subject_grades.txt");
    if (!file.is_open()) {
        return subjectGradesMap;
    }
    
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string type, subjectName, studentIdStr, itemName, gradeStr;
        
        if (getline(ss, type, ',') &&
            getline(ss, subjectName, ',') &&
            getline(ss, studentIdStr, ',') &&
            getline(ss, itemName, ',') &&
            getline(ss, gradeStr, ',')) {
            
            int studentId = stoi(studentIdStr);
            double grade = stod(gradeStr);
            
            if (subjectGradesMap.find(subjectName) == subjectGradesMap.end()) {
                subjectGradesMap[subjectName] = DataSubjectGrades();
                subjectGradesMap[subjectName].subjectName = subjectName;
            }
            
            if (type == "ASSIGNMENT") {
                subjectGradesMap[subjectName].assignmentGrades[studentId][itemName] = grade;
            } else if (type == "REPORT") {
                subjectGradesMap[subjectName].reportGrades[studentId][itemName] = grade;
            }
        }
    }
    file.close();
    return subjectGradesMap;
}

vector<shared_ptr<Assignment>> DataManager::loadAssignments() {
    vector<shared_ptr<Assignment>> assignments;
    ifstream file(DATA_DIR + "/assignments.txt");
    if (!file.is_open()) {
        return assignments;
    }
    
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string name, description, maxScoreStr, subjectName;
        
        if (getline(ss, name, ',') &&
            getline(ss, description, ',') &&
            getline(ss, maxScoreStr, ',') &&
            getline(ss, subjectName, ',')) {
            
            double maxScore = stod(maxScoreStr);
            assignments.push_back(make_shared<Assignment>(name, subjectName, maxScore));
        }
    }
    file.close();
    return assignments;
}

vector<shared_ptr<Report>> DataManager::loadReports() {
    vector<shared_ptr<Report>> reports;
    ifstream file(DATA_DIR + "/reports.txt");
    if (!file.is_open()) {
        return reports;
    }
    
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string topic, subjectName, maxPartStr, completedStr;
        
        if (getline(ss, topic, ',') &&
            getline(ss, subjectName, ',') &&
            getline(ss, maxPartStr, ',') &&
            getline(ss, completedStr, ',')) {
            
            int maxParticipants = stoi(maxPartStr);
            auto report = make_shared<Report>(topic, subjectName, maxParticipants);
            
            string studentIdStr;
            while (getline(ss, studentIdStr, ',')) {
                if (!studentIdStr.empty()) {
                    int studentId = stoi(studentIdStr);
                    report->addStudent(studentId);
                }
            }
            
            if (completedStr == "1") {
                report->markAsCompleted();
            }
            
            reports.push_back(report);
        }
    }
    file.close();
    return reports;
}

map<int, vector<string>> DataManager::loadEnrollments() {
    map<int, vector<string>> enrollments;
    ifstream file(DATA_DIR + "/enrollments.txt");
    if (!file.is_open()) {
        return enrollments;
    }
    
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string studentIdStr;
        
        if (getline(ss, studentIdStr, ',')) {
            int studentId = stoi(studentIdStr);
            vector<string> subjects;
            string subjectName;
            
            while (getline(ss, subjectName, ',')) {
                subjects.push_back(subjectName);
            }
            
            enrollments[studentId] = subjects;
        }
    }
    file.close();
    return enrollments;
}

vector<DataSubmission> DataManager::loadSubmissions() {
    vector<DataSubmission> submissions;
    ifstream file(DATA_DIR + "/submissions.txt");
    if (!file.is_open()) {
        return submissions;
    }
    
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        DataSubmission submission;
        string studentIdStr;
        
        if (getline(ss, studentIdStr, ',') &&
            getline(ss, submission.subjectName, ',') &&
            getline(ss, submission.assignmentName, ',') &&
            getline(ss, submission.status, ',') &&
            getline(ss, submission.timestamp, ',')) {
            
            submission.studentId = stoi(studentIdStr);
            submissions.push_back(submission);
        }
    }
    file.close();
    return submissions;
}

vector<DataGrade> DataManager::loadGrades() {
    vector<DataGrade> grades;
    ifstream file(DATA_DIR + "/grades.txt");
    if (!file.is_open()) {
        return grades;
    }
    
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        DataGrade grade;
        string studentIdStr, scoreStr;
        
        if (getline(ss, studentIdStr, ',') &&
            getline(ss, grade.subjectName, ',') &&
            getline(ss, grade.assignmentName, ',') &&
            getline(ss, scoreStr, ',') &&
            getline(ss, grade.type, ',') &&
            getline(ss, grade.timestamp, ',')) {
            
            grade.studentId = stoi(studentIdStr);
            grade.score = stod(scoreStr);
            grades.push_back(grade);
        }
    }
    file.close();
    return grades;
}

string DataManager::getCurrentTimestamp() {
    time_t now = time(nullptr);
    tm* tm = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
    return string(buffer);
}