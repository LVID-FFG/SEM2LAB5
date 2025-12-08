#pragma once
#include "user.h"
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

using namespace std;

class Student : public User {
public:
    Student(const string& name, const string& password);
    Student(const string& name, const string& passwordHash, int id);
    
    Role getRole() const override { return Role::STUDENT; }
    string getRoleString() const override { return "Студент"; }
    void displayInfo() const override;
    void save(ostream& file) const override;
    
    void onGradeUpdated(const string& subjectName, const string& assignment, double grade);
    
    static shared_ptr<Student> create(const string& name, const string& password);
    static shared_ptr<Student> load(const string& name, const string& passwordHash, int id);
};