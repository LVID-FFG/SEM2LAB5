package main

import "fmt"

type Professor struct {
	*User
}

func NewProfessor(name, password string) *Professor {
	return &Professor{
		User: NewUser(name, password, RoleProfessor),
	}
}

func LoadProfessor(name, passwordHash string, id int) *Professor {
	return &Professor{
		User: LoadUser(name, passwordHash, id, RoleProfessor),
	}
}

func (p *Professor) CreateSubject(name, code string, professorId int) *Subject {
	subject := NewSubject(name, code, professorId)
	fmt.Printf("Предмет '%s' создан успешно!\n", name)
	return subject
}

func (p *Professor) CreateAssignment(name, description string, subject *Subject) *Assignment {
	assignment := NewAssignment(name, subject.Name, 100.0)
	subject.AddAssignment(name)
	fmt.Printf("Задание '%s' создано успешно!\n", name)
	return assignment
}

func (p *Professor) CreateReport(topic string, subject *Subject, maxParticipants int) *Report {
	report := NewReport(topic, subject.Name, maxParticipants)
	subject.AddReport(topic)
	fmt.Printf("Доклад '%s' создан успешно!\n", topic)
	return report
}
