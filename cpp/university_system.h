};

struct GradeRecord {
    int studentId;          // ID студента
    string subjectName;     // Название предмета
    string itemName;        // Название задания или доклада
    string type;            // Тип: "assignment" или "report"
    double score;           // Оценка
    string timestamp;       // Время выставления
};

class UniversitySystem {
private:
    map<string, shared_ptr<User>> users;       // Все пользователи по имени
    map<int, shared_ptr<Student>> students;    // Студенты по ID
    map<int, shared_ptr<Professor>> professors; // Преподаватели по ID
    
    vector<shared_ptr<Subject>> subjects;      // Все предметы
    vector<shared_ptr<Assignment>> assignments; // Все задания
    vector<shared_ptr<Report>> reports;        // Все доклады
    
    map<int, vector<string>> studentEnrollments; // Записи студентов на предметы
    map<string, vector<int>> subjectEnrollments; // Записи по предметам
    vector<SubmissionRecord> submissions;        // Все сдачи работ
    vector<GradeRecord> grades;                  // Все оценки
    
    shared_ptr<User> currentUser;                // Текущий авторизованный пользователь
    
    void loadAllData();                          // Загрузка всех данных при запуске
    void saveAllData();                          // Сохранение всех данных
    void showMainMenu();                         // Отображение главного меню (до входа)
    
    bool isStudentAlreadyEnrolled(int studentId, const string& subjectName) const; // Проверка двойной записи
    shared_ptr<Subject> findSubjectByNameOrCode(const string& identifier) const;   // Поиск по названию или коду
    void removeReport(const string& subjectName, const string& reportName);        // Удаление доклада после оценки
    shared_ptr<Report> findReportForSubject(const string& subjectName, const string& reportName) const;  // Поиск доклада по предмету
    
    shared_ptr<Subject> findSubject(const string& name) const;  // Поиск предмета по имени
    shared_ptr<Report> findReport(const string& topic) const;   // Поиск доклада по теме
    shared_ptr<Student> findStudentById(int id) const;          // Поиск студента по ID
    
    void addSubject(shared_ptr<Subject> subject);                // Добавление нового предмета
    void enrollStudentInSubject(int studentId, const string& identifier); // Запись студента на предмет
    vector<string> getStudentSubjects(int studentId) const;      // Получение предметов студента
    
    void addAssignment(shared_ptr<Assignment> assignment);       // Добавление задания
    void addReport(shared_ptr<Report> report);                   // Добавление доклада
    
    bool submitAssignment(int studentId, const string& subjectName,  // Сдача задания
                         const string& assignmentName);
    bool submitReport(int studentId, const string& subjectName,      // Сдача доклада
                     const string& reportName);
    
    bool gradeAssignment(int studentId, const string& subjectName,   // Оценка за задание
                        const string& assignmentName, double grade);
    bool gradeReport(const string& identifier,                       // Оценка за доклад
                    const string& reportName, double grade);
    
    vector<SubmissionRecord> getPendingSubmissions(const string& subjectName = "") const;  // Работы на проверке
    
    void listAllSubjects() const;    // Список всех предметов
    void listAllReports() const;     // Список всех докладов
    void listAllStudents() const;    // Список всех студентов
    void listAllProfessors() const;  // Список всех преподавателей
    
    void showSubjectStatistics(const string& subjectName) const;    // Статистика по предмету
    void showStudentSubjectSummary(int studentId) const;            // Итоги по предметам для студента
    
    bool login(const string& name, const string& password);  // Вход в систему
    void logout();                                           // Выход из системы
    bool registerUser(const string& name, const string& password, User::Role role); // Регистрация нового пользователя
    
    void runStudentMenu(shared_ptr<Student> student);      // Меню для студента
    void runProfessorMenu(shared_ptr<Professor> professor); // Меню для преподавателя
    
public:
    UniversitySystem();
    ~UniversitySystem();
    
    void run();                                            // Главный цикл программы
};
