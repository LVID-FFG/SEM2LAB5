#include "user.h"
#include "data_manager.h"

using namespace std;

int User::nextId = 1;

// КОНСТРУКТОР ДЛЯ НОВОГО ПОЛЬЗОВАТЕЛЯ
User::User(const string& name, const string& password, Role role)
    : name(name), role(role) {
    this->id = nextId++;
    this->passwordHash = hashPassword(password);  // Прямой вызов метода хэширования
}

// КОНСТРУКТОР ДЛЯ ЗАГРУЗКИ ПОЛЬЗОВАТЕЛЯ ИЗ ФАЙЛА
User::User(const string& name, const string& passwordHash, int id, Role role)
    : name(name), passwordHash(passwordHash), id(id), role(role) {
    if (id >= nextId) {
        nextId = id + 1;
    }
}

// ПРОВЕРКА ПАРОЛЯ (простая прямая реализация)
bool User::checkPassword(const string& password) const {
    return passwordHash == hashPassword(password);
}

// СТАТИЧЕСКИЙ МЕТОД ХЭШИРОВАНИЯ ПАРОЛЯ
string User::hashPassword(const string& password) {
    hash<string> hasher;
    auto hashValue = hasher(password + "university_salt_2024");
    stringstream ss;
    ss << hex << hashValue;
    return ss.str();
}