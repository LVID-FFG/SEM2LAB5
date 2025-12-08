#include "professor.h"
#include "object.h"
#include <iostream>

using namespace std;

Professor::Professor(const string& name, const string& password)
    : User(name, password, Role::PROFESSOR) {}

Professor::Professor(const string& name, const string& passwordHash, int id)
    : User(name, passwordHash, id, Role::PROFESSOR) {}

void Professor::displayInfo() const {
    cout << "Преподаватель: " << name << " (ID: " << id << ")\n";
}

void Professor::save(ostream& file) const {
    file << id << "," << name << "," << passwordHash << "," 
         << static_cast<int>(role) << "\n";
}

shared_ptr<Subject> Professor::createSubject(const string& name, 
                                                 const string& code,
                                                 int professorId) {
    auto subject = make_shared<Subject>(name, code, professorId);
    cout << "Предмет '" << name << "' создан успешно!\n";
    return subject;
}

shared_ptr<Assignment> Professor::createAssignment(const string& name, 
                                                       const string& description,
                                                       shared_ptr<Subject> subject) {
    auto assignment = make_shared<Assignment>(name, subject->getName(), 100.0);
    subject->addAssignment(name);
    cout << "Задание '" << name << "' создано успешно!\n";
    return assignment;
}

shared_ptr<Report> Professor::createReport(const string& topic, 
                                               shared_ptr<Subject> subject,
                                               int maxParticipants) {
    auto report = make_shared<Report>(topic, subject->getName(), maxParticipants);
    subject->addReport(topic);
    cout << "Доклад '" << topic << "' создан успешно!\n";
    return report;
}

shared_ptr<Professor> Professor::create(const string& name, 
                                            const string& password) {
    return make_shared<Professor>(name, password);
}

shared_ptr<Professor> Professor::load(const string& name, 
                                          const string& passwordHash, int id) {
    return make_shared<Professor>(name, passwordHash, id);
}