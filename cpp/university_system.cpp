#include "university_system.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <filesystem>
using namespace std;

UniversitySystem::UniversitySystem() {
    DataManager::initDataDirectory();
    loadAllData();
}

UniversitySystem::~UniversitySystem() {
    saveAllData();
}

void UniversitySystem::run() {
    while (true) {
        if (!currentUser) {
            showMainMenu();
        } else {
            switch (currentUser->getRole()) {
                case User::Role::STUDENT:
                    runStudentMenu(dynamic_pointer_cast<Student>(currentUser));
                    break;
                case User::Role::PROFESSOR:
                    runProfessorMenu(dynamic_pointer_cast<Professor>(currentUser));
                    break;
            }
        }
    }
}

// ==================== ПРИВАТНЫЕ МЕТОДЫ ====================

void UniversitySystem::loadAllData() {
    int nextId = DataManager::loadNextUserId();
    User::updateNextId(nextId);
    
    users = DataManager::loadUsers();
    subjects = DataManager::loadSubjects();
    
    auto subjectGradesMap = DataManager::loadSubjectGrades();
    
    auto loadedAssignments = DataManager::loadAssignments();
    assignments.clear();
    
    for (const auto& assignmentPtr : loadedAssignments) {
        string subjectName = assignmentPtr->getSubjectName();
        double maxScore = assignmentPtr->getMaxScore();
        
        if (subjectName.empty()) {
            continue;
        }
        
        bool subjectFound = false;
        for (const auto& sysSubject : subjects) {
            if (sysSubject->getName() == subjectName) {
                auto assignment = make_shared<Assignment>(
                    assignmentPtr->getName(),
                    subjectName,
                    maxScore
                );
                assignments.push_back(assignment);
                sysSubject->addAssignment(assignment->getName());
                subjectFound = true;
                break;
            }
        }
        
        if (!subjectFound) {
            assignments.push_back(assignmentPtr);
        }
    }
    
    auto loadedReports = DataManager::loadReports();
    reports.clear();
    
    for (const auto& reportPtr : loadedReports) {
        string subjectName = reportPtr->getSubjectName();
        bool subjectFound = false;
        for (const auto& sysSubject : subjects) {
            if (sysSubject->getName() == subjectName) {
                auto report = make_shared<Report>(
                    reportPtr->getTopic(),
                    subjectName,
                    reportPtr->getMaxParticipants()
                );
                
                auto studentIds = reportPtr->getSignedUpStudents();
                for (int studentId : studentIds) {
                    report->addStudent(studentId);
                }
                
                if (reportPtr->getIsCompleted()) {
                    report->markAsCompleted();
                }
                
                reports.push_back(report);
                sysSubject->addReport(reportPtr->getTopic());
                subjectFound = true;
                break;
            }
        }
    }
    
    studentEnrollments = DataManager::loadEnrollments();
    
    for (const auto& [studentId, subjectNames] : studentEnrollments) {
        for (const auto& subjectName : subjectNames) {
            auto subject = findSubject(subjectName);
            if (subject) {
                subject->enrollStudent(studentId);
                subjectEnrollments[subjectName].push_back(studentId);
            }
        }
    }
    
    // ЗАГРУЗКА ОЦЕНОК ИЗ subject_grades.txt
    for (const auto& [subjectName, subjectGrades] : subjectGradesMap) {
        auto subject = findSubject(subjectName);
        
        // Оценки за задания
        for (const auto& [studentId, grades] : subjectGrades.assignmentGrades) {
            for (const auto& [assignmentName, grade] : grades) {
                if (!subject->isStudentEnrolled(studentId)) {
                    subject->enrollStudent(studentId);
                    studentEnrollments[studentId].push_back(subjectName);
                }
                subject->gradeAssignment(studentId, assignmentName, grade);
            }
        }
        
        // Оценки за доклады
        for (const auto& [studentId, grades] : subjectGrades.reportGrades) {
            for (const auto& [reportName, grade] : grades) {
                if (!subject->isStudentEnrolled(studentId)) {
                    subject->enrollStudent(studentId);
                    studentEnrollments[studentId].push_back(subjectName);
                }
                subject->gradeReport(studentId, reportName, grade);
            }
        }
    }
    
    auto submissionsData = DataManager::loadSubmissions();
    submissions.clear();
    for (const auto& sub : submissionsData) {
        SubmissionRecord rec;
        rec.studentId = sub.studentId;
        rec.subjectName = sub.subjectName;
        rec.assignmentName = sub.assignmentName;
        rec.type = "assignment";
        for (const auto& report : reports) {
            if (report->getTopic() == sub.assignmentName) {
                rec.type = "report";
                break;
            }
        }
        rec.status = sub.status;
        rec.timestamp = sub.timestamp;
        submissions.push_back(rec);
    }
    
    auto gradesData = DataManager::loadGrades();
    grades.clear();
    for (const auto& g : gradesData) {
        GradeRecord rec;
        rec.studentId = g.studentId;
        rec.subjectName = g.subjectName;
        rec.itemName = g.assignmentName;
        rec.type = g.type;
        rec.score = g.score;
        rec.timestamp = g.timestamp;
        grades.push_back(rec);
    }
    
    for (const auto& [name, user] : users) {
        if (user->getRole() == User::Role::STUDENT) {
            auto student = dynamic_pointer_cast<Student>(user);
            if (student) {
                students[user->getId()] = student;
            }
        } else if (user->getRole() == User::Role::PROFESSOR) {
            auto professor = dynamic_pointer_cast<Professor>(user);
            if (professor) {
                professors[user->getId()] = professor;
            }
        }
    }
}

void UniversitySystem::saveAllData() {
    vector<DataSubmission> submissionsData;
    for (const auto& sub : submissions) {
        DataSubmission s;
        s.studentId = sub.studentId;
        s.subjectName = sub.subjectName;
        s.assignmentName = sub.assignmentName;
        s.status = sub.status;
        s.timestamp = sub.timestamp;
        submissionsData.push_back(s);
    }
    
    vector<DataGrade> gradesData;
    for (const auto& g : grades) {
        DataGrade grade;
        grade.studentId = g.studentId;
        grade.subjectName = g.subjectName;
        grade.assignmentName = g.itemName;
        grade.type = g.type;
        grade.score = g.score;
        grade.timestamp = g.timestamp;
        gradesData.push_back(grade);
    }
    
    DataManager::saveAllData(users, subjects, assignments, reports,
                            studentEnrollments, submissionsData, gradesData);
}

bool UniversitySystem::login(const string& name, const string& password) {
    auto it = users.find(name);
    if (it == users.end()) {
        cout << "Пользователь не найден!\n";
        return false;
    }
    
    if (it->second->checkPassword(password)) {
        currentUser = it->second;
        cout << "\n=== Вход успешен! ===\n";
        cout << "Добро пожаловать, " << name << " (" 
                  << it->second->getRoleString() << ")\n";
        return true;
    }
    
    cout << "Неверный пароль!\n";
    return false;
}

void UniversitySystem::logout() {
    if (currentUser) {
        cout << "Выход из системы пользователя: " << currentUser->getName() << endl;
        currentUser = nullptr;
    }
}

bool UniversitySystem::registerUser(const string& name, const string& password,
                                   User::Role role) {
    if (users.find(name) != users.end()) {
        cout << "Пользователь с таким именем уже существует!\n";
        return false;
    }
    
    shared_ptr<User> user;
    switch (role) {
        case User::Role::STUDENT:
            user = make_shared<Student>(name, password);
            students[user->getId()] = dynamic_pointer_cast<Student>(user);
            break;
        case User::Role::PROFESSOR:
            user = make_shared<Professor>(name, password);
            professors[user->getId()] = dynamic_pointer_cast<Professor>(user);
            break;
    }
    
    users[name] = user;
    cout << "Пользователь " << name << " успешно зарегистрирован!\n";
    saveAllData();
    return true;
}

void UniversitySystem::showMainMenu() {
    cout << "\n=== УНИВЕРСИТЕТСКАЯ СИСТЕМА ===\n";
    cout << "1. Вход\n";
    cout << "2. Регистрация\n";
    cout << "3. Выход\n";
    cout << "Выберите действие: ";
    
    int choice;
    cin >> choice;
    cin.ignore();
    
    switch (choice) {
        case 1: {
            string name, password;
            cout << "Имя: ";
            getline(cin, name);
            cout << "Пароль: ";
            getline(cin, password);
            login(name, password);
            break;
        }
        case 2: {
            string name, password;
            cout << "Имя: ";
            getline(cin, name);
            cout << "Пароль: ";
            getline(cin, password);
            
            cout << "Выберите роль:\n";
            cout << "1. Студент\n";
            cout << "2. Преподаватель\n";
            cout << "Выбор: ";
            int roleChoice;
            cin >> roleChoice;
            cin.ignore();
            
            User::Role role;
            switch (roleChoice) {
                case 1: role = User::Role::STUDENT; break;
                case 2: role = User::Role::PROFESSOR; break;
                default: 
                    cout << "Неверный выбор!\n";
                    return;
            }
            
            registerUser(name, password, role);
            break;
        }
        case 3:
            saveAllData();
            cout << "До свидания!\n";
            exit(0);
        default:
            cout << "Неверный выбор!\n";
    }
}

bool UniversitySystem::isStudentAlreadyEnrolled(int studentId, const string& subjectName) const {
    auto it = studentEnrollments.find(studentId);
    if (it != studentEnrollments.end()) {
        const auto& subjects = it->second;
        return find(subjects.begin(), subjects.end(), subjectName) != subjects.end();
    }
    return false;
}

shared_ptr<Subject> UniversitySystem::findSubjectByNameOrCode(const string& identifier) const {
    if (!identifier.empty() && identifier[0] == '$') {
        string code = identifier.substr(1);
        for (const auto& subject : subjects) {
            if (subject->getCode() == code) {
                return subject;
            }
        }
    }
    return findSubject(identifier);
}

void UniversitySystem::removeReport(const string& subjectName, const string& reportName) {
    reports.erase(remove_if(reports.begin(), reports.end(),
        [&](const shared_ptr<Report>& r) {
            return r->getTopic() == reportName && 
                   r->getSubjectName() == subjectName;
        }), reports.end());
    
    auto subject = findSubject(subjectName);
    if (subject) {
        subject->removeReport(reportName);
    }
}

shared_ptr<Report> UniversitySystem::findReportForSubject(const string& subjectName,
                                                             const string& reportName) const {
    for (const auto& report : reports) {
        if (report->getTopic() == reportName && 
            report->getSubjectName() == subjectName) {
            return report;
        }
    }
    return nullptr;
}

shared_ptr<Subject> UniversitySystem::findSubject(const string& name) const {
    for (const auto& subject : subjects) {
        if (subject->getName() == name) {
            return subject;
        }
    }
    return nullptr;
}

shared_ptr<Report> UniversitySystem::findReport(const string& topic) const {
    for (const auto& report : reports) {
        if (report->getTopic() == topic) {
            return report;
        }
    }
    return nullptr;
}

shared_ptr<Student> UniversitySystem::findStudentById(int id) const {
    auto it = students.find(id);
    if (it != students.end()) {
        return it->second;
    }
    return nullptr;
}

void UniversitySystem::addSubject(shared_ptr<Subject> subject) {
    subjects.push_back(subject);
    saveAllData();
}

void UniversitySystem::enrollStudentInSubject(int studentId, const string& identifier) {
    auto subject = findSubjectByNameOrCode(identifier);
    if (subject) {
        if (isStudentAlreadyEnrolled(studentId, subject->getName())) {
            cout << "Студент ID " << studentId << " уже зачислен на предмет " << subject->getName() << endl;
            return;
        }
        
        subject->enrollStudent(studentId);
        studentEnrollments[studentId].push_back(subject->getName());
        subjectEnrollments[subject->getName()].push_back(studentId);
        cout << "Студент ID " << studentId << " зачислен на предмет " << subject->getName() << endl;
        saveAllData();
    } else {
        cout << "Предмет не найден! Используйте название или код (например: $100)\n";
    }
}

vector<string> UniversitySystem::getStudentSubjects(int studentId) const {
    auto it = studentEnrollments.find(studentId);
    if (it != studentEnrollments.end()) {
        return it->second;
    }
    return {};
}

void UniversitySystem::addAssignment(shared_ptr<Assignment> assignment) {
    assignments.push_back(assignment);
    saveAllData();
}

void UniversitySystem::addReport(shared_ptr<Report> report) {
    reports.push_back(report);
    saveAllData();
}

bool UniversitySystem::submitReport(int studentId, const string& subjectName,
                                   const string& reportName) {
    auto subject = findSubject(subjectName);
    if (!subject || !subject->isStudentEnrolled(studentId) || 
        !subject->hasReport(reportName)) {
        return false;
    }
    
    SubmissionRecord submission;
    submission.studentId = studentId;
    submission.subjectName = subjectName;
    submission.assignmentName = reportName;
    submission.type = "report";
    submission.status = "pending";
    submission.timestamp = DataManager::getCurrentTimestamp();
    
    submissions.push_back(submission);
    saveAllData();
    return true;
}

bool UniversitySystem::gradeAssignment(int studentId, const string& subjectName,
                                      const string& assignmentName, double grade) {
    auto subject = findSubject(subjectName);
    if (!subject || !subject->isStudentEnrolled(studentId)) {
        cout << "Ошибка: студент не найден или не зачислен на предмет\n";
        return false;
    }
    
    if (!subject->hasAssignment(assignmentName)) {
        cout << "Ошибка: задание '" << assignmentName << "' не существует\n";
        return false;
    }
    
    double maxScore = 100.0;
    for (const auto& assignment : assignments) {
        if (assignment->getName() == assignmentName && 
            assignment->getSubjectName() == subjectName) {
            maxScore = assignment->getMaxScore();
            break;
        }
    }
    
    if (grade < 0 || grade > maxScore) {
        cout << "Ошибка: оценка должна быть от 0 до " << maxScore << endl;
        return false;
    }
    
    if (subject->getStudentAssignmentGrade(studentId, assignmentName) >= 0) {
        cout << "Ошибка: оценка за это задание уже выставлена\n";
        return false;
    }
    
    bool found = false;
    for (auto& sub : submissions) {
        if (sub.studentId == studentId && 
            sub.subjectName == subjectName && 
            sub.assignmentName == assignmentName &&
            sub.type == "assignment") {
            sub.status = "approved";
            found = true;
            break;
        }
    }
    
    if (!found) {
        SubmissionRecord submission;
        submission.studentId = studentId;
        submission.subjectName = subjectName;
        submission.assignmentName = assignmentName;
        submission.type = "assignment";
        submission.status = "approved";
        submission.timestamp = DataManager::getCurrentTimestamp();
        submissions.push_back(submission);
    }
    
    subject->gradeAssignment(studentId, assignmentName, grade);
    
    GradeRecord gradeRecord;
    gradeRecord.studentId = studentId;
    gradeRecord.subjectName = subjectName;
    gradeRecord.itemName = assignmentName;
    gradeRecord.type = "assignment";
    gradeRecord.score = grade;
    gradeRecord.timestamp = DataManager::getCurrentTimestamp();
    
    grades.push_back(gradeRecord);
    cout << "Оценка " << grade << " успешно выставлена за задание '" << assignmentName 
              << "' (макс. балл: " << maxScore << ")\n";
    
    auto student = findStudentById(studentId);
    if (student) {
        student->onGradeUpdated(subjectName, assignmentName, grade);
    }
    
    saveAllData();
    return true;
}

bool UniversitySystem::submitAssignment(int studentId, const string& subjectName, 
                                       const string& assignmentName) {
    auto subject = findSubject(subjectName);
    if (!subject || !subject->isStudentEnrolled(studentId)) {
        cout << "Ошибка: студент не зачислен на предмет или предмет не найден\n";
        return false;
    }
    
    if (!subject->hasAssignment(assignmentName)) {
        cout << "Ошибка: задание '" << assignmentName << "' не существует в предмете " << subjectName << endl;
        return false;
    }
    
    bool canResubmit = false;
    
    for (const auto& sub : submissions) {
        if (sub.studentId == studentId && 
            sub.subjectName == subjectName && 
            sub.assignmentName == assignmentName &&
            sub.type == "assignment") {
            
            if (sub.status == "pending") {
                cout << "Ошибка: вы уже отправили это задание и оно ожидает проверки\n";
                return false;
            } else if (sub.status == "rejected") {
                canResubmit = true;
            } else if (sub.status == "approved") {
                if (subject->getStudentAssignmentGrade(studentId, assignmentName) >= 0) {
                    cout << "Ошибка: за это задание уже выставлена оценка\n";
                    return false;
                }
            }
        }
    }
    
    if (subject->getStudentAssignmentGrade(studentId, assignmentName) >= 0) {
        cout << "Ошибка: за это задание уже выставлена оценка\n";
        return false;
    }
    
    if (canResubmit) {
        for (auto& sub : submissions) {
            if (sub.studentId == studentId && 
                sub.subjectName == subjectName && 
                sub.assignmentName == assignmentName &&
                sub.type == "assignment" &&
                sub.status == "rejected") {
                sub.status = "pending";
                sub.timestamp = DataManager::getCurrentTimestamp();
                cout << "Задание '" << assignmentName << "' успешно пересдано на проверку!\n";
                saveAllData();
                return true;
            }
        }
    }
    
    SubmissionRecord submission;
    submission.studentId = studentId;
    submission.subjectName = subjectName;
    submission.assignmentName = assignmentName;
    submission.type = "assignment";
    submission.status = "pending";
    submission.timestamp = DataManager::getCurrentTimestamp();
    
    submissions.push_back(submission);
    cout << "Задание '" << assignmentName << "' успешно сдано на проверку!\n";
    saveAllData();
    return true;
}

bool UniversitySystem::gradeReport(const string& identifier,
                                  const string& reportName, double grade) {
    auto subject = findSubjectByNameOrCode(identifier);
    if (!subject) {
        cout << "Ошибка: предмет не найден\n";
        return false;
    }
    
    string subjectName = subject->getName();
    
    if (!subject->hasReport(reportName)) {
        cout << "Ошибка: доклад '" << reportName << "' не существует\n";
        return false;
    }
    
    bool hasGrades = false;
    for (const auto& g : grades) {
        if (g.type == "report" && g.itemName == reportName && 
            g.subjectName == subjectName) {
            hasGrades = true;
            break;
        }
    }
    
    if (hasGrades) {
        cout << "Ошибка: оценки за этот доклад уже выставлены\n";
        return false;
    }
    
    if (grade < 0 || grade > 100) {
        cout << "Ошибка: оценка должна быть от 0 до 100\n";
        return false;
    }
    
    auto report = findReportForSubject(subjectName, reportName);
    if (!report) {
        cout << "Ошибка: информация о докладе не найдена\n";
        return false;
    }
    
    auto participants = report->getSignedUpStudents();
    
    if (participants.empty()) {
        cout << "Ошибка: на этот доклад не записан ни один студент\n";
        return false;
    }
    
    subject->gradeAllReports(reportName, grade, participants);
    
    int count = 0;
    for (int studentId : participants) {
        if (subject->isStudentEnrolled(studentId)) {
            GradeRecord gradeRecord;
            gradeRecord.studentId = studentId;
            gradeRecord.subjectName = subjectName;
            gradeRecord.itemName = reportName;
            gradeRecord.type = "report";
            gradeRecord.score = grade;
            gradeRecord.timestamp = DataManager::getCurrentTimestamp();
            
            grades.push_back(gradeRecord);
            count++;
            
            auto student = findStudentById(studentId);
            if (student) {
                student->onGradeUpdated(subjectName, reportName, grade);
            }
        }
    }
    
    cout << "Оценка " << fixed << setprecision(2) << grade << " выставлена " 
         << count << " студентам за доклад '" << reportName << "'\n";
    
    removeReport(subjectName, reportName);
    cout << "Доклад '" << reportName << "' удален\n";
    
    saveAllData();
    
    return true;
}

vector<SubmissionRecord> UniversitySystem::getPendingSubmissions(const string& subjectName) const {
    vector<SubmissionRecord> result;
    for (const auto& sub : submissions) {
        if (sub.status == "pending") {
            if (subjectName.empty() || sub.subjectName == subjectName) {
                result.push_back(sub);
            }
        }
    }
    return result;
}

void UniversitySystem::listAllSubjects() const {
    cout << "\nВсе предметы (" << subjects.size() << "):\n";
    for (const auto& subject : subjects) {
        cout << "- " << subject->getName() 
                  << " (" << subject->getCode() 
                  << "), Преподаватель ID: " << subject->getProfessorId() << endl;
    }
}

void UniversitySystem::listAllReports() const {
    cout << "\nВсе доклады (" << reports.size() << "):\n";
    for (const auto& report : reports) {
        cout << "- " << report->getTopic() 
                  << " (Предмет: " << report->getSubjectName()
                  << ", Участников: " << report->getSignedUpCount()
                  << "/" << report->getMaxParticipants() << ")\n";
    }
}

void UniversitySystem::listAllStudents() const {
    cout << "\nВсе студенты (" << students.size() << "):\n";
    for (const auto& [id, student] : students) {
        cout << "- " << student->getName() 
                  << " (ID: " << id << ")\n";
    }
}

void UniversitySystem::listAllProfessors() const {
    cout << "\nВсе преподаватели (" << professors.size() << "):\n";
    for (const auto& [id, professor] : professors) {
        cout << "- " << professor->getName() 
                  << " (ID: " << id << ")\n";
    }
}

void UniversitySystem::showSubjectStatistics(const string& subjectName) const {
    auto subject = findSubject(subjectName);
    if (!subject) {
        cout << "Предмет не найден!\n";
        return;
    }
    
    cout << "\n=== Статистика по предмету " << subjectName << " ===\n";
    
    auto enrolled = subject->getEnrolledStudents();
    cout << "Всего студентов: " << enrolled.size() << endl;
    
    int submitted = 0;
    for (const auto& sub : submissions) {
        if (sub.subjectName == subjectName && sub.type == "assignment") {
            submitted++;
        }
    }
    
    cout << "Заданий сдано: " << submitted << endl;
    cout << "Заданий на проверке: " 
              << getPendingSubmissions(subjectName).size() << endl;
    
    double total = 0;
    int count = 0;
    for (const auto& grade : grades) {
        if (grade.subjectName == subjectName) {
            total += grade.score;
            count++;
        }
    }
    
    if (count > 0) {
        cout << "Средняя оценка: " << (total / count) << endl;
        cout << "Суммарная оценка: " << total << endl;
    }
}

void UniversitySystem::showStudentSubjectSummary(int studentId) const {
    auto student = findStudentById(studentId);
    if (!student) {
        cout << "Студент не найден!\n";
        return;
    }
    
    cout << "\n=== ИТОГИ ПО ПРЕДМЕТАМ ДЛЯ " << student->getName() << " ===\n";
    
    auto studentSubjects = getStudentSubjects(studentId);
    if (studentSubjects.empty()) {
        cout << "Студент не зачислен ни на один предмет.\n";
        return;
    }
    
    set<string> uniqueSubjects;
    for (const auto& subject : studentSubjects) {
        uniqueSubjects.insert(subject);
    }
    
    double overallTotal = 0;
    int overallCount = 0;
    
    for (const auto& subjectName : uniqueSubjects) {
        auto subject = findSubject(subjectName);
        if (!subject) continue;
        
        cout << "\nПредмет: " << subjectName << " (код: " << subject->getCode() << ")\n";
        
        auto gradesSummary = subject->getStudentGradesSummary(studentId);
        
        if (gradesSummary.empty()) {
            cout << "  Нет оценок\n";
            continue;
        }
        
        double subjectTotal = 0;
        int subjectCount = 0;
        
        cout << "  Оценки:\n";
        for (const auto& [item, grade] : gradesSummary) {
            cout << "  - " << item << ": " << grade << endl;
            subjectTotal += grade;
            subjectCount++;
        }
        
        double subjectAverage = subjectTotal / subjectCount;
        cout << "  Средний балл: " << fixed << setprecision(2) << subjectAverage << endl;
        cout << "  Суммарный балл: " << fixed << setprecision(2) << subjectTotal << endl;
        
        overallTotal += subjectTotal;
        overallCount += subjectCount;
    }
    
    if (overallCount > 0) {
        cout << "\n=== ОБЩИЕ ИТОГИ ===\n";
        cout << "Всего предметов: " << uniqueSubjects.size() << endl;
        cout << "Всего оценок: " << overallCount << endl;
        cout << "Cредний балл: " << fixed << setprecision(2) 
                  << (overallTotal / uniqueSubjects.size()) << endl;
    }
}

void UniversitySystem::runStudentMenu(shared_ptr<Student> student) {
    while (true) {
        cout << "\n=== МЕНЮ СТУДЕНТА ===\n";
        cout << "1. Просмотреть мои предметы\n";
        cout << "2. Сдать задание\n";
        cout << "3. Записаться на доклад\n";
        cout << "4. Отказаться от доклада\n";
        cout << "5. Посмотреть мои оценки по предметам\n";
        cout << "6. Выход из системы\n";
        cout << "Выберите действие: ";
        
        int choice;
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case 1: {
                auto studentSubjects = getStudentSubjects(student->getId());
                if (studentSubjects.empty()) {
                    cout << "Вы не зачислены ни на один предмет.\n";
                } else {
                    cout << "Ваши предметы (" << studentSubjects.size() << "):\n";
                    set<string> uniqueSubjects;
                    for (const auto& subject : studentSubjects) {
                        uniqueSubjects.insert(subject);
                    }
                    for (const auto& subject : uniqueSubjects) {
                        auto subj = findSubject(subject);
                        if (subj) {
                            cout << "- " << subject << " (код: " << subj->getCode() << ")\n";
                        }
                    }
                }
                break;
            }
            case 2: {
                auto studentSubjects = getStudentSubjects(student->getId());
                if (studentSubjects.empty()) {
                    cout << "Вы не зачислены ни на один предмет.\n";
                    break;
                }
                
                cout << "Ваши предметы:\n";
                for (const auto& subjectName : studentSubjects) {
                    auto subj = findSubject(subjectName);
                    if (subj) {
                        cout << "- " << subjectName << " (код: " << subj->getCode() << ")\n";
                    }
                }
                
                cout << "Введите название предмета или код (например: $100): ";
                string identifier;
                getline(cin, identifier);
                
                auto subject = findSubjectByNameOrCode(identifier);
                if (subject) {
                    auto assignments = subject->getAssignments();
                    if (assignments.empty()) {
                        cout << "В этом предмете нет заданий.\n";
                    } else {
                        cout << "Доступные задания:\n";
                        for (const auto& a : assignments) {
                            cout << "- " << a << endl;
                        }
                        
                        cout << "Введите название задания: ";
                        string assignmentName;
                        getline(cin, assignmentName);
                        
                        submitAssignment(student->getId(), subject->getName(), assignmentName);
                    }
                } else {
                    cout << "Предмет не найден! Используйте название или код.\n";
                }
                break;
            }
            case 3: {
                auto studentSubjects = getStudentSubjects(student->getId());
                if (studentSubjects.empty()) {
                    cout << "Вы не зачислены ни на один предмет.\n";
                    break;
                }
                
                cout << "Список доступных докладов:\n";
                bool hasReports = false;
                int counter = 1;
                vector<shared_ptr<Report>> availableReports;
                
                for (const auto& report : reports) {
                    if (find(studentSubjects.begin(), studentSubjects.end(), 
                                 report->getSubjectName()) != studentSubjects.end() &&
                        !report->isFull() && !report->hasStudent(student->getId())) {
                        cout << counter++ << ". " << report->getTopic() 
                                  << " (Предмет: " << report->getSubjectName()
                                  << ", Участников: " << report->getSignedUpCount()
                                  << "/" << report->getMaxParticipants() << ")\n";
                        availableReports.push_back(report);
                        hasReports = true;
                    }
                }
                
                if (!hasReports) {
                    cout << "Нет доступных докладов по вашим предметам.\n";
                    break;
                }
                
                cout << "Выберите номер доклада: ";
                int reportNum;
                cin >> reportNum;
                cin.ignore();
                
                if (reportNum > 0 && reportNum <= static_cast<int>(availableReports.size())) {
                    auto report = availableReports[reportNum-1];
                    if (report->addStudent(student->getId())) {
                        cout << "Успешно записался на доклад: " << report->getTopic() << endl;
                        saveAllData();
                    } else {
                        cout << "Не могу записаться на доклад\n";
                    }
                } else {
                    cout << "Неверный номер доклада!\n";
                }
                break;
            }
            case 4: {
                cout << "Введите тему доклада: ";
                string reportTopic;
                getline(cin, reportTopic);
                
                auto report = findReport(reportTopic);
                if (report) {
                    if (report->removeStudent(student->getId())) {
                        cout << "Отписался от доклада: " << reportTopic << endl;
                        saveAllData();
                    } else {
                        cout << "Вы не записаны на этот доклад.\n";
                    }
                } else {
                    cout << "Доклад с такой темой не найден.\n";
                }
                break;
            }
            case 5: {
                showStudentSubjectSummary(student->getId());
                break;
            }
            case 6:
                logout();
                saveAllData();
                return;
            default:
                cout << "Неверный выбор!\n";
        }
    }
}

void UniversitySystem::runProfessorMenu(shared_ptr<Professor> professor) {
    while (true) {
        cout << "\n=== МЕНЮ ПРЕПОДАВАТЕЛЯ ===\n";
        cout << "Ваши предметы:\n";
        
        bool hasSubjects = false;
        for (const auto& subject : subjects) {
            if (subject->isProfessor(professor->getId())) {
                cout << "- " << subject->getName() 
                          << " (код: " << subject->getCode() << ")\n";
                hasSubjects = true;
            }
        }
        
        if (!hasSubjects) {
            cout << "  (нет предметов)\n";
        }
        
        cout << "\n1. Создать предмет\n";
        cout << "2. Создать задание\n";
        cout << "3. Создать доклад\n";
        cout << "4. Записать студента на предмет\n";
        cout << "5. Проверить сданные работы\n";
        cout << "6. Выставить оценку за доклад\n";
        cout << "7. Просмотреть статистику предмета\n";
        cout << "8. Создать итоговый отчет\n";
        cout << "9. Выход из системы\n";
        cout << "Выберите действие: ";
        
        int choice;
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case 1: {
                string name, code;
                cout << "Название предмета: ";
                getline(cin, name);
                cout << "Код предмета: ";
                getline(cin, code);
                
                auto existingSubject = findSubject(name);
                if (existingSubject) {
                    cout << "\nВнимание: предмет '" << name << "' уже существует!\n";
                    cout << "Текущий преподаватель: ID " << existingSubject->getProfessorId() << endl;
                    cout << "Вы хотите стать преподавателем этого предмета?\n";
                    cout << "1. Да, заменить преподавателя\n";
                    cout << "2. Нет, отменить создание\n";
                    cout << "Выберите: ";
                    
                    int choice;
                    cin >> choice;
                    cin.ignore();
                    
                    if (choice == 1) {
                        auto enrolledStudents = existingSubject->getEnrolledStudentIds();
                        auto assignmentsList = existingSubject->getAssignmentList();
                        auto reportsList = existingSubject->getReportList();
                        auto assignmentGrades = existingSubject->getAllAssignmentGrades();
                        auto reportGrades = existingSubject->getAllReportGrades();
                        
                        auto newSubject = make_shared<Subject>(name, code, professor->getId());
                        
                        for (int studentId : enrolledStudents) {
                            newSubject->enrollStudent(studentId);
                        }
                        
                        newSubject->setAssignmentGrades(assignmentGrades);
                        newSubject->setReportGrades(reportGrades);
                        
                        for (auto& subject : subjects) {
                            if (subject->getName() == name) {
                                subject = newSubject;
                                break;
                            }
                        }
                        
                        for (auto& assignment : assignments) {
                            if (assignment->getSubjectName() == name) {
                                assignment->setSubjectName(name);
                            }
                        }
                        
                        for (auto& report : reports) {
                            if (report->getSubjectName() == name) {
                                auto newReport = make_shared<Report>(
                                    report->getTopic(),
                                    name,
                                    report->getMaxParticipants()
                                );
                                
                                auto signedUpStudents = report->getSignedUpStudents();
                                for (int studentId : signedUpStudents) {
                                    newReport->addStudent(studentId);
                                }
                                
                                if (report->getIsCompleted()) {
                                    newReport->markAsCompleted();
                                }
                                
                                report = newReport;
                            }
                        }
                        
                        cout << "Вы теперь преподаватель предмета '" << name << "'\n";
                        cout << "Сохранено: " << enrolledStudents.size() << " студентов, " 
                                  << assignmentsList.size() << " заданий, " 
                                  << reportsList.size() << " докладов\n";
                        saveAllData();
                    } else {
                        cout << "Создание предмета отменено.\n";
                    }
                } else {
                    auto subject = professor->createSubject(name, code, professor->getId());
                    addSubject(subject);
                    cout << "Предмет '" << name << "' создан успешно!\n";
                }
                break;
            }
            case 2: {
                bool hasSubjects = false;
                cout << "Ваши предметы:\n";
                for (const auto& subject : subjects) {
                    if (subject->isProfessor(professor->getId())) {
                        cout << "- " << subject->getName() 
                                  << " (код: " << subject->getCode() << ")\n";
                        hasSubjects = true;
                    }
                }
                
                if (!hasSubjects) {
                    cout << "У вас нет предметов. Сначала создайте предмет.\n";
                    break;
                }
                
                cout << "Введите название предмета или код: ";
                string identifier;
                getline(cin, identifier);
                
                auto subject = findSubjectByNameOrCode(identifier);
                if (subject && subject->isProfessor(professor->getId())) {
                    string name;
                    double maxScore;
                    cout << "Название задания: ";
                    getline(cin, name);
                    cout << "Максимальный балл: ";
                    cin >> maxScore;
                    cin.ignore();
                    
                    auto assignment = professor->createAssignment(name, "", subject);
                    assignment->setMaxScore(maxScore);
                    addAssignment(assignment);
                    cout << "Задание '" << name << "' создано с максимальным баллом: " 
                              << maxScore << endl;
                } else {
                    cout << "Предмет не найден или вы не ведете его!\n";
                }
                break;
            }
            case 3: {
                bool hasSubjects = false;
                cout << "Ваши предметы:\n";
                for (const auto& subject : subjects) {
                    if (subject->isProfessor(professor->getId())) {
                        cout << "- " << subject->getName() 
                                  << " (код: " << subject->getCode() << ")\n";
                        hasSubjects = true;
                    }
                }
                
                if (!hasSubjects) {
                    cout << "У вас нет предметов. Сначала создайте предмет.\n";
                    break;
                }
                
                cout << "Введите название предмета или код: ";
                string identifier;
                getline(cin, identifier);
                
                auto subject = findSubjectByNameOrCode(identifier);
                if (subject && subject->isProfessor(professor->getId())) {
                    string topic;
                    int maxParticipants;
                    cout << "Тема доклада: ";
                    getline(cin, topic);
                    cout << "Макс. участников: ";
                    cin >> maxParticipants;
                    cin.ignore();
                    
                    auto report = professor->createReport(topic, subject, maxParticipants);
                    addReport(report);
                } else {
                    cout << "Предмет не найден или вы не ведете его!\n";
                }
                break;
            }
            case 4: {
                bool hasSubjects = false;
                cout << "Ваши предметы:\n";
                for (const auto& subject : subjects) {
                    if (subject->isProfessor(professor->getId())) {
                        cout << "- " << subject->getName() 
                                  << " (код: " << subject->getCode() << ")\n";
                        hasSubjects = true;
                    }
                }
                
                if (!hasSubjects) {
                    cout << "У вас нет предметов. Сначала создайте предмет.\n";
                    break;
                }
                
                listAllStudents();
                cout << "Введите ID студента: ";
                int studentId;
                cin >> studentId;
                cin.ignore();
                
                cout << "Введите название предмета или код: ";
                string identifier;
                getline(cin, identifier);
                
                auto subject = findSubjectByNameOrCode(identifier);
                if (subject && subject->isProfessor(professor->getId())) {
                    enrollStudentInSubject(studentId, identifier);
                } else {
                    cout << "Предмет не найден или вы не ведете его!\n";
                }
                break;
            }
            case 5: {
                vector<SubmissionRecord> professorPending;
                for (const auto& sub : submissions) {
                    if (sub.status == "pending") {
                        auto subject = findSubject(sub.subjectName);
                        if (subject && subject->isProfessor(professor->getId())) {
                            professorPending.push_back(sub);
                        }
                    }
                }
                
                if (professorPending.empty()) {
                    cout << "Нет работ на проверку по вашим предметам.\n";
                } else {
                    cout << "Работы на проверку по вашим предметам (" << professorPending.size() << "):\n";
                    for (size_t i = 0; i < professorPending.size(); i++) {
                        const auto& sub = professorPending[i];
                        auto student = findStudentById(sub.studentId);
                        string studentName = student ? student->getName() : "Неизвестный";
                        
                        double maxScore = 100.0;
                        for (const auto& assignment : assignments) {
                            if (assignment->getName() == sub.assignmentName && 
                                assignment->getSubjectName() == sub.subjectName) {
                                maxScore = assignment->getMaxScore();
                                break;
                            }
                        }
                        
                        cout << i+1 << ". Студент: " << studentName 
                                  << " (ID: " << sub.studentId << ")"
                                  << ", Предмет: " << sub.subjectName 
                                  << ", Задание: " << sub.assignmentName 
                                  << " (макс. балл: " << maxScore << ")"
                                  << " (отправлено: " << sub.timestamp << ")\n";
                    }
                    
                    cout << "\nВыберите работу для проверки (номер): ";
                    int workNum;
                    cin >> workNum;
                    cin.ignore();
                    
                    if (workNum > 0 && workNum <= static_cast<int>(professorPending.size())) {
                        auto& sub = professorPending[workNum-1];
                        auto student = findStudentById(sub.studentId);
                        string studentName = student ? student->getName() : "Неизвестный";
                        
                        cout << "\nВы выбрали работу:\n";
                        cout << "Студент: " << studentName << " (ID: " << sub.studentId << ")\n";
                        cout << "Предмет: " << sub.subjectName << endl;
                        cout << "Задание: " << sub.assignmentName << endl;
                        
                        double maxScore = 100.0;
                        for (const auto& assignment : assignments) {
                            if (assignment->getName() == sub.assignmentName && 
                                assignment->getSubjectName() == sub.subjectName) {
                                maxScore = assignment->getMaxScore();
                                break;
                            }
                        }
                        cout << "Максимальный балл: " << maxScore << endl;
                        
                        cout << "\n1. Утвердить и выставить оценку\n";
                        cout << "2. Отклонить\n";
                        cout << "Выберите действие: ";
                        int action;
                        cin >> action;
                        cin.ignore();
                        
                        if (action == 1) {
                            cout << "Введите оценку (0-" << maxScore << "): ";
                            double grade;
                            cin >> grade;
                            cin.ignore();
                            
                            gradeAssignment(sub.studentId, sub.subjectName, sub.assignmentName, grade);
                        } else if (action == 2) {
                            for (auto& s : submissions) {
                                if (s.studentId == sub.studentId && 
                                    s.subjectName == sub.subjectName && 
                                    s.assignmentName == sub.assignmentName &&
                                    s.status == "pending") {
                                    s.status = "rejected";
                                    cout << "Работа отклонена. Студент может пересдать.\n";
                                    saveAllData();
                                    break;
                                }
                            }
                        }
                    } else {
                        cout << "Неверный номер работы!\n";
                    }
                }
                break;
            }
            case 6: {
                vector<shared_ptr<Report>> professorReports;
                for (const auto& report : reports) {
                    auto subject = findSubject(report->getSubjectName());
                    if (subject && subject->isProfessor(professor->getId())) {
                        bool hasGrades = false;
                        for (const auto& grade : grades) {
                            if (grade.type == "report" && grade.itemName == report->getTopic()) {
                                hasGrades = true;
                                break;
                            }
                        }
                        
                        if (!hasGrades) {
                            professorReports.push_back(report);
                        }
                    }
                }
                
                if (professorReports.empty()) {
                    cout << "Нет докладов для оценки по вашим предметам.\n";
                    break;
                }
                
                cout << "Доступные для оценки доклады по вашим предметам:\n";
                for (size_t i = 0; i < professorReports.size(); i++) {
                    const auto& report = professorReports[i];
                    cout << i+1 << ". " << report->getTopic() 
                              << " (Предмет: " << report->getSubjectName()
                              << ", Участников: " << report->getSignedUpCount() << ")\n";
                }
                
                cout << "Выберите номер доклада: ";
                int reportNum;
                cin >> reportNum;
                cin.ignore();
                
                if (reportNum > 0 && reportNum <= static_cast<int>(professorReports.size())) {
                    auto report = professorReports[reportNum-1];
                    string subjectName = report->getSubjectName();
                    string reportName = report->getTopic();
                    
                    auto participants = report->getSignedUpStudents();
                    
                    if (participants.empty()) {
                        cout << "На этот доклад не записан ни один студент.\n";
                        break;
                    }
                    
                    cout << "Студенты, записанные на доклад '" << reportName << "':\n";
                    for (int studentId : participants) {
                        auto student = findStudentById(studentId);
                        if (student) {
                            cout << "- " << student->getName() << " (ID: " << studentId << ")\n";
                        }
                    }
                    
                    cout << "Введите оценку для всех участников (0-100): ";
                    double grade;
                    cin >> grade;
                    cin.ignore();
                    
                    gradeReport(subjectName, reportName, grade);
                } else {
                    cout << "Неверный номер доклада!\n";
                }
                break;
            }
            case 7: {
                bool hasSubjects = false;
                cout << "Ваши предметы:\n";
                for (const auto& subject : subjects) {
                    if (subject->isProfessor(professor->getId())) {
                        cout << "- " << subject->getName() 
                                  << " (код: " << subject->getCode() << ")\n";
                        hasSubjects = true;
                    }
                }
                
                if (!hasSubjects) {
                    cout << "У вас нет предметов.\n";
                    break;
                }
                
                cout << "Введите название предмета или код: ";
                string identifier;
                getline(cin, identifier);
                
                auto subject = findSubjectByNameOrCode(identifier);
                if (subject && subject->isProfessor(professor->getId())) {
                    showSubjectStatistics(subject->getName());
                } else {
                    cout << "Предмет не найден или вы не ведете его!\n";
                }
                break;
            }
            case 8: {
                bool hasSubjects = false;
                cout << "Ваши предметы:\n";
                for (const auto& subject : subjects) {
                    if (subject->isProfessor(professor->getId())) {
                        cout << "- " << subject->getName() 
                                  << " (код: " << subject->getCode() << ")\n";
                        hasSubjects = true;
                    }
                }
                
                if (!hasSubjects) {
                    cout << "У вас нет предметов.\n";
                    break;
                }
                
                cout << "Введите название предмета или код: ";
                string identifier;
                getline(cin, identifier);
                
                auto subject = findSubjectByNameOrCode(identifier);
                if (subject && subject->isProfessor(professor->getId())) {
                    map<int, string> studentNames;
                    auto enrolledStudents = subject->getEnrolledStudents();
                    for (int studentId : enrolledStudents) {
                        auto student = findStudentById(studentId);
                        if (student) {
                            studentNames[studentId] = student->getName();
                        }
                    }
                    
                    subject->generateFinalReport(studentNames);
                } else {
                    cout << "Предмет не найден или вы не ведете его!\n";
                }
                break;
            }
            case 9:
                logout();
                saveAllData();
                return;
            default:
                cout << "Неверный выбор!\n";
        }
    }
}

