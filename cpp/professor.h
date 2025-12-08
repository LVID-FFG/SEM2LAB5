#pragma once
#include "user.h"
#include <vector>
#include <memory>
#include <string>

using namespace std;

class Subject;
class Assignment;
class Report;

class Professor : public User {
public:
    Professor(const string& name, const string& password);
    Professor(const string& name, const string& passwordHash, int id);
    
    Role getRole() const override { return Role::PROFESSOR; }
    string getRoleString() const override { return "Преподаватель"; }
    void displayInfo() const override;
    void save(ostream& file) const override;
    
    shared_ptr<Subject> createSubject(const string& name, const string& code, int professorId);
    shared_ptr<Assignment> createAssignment(const string& name, const string& description, shared_ptr<Subject> subject);
    shared_ptr<Report> createReport(const string& topic, shared_ptr<Subject> subject, int maxParticipants = 5);
    
    static shared_ptr<Professor> create(const string& name, const string& password);
    static shared_ptr<Professor> load(const string& name, const string& passwordHash, int id);
};