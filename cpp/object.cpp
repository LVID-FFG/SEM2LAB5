#include "object.h"
#include "student.h"

using namespace std;

Subject::Subject(const string& name, const string& code, int professorId)
    : name(name), code(code), professorId(professorId) {}

void Subject::enrollStudent(int studentId) {
    enrolledStudentIds.insert(studentId);
}

bool Subject::isStudentEnrolled(int studentId) const {
    return enrolledStudentIds.find(studentId) != enrolledStudentIds.end();
}

void Subject::addAssignment(const string& assignmentName) {
    assignments.push_back(assignmentName);
}

void Subject::addReport(const string& reportName) {
    reports.push_back(reportName);
}

bool Subject::removeReport(const string& reportName) {
    auto it = find(reports.begin(), reports.end(), reportName);
    if (it != reports.end()) {
        reports.erase(it);
        return true;
    }
    return false;
}

void Subject::gradeAssignment(int studentId, const string& assignmentName, double grade) {
    if (isStudentEnrolled(studentId) && hasAssignment(assignmentName)) {
        assignmentGrades[studentId][assignmentName] = grade;
    }
}

void Subject::gradeReport(int studentId, const string& reportName, double grade) {
    if (isStudentEnrolled(studentId)) {
        reportGrades[studentId][reportName] = grade;
    }
}

void Subject::gradeAllReports(const string& reportName, double grade, const vector<int>& participants) {
    if (!hasReport(reportName)) return;
    
    if (participants.empty()) {
        for (int studentId : enrolledStudentIds) {
            reportGrades[studentId][reportName] = grade;
        }
    } else {
        for (int studentId : participants) {
            if (enrolledStudentIds.find(studentId) != enrolledStudentIds.end()) {
                reportGrades[studentId][reportName] = grade;
            }
        }
    }
}

double Subject::getStudentAssignmentGrade(int studentId, const string& assignmentName) const {
    auto itStudent = assignmentGrades.find(studentId);
    if (itStudent != assignmentGrades.end()) {
        auto itAssignment = itStudent->second.find(assignmentName);
        if (itAssignment != itStudent->second.end()) {
            return itAssignment->second;
        }
    }
    return -1.0;
}

double Subject::getStudentReportGrade(int studentId, const string& reportName) const {
    auto itStudent = reportGrades.find(studentId);
    if (itStudent != reportGrades.end()) {
        auto itReport = itStudent->second.find(reportName);
        if (itReport != itStudent->second.end()) {
            return itReport->second;
        }
    }
    return -1.0;
}

vector<int> Subject::getEnrolledStudents() const {
    return vector<int>(enrolledStudentIds.begin(), enrolledStudentIds.end());
}

vector<string> Subject::getAssignments() const {
    return assignments;
}

vector<string> Subject::getReports() const {
    return reports;
}

void Subject::generateFinalReport(const map<int, string>& studentNames) const {
    cout << "\n=== ИТОГОВЫЙ ОТЧЕТ: " << name << " (" << code << ") ===\n";
    cout << "ID преподавателя: " << professorId << "\n";
    cout << "Зачисленных студентов: " << enrolledStudentIds.size() << "\n";
    cout << "==============================================\n";
    
    for (int studentId : enrolledStudentIds) {
        auto itName = studentNames.find(studentId);
        string studentName = (itName != studentNames.end()) ? itName->second : "Неизвестный";
        
        cout << "\nСтудент: " << studentName << " (ID: " << studentId << ")\n";
        
        double total = 0.0;
        int count = 0;
        
        auto itAssignments = assignmentGrades.find(studentId);
        if (itAssignments != assignmentGrades.end()) {
            cout << "Оценки за задания:\n";
            for (const auto& [assignment, grade] : itAssignments->second) {
                cout << "  " << left << setw(20) << assignment 
                          << ": " << right << setw(6) << fixed 
                          << setprecision(2) << grade << "\n";
                total += grade;
                count++;
            }
        }
        
        auto itReports = reportGrades.find(studentId);
        if (itReports != reportGrades.end()) {
            cout << "Оценки за доклады:\n";
            for (const auto& [report, grade] : itReports->second) {
                cout << "  " << left << setw(20) << report 
                          << ": " << right << setw(6) << fixed 
                          << setprecision(2) << grade << "\n";
                total += grade;
                count++;
            }
        }
        
        if (count > 0) {
            cout << "  Среднее: " << fixed << setprecision(2) << (total / count) << "\n";
            cout << "  Суммарное: " << fixed << setprecision(2) << total << "\n";
        } else {
            cout << "  Нет оценок\n";
        }
    }
    cout << "\n";
}

bool Subject::hasAssignment(const string& assignmentName) const {
    return find(assignments.begin(), assignments.end(), assignmentName) != assignments.end();
}

bool Subject::hasReport(const string& reportName) const {
    return find(reports.begin(), reports.end(), reportName) != reports.end();
}

vector<pair<string, double>> Subject::getStudentGradesSummary(int studentId) const {
    vector<pair<string, double>> result;
    
    auto itAssignments = assignmentGrades.find(studentId);
    if (itAssignments != assignmentGrades.end()) {
        for (const auto& [assignment, grade] : itAssignments->second) {
            result.emplace_back("Задание: " + assignment, grade);
        }
    }
    
    auto itReports = reportGrades.find(studentId);
    if (itReports != reportGrades.end()) {
        for (const auto& [report, grade] : itReports->second) {
            result.emplace_back("Доклад: " + report, grade);
        }
    }
    
    return result;
}

Report::Report(const string& topic, const string& subjectName, int maxParticipants)
    : topic(topic), subjectName(subjectName), maxParticipants(maxParticipants), 
      isCompleted(false) {
    date = time(nullptr);
}

bool Report::addStudent(int studentId) {
    if (isFull() || isCompleted) {
        return false;
    }
    
    if (signedUpStudentIds.find(studentId) == signedUpStudentIds.end()) {
        signedUpStudentIds.insert(studentId);
        return true;
    }
    
    return false;
}

bool Report::removeStudent(int studentId) {
    return signedUpStudentIds.erase(studentId) > 0;
}

bool Report::isFull() const {
    return signedUpStudentIds.size() >= maxParticipants;
}

bool Report::hasStudent(int studentId) const {
    return signedUpStudentIds.find(studentId) != signedUpStudentIds.end();
}

vector<int> Report::getSignedUpStudents() const {
    return vector<int>(signedUpStudentIds.begin(), signedUpStudentIds.end());
}