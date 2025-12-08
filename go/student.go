package main

import "fmt"

type Student struct {
	*User
}

func NewStudent(name, password string) *Student {
	return &Student{
		User: NewUser(name, password, RoleStudent),
	}
}

func LoadStudent(name, passwordHash string, id int) *Student {
	return &Student{
		User: LoadUser(name, passwordHash, id, RoleStudent),
	}
}

func (s *Student) OnGradeUpdated(subjectName, assignment string, grade float64) {
	fmt.Println("\n=== УВЕДОМЛЕНИЕ ОБ ОЦЕНКЕ ===")
	fmt.Printf("Студент: %s\n", s.Name)
	fmt.Printf("Предмет: %s\n", subjectName)
	fmt.Printf("Задание: %s\n", assignment)
	fmt.Printf("Оценка: %.2f\n", grade)
	fmt.Println("================================")
}
