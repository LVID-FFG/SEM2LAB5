#include "university_system.h"
#include <iostream>

using namespace std;

int main() {
    UniversitySystem system;
    
    cout << "========================================\n";
    cout << "    УНИВЕРСИТЕТСКАЯ СИСТЕМА\n";
    cout << "========================================\n";
    system.run();
    
    return 0;
}
//g++ -o lab5 data_manager.cpp lab5.cpp object.cpp professor.cpp student.cpp university_system.cpp user.cpp