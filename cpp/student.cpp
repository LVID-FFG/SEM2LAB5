#include "student.h"
#include "object.h"
#include <iostream>

using namespace std;

Student::Student(const string& name, const string& password)
    : User(name, password, Role::STUDENT) {}

Student::Student(const string& name, const string& passwordHash, int id)
    : User(name, passwordHash, id, Role::STUDENT) {}

void Student::displayInfo() const {
    cout << "Студент: " << name << " (ID: " << id << ")\n";
}

void Student::save(ostream& file) const {
    file << id << "," << name << "," << passwordHash << "," 
         << static_cast<int>(role) << "\n";
}

void Student::onGradeUpdated(const string& subjectName, 
                            const string& assignment, 
                            double grade) {
    cout << "\n=== УВЕДОМЛЕНИЕ ОБ ОЦЕНКЕ ===\n";
    cout << "Студент: " << name << "\n";
    cout << "Предмет: " << subjectName << "\n";
    cout << "Задание: " << assignment << "\n";
    cout << "Оценка: " << grade << "\n";
    cout << "================================\n\n";
}

shared_ptr<Student> Student::create(const string& name, 
                                        const string& password) {
    return make_shared<Student>(name, password);
}

shared_ptr<Student> Student::load(const string& name, 
                                      const string& passwordHash, int id) {
    return make_shared<Student>(name, passwordHash, id);
}