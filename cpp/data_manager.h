#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>

class User;
class Subject;
class Assignment;
class Report;

using namespace std;

// структура для серелизации данных о заданиях
// Используется при сохранении в файл / загрузке из файла
struct DataSubmission {
    int studentId;          // ID студента
    string subjectName;     // Название предмета
    string assignmentName;  // Название задания/доклада
    string status;          // Статус: "pending", "approved", "rejected"
    string timestamp;       // Время сдачи
};

// структура для серелизации данных о оценке
struct DataGrade {
    int studentId;          // ID студента
    string subjectName;     // Название предмета
    string assignmentName;  // Название задания/доклада
    double score;           // Оценка
    string type;            // Тип: "assignment" (задание) или "report" (доклад)
    string timestamp;       // Время выставления оценки
};

// структура для хранения оценок
struct DataSubjectGrades {
    string subjectName;                             // Название предмета
    map<int, map<string, double>> assignmentGrades; // Оценки за задания
    map<int, map<string, double>> reportGrades;     // Оценки за доклады
};

class DataManager {
private:
    static const string DATA_DIR;  // Путь к директории данных
    
public:
    static void initDataDirectory();  // Создает папку "data/" если её нет
    
    // сохранение данных, каждый метод записывает свой тип данных в отдельный файл
    static void saveUsers(const map<string, shared_ptr<User>>& users);           // users.txt
    static void saveSubjects(const vector<shared_ptr<Subject>>& subjects);       // subjects.txt
    static void saveAssignments(const vector<shared_ptr<Assignment>>& assignments); // assignments.txt
    static void saveReports(const vector<shared_ptr<Report>>& reports);          // reports.txt
    static void saveEnrollments(const map<int, vector<string>>& studentEnrollments); // enrollments.txt
    static void saveSubmissions(const vector<DataSubmission>& submissions);      // submissions.txt
    static void saveGrades(const vector<DataGrade>& grades);                     // grades.txt
    static void saveSubjectGrades(const vector<shared_ptr<Subject>>& subjects);  // subject_grades.txt
    static void saveNextUserId(int nextId);                                      // next_id.txt
    
    // чтение из файлов и восстановление объектов
    static map<string, shared_ptr<User>> loadUsers();
    static vector<shared_ptr<Subject>> loadSubjects();
    static vector<shared_ptr<Assignment>> loadAssignments();
    static vector<shared_ptr<Report>> loadReports();
    static map<int, vector<string>> loadEnrollments();
    static vector<DataSubmission> loadSubmissions();
    static vector<DataGrade> loadGrades();
    static map<string, DataSubjectGrades> loadSubjectGrades();
    static int loadNextUserId();  // Загрузка следующего доступного ID пользователя
    
    static string getCurrentTimestamp();  // Получение текущей даты/времени в формате строки
    
    static void saveAllData(const map<string, shared_ptr<User>>& users,
                           const vector<shared_ptr<Subject>>& subjects,
                           const vector<shared_ptr<Assignment>>& assignments,
                           const vector<shared_ptr<Report>>& reports,
                           const map<int, vector<string>>& studentEnrollments,
                           const vector<DataSubmission>& submissions,
                           const vector<DataGrade>& grades);
};