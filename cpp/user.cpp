#include "user.h"
#include "data_manager.h"

using namespace std;

int User::nextId = 1;

User::User(const string& name, const string& password, Role role)
    : name(name), role(role) {
    this->id = nextId++;
    this->passwordHash = hashPassword(password);  // Прямой вызов метода хэширования
}

User::User(const string& name, const string& passwordHash, int id, Role role)
    : name(name), passwordHash(passwordHash), id(id), role(role) {
    if (id >= nextId) {
        nextId = id + 1;
    }
}

bool User::checkPassword(const string& password) const {
    return passwordHash == hashPassword(password);
}

string User::hashPassword(const string& password) {
    hash<string> hasher;
    auto hashValue = hasher(password + "university_salt_2024");
    stringstream ss;
    ss << hex << hashValue;
    return ss.str();
}
