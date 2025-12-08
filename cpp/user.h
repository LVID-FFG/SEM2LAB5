#pragma once
#include <string>
#include <memory>
#include <functional>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>

using namespace std;

class User {
protected:
    string name;                           // Имя пользователя (уникальный логин)
    string passwordHash;                   // Хэш пароля
    int id;                                // Уникальный числовой идентификатор
    static int nextId;                     // Статическая переменная для генерации ID
    
public:
    enum class Role { STUDENT = 0, PROFESSOR = 1 } role;
    
    User(const string& name, const string& password, Role role);  // Для нового пользователя
    User(const string& name, const string& passwordHash, int id, Role role);  // Для загрузки из файла
    virtual ~User() = default;
    bool checkPassword(const string& password) const;

    virtual Role getRole() const = 0;                // Получить роль
    virtual string getRoleString() const = 0;        // Текстовое представление роли
    
    string getName() const { return name; }
    int getId() const { return id; }
    string getPasswordHash() const { return passwordHash; }
    
    virtual void displayInfo() const = 0;            // Вывести информацию о пользователе
    virtual void save(ostream& file) const = 0;      // Сохранить в поток
    
    static string hashPassword(const string& password);
    static void updateNextId(int newNextId) { nextId = newNextId; }
    static int getNextId() { return nextId; }
};