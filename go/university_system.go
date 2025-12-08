package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

type SubmissionRecord struct {
	StudentID      int
	SubjectName    string
	AssignmentName string
	Type           string
	Status         string
	Timestamp      string
}

type GradeRecord struct {
	StudentID   int
	SubjectName string
	ItemName    string
	Type        string
	Score       float64
	Timestamp   string
}

type UniversitySystem struct {
	users              map[string]*User
	students           map[int]*Student
	professors         map[int]*Professor
	subjects           []*Subject
	assignments        []*Assignment
	reports            []*Report
	studentEnrollments map[int][]string
	subjectEnrollments map[string][]int
	submissions        []SubmissionRecord
	grades             []GradeRecord
	currentUser        *User
	dataManager        *DataManager
}

func NewUniversitySystem() *UniversitySystem {
	system := &UniversitySystem{
		users:              make(map[string]*User),
		students:           make(map[int]*Student),
		professors:         make(map[int]*Professor),
		subjects:           make([]*Subject, 0),
		assignments:        make([]*Assignment, 0),
		reports:            make([]*Report, 0),
		studentEnrollments: make(map[int][]string),
		subjectEnrollments: make(map[string][]int),
		submissions:        make([]SubmissionRecord, 0),
		grades:             make([]GradeRecord, 0),
		dataManager:        &DataManager{},
	}

	system.dataManager.InitDataDirectory()
	system.LoadAllData()
	return system
}

func (s *UniversitySystem) LoadAllData() {
	// Загружаем следующий ID
	nextId := s.dataManager.LoadNextUserId()
	UpdateNextId(nextId)

	// Загружаем пользователей
	s.users = s.dataManager.LoadUsers()

	// Загружаем предметы
	s.subjects = s.dataManager.LoadSubjects()

	// Загружаем задания
	s.assignments = s.dataManager.LoadAssignments()

	// Загружаем доклады
	s.reports = s.dataManager.LoadReports()

	// Загружаем записи на предметы
	s.studentEnrollments = s.dataManager.LoadEnrollments()

	// Загружаем отправки
	s.submissions = s.dataManager.LoadSubmissions()

	// Загружаем оценки
	s.grades = s.dataManager.LoadGrades()

	// Загружаем оценки по предметам
	subjectGradesMap := s.dataManager.LoadSubjectGrades()

	// Инициализируем студентов и преподавателей
	for _, user := range s.users {
		switch user.Role {
		case RoleStudent:
			s.students[user.ID] = &Student{User: user}
		case RoleProfessor:
			s.professors[user.ID] = &Professor{User: user}
		}
	}

	// Загружаем связи предметов и студентов
	for studentID, subjectNames := range s.studentEnrollments {
		for _, subjectName := range subjectNames {
			subject := s.findSubject(subjectName)
			if subject != nil {
				subject.EnrollStudent(studentID)
				s.subjectEnrollments[subjectName] = append(s.subjectEnrollments[subjectName], studentID)
			}
		}
	}

	// Загружаем задания в предметы
	for _, assignment := range s.assignments {
		subject := s.findSubject(assignment.SubjectName)
		if subject != nil {
			subject.AddAssignment(assignment.Name)
		}
	}

	// Загружаем доклады в предметы
	for _, report := range s.reports {
		subject := s.findSubject(report.SubjectName)
		if subject != nil {
			subject.AddReport(report.Topic)
		}
	}

	// Загружаем оценки в предметы
	for subjectName, subjectGrades := range subjectGradesMap {
		subject := s.findSubject(subjectName)
		if subject == nil {
			continue
		}

		// Загружаем оценки за задания
		for studentID, assignmentGrades := range subjectGrades.AssignmentGrades {
			// Убеждаемся, что студент зачислен
			if !subject.IsStudentEnrolled(studentID) {
				subject.EnrollStudent(studentID)
				s.studentEnrollments[studentID] = append(s.studentEnrollments[studentID], subjectName)
				s.subjectEnrollments[subjectName] = append(s.subjectEnrollments[subjectName], studentID)
			}

			// Загружаем оценки
			for assignmentName, grade := range assignmentGrades {
				subject.GradeAssignment(studentID, assignmentName, grade)
			}
		}

		// Загружаем оценки за доклады
		for studentID, reportGrades := range subjectGrades.ReportGrades {
			// Убеждаемся, что студент зачислен
			if !subject.IsStudentEnrolled(studentID) {
				subject.EnrollStudent(studentID)
				s.studentEnrollments[studentID] = append(s.studentEnrollments[studentID], subjectName)
				s.subjectEnrollments[subjectName] = append(s.subjectEnrollments[subjectName], studentID)
			}

			// Загружаем оценки
			for reportName, grade := range reportGrades {
				subject.GradeReport(studentID, reportName, grade)
			}
		}
	}
}

func (s *UniversitySystem) SaveAllData() {
	s.dataManager.SaveAllData(s)
}

func (s *UniversitySystem) Login(name, password string) bool {
	user, exists := s.users[name]
	if !exists {
		fmt.Println("Пользователь не найден!")
		return false
	}

	if user.CheckPassword(password) {
		s.currentUser = user
		fmt.Println("\n=== Вход успешен! ===")
		fmt.Printf("Добро пожаловать, %s (%s)\n", name, user.GetRoleString())
		return true
	}

	fmt.Println("Неверный пароль!")
	return false
}

func (s *UniversitySystem) Logout() {
	if s.currentUser != nil {
		fmt.Printf("Выход из системы пользователя: %s\n", s.currentUser.Name)
		s.currentUser = nil
	}
}

func (s *UniversitySystem) RegisterUser(name, password string, role Role) bool {
	if _, exists := s.users[name]; exists {
		fmt.Println("Пользователь с таким именем уже существует!")
		return false
	}

	switch role {
	case RoleStudent:
		student := NewStudent(name, password)
		s.users[name] = student.User
		s.students[student.ID] = student
	case RoleProfessor:
		professor := NewProfessor(name, password)
		s.users[name] = professor.User
		s.professors[professor.ID] = professor
	}

	fmt.Printf("Пользователь %s успешно зарегистрирован!\n", name)
	s.SaveAllData()
	return true
}

func (s *UniversitySystem) Run() {
	scanner := bufio.NewScanner(os.Stdin)

	for {
		if s.currentUser == nil {
			s.ShowMainMenu(scanner)
		} else {
			switch s.currentUser.Role {
			case RoleStudent:
				s.RunStudentMenu(scanner, s.students[s.currentUser.ID])
			case RoleProfessor:
				s.RunProfessorMenu(scanner, s.professors[s.currentUser.ID])
			}
		}
	}
}

func (s *UniversitySystem) ShowMainMenu(scanner *bufio.Scanner) {
	fmt.Println("\n=== УНИВЕРСИТЕТСКАЯ СИСТЕМА ===")
	fmt.Println("1. Вход")
	fmt.Println("2. Регистрация")
	fmt.Println("3. Выход")
	fmt.Print("Выберите действие: ")

	scanner.Scan()
	choiceStr := scanner.Text()
	choice, _ := strconv.Atoi(choiceStr)

	switch choice {
	case 1:
		fmt.Print("Имя: ")
		scanner.Scan()
		name := scanner.Text()

		fmt.Print("Пароль: ")
		scanner.Scan()
		password := scanner.Text()

		s.Login(name, password)
	case 2:
		fmt.Print("Имя: ")
		scanner.Scan()
		name := scanner.Text()

		fmt.Print("Пароль: ")
		scanner.Scan()
		password := scanner.Text()

		fmt.Println("Выберите роль:")
		fmt.Println("1. Студент")
		fmt.Println("2. Преподаватель")
		fmt.Print("Выбор: ")

		scanner.Scan()
		roleChoiceStr := scanner.Text()
		roleChoice, _ := strconv.Atoi(roleChoiceStr)

		var role Role
		switch roleChoice {
		case 1:
			role = RoleStudent
		case 2:
			role = RoleProfessor
		default:
			fmt.Println("Неверный выбор!")
			return
		}

		s.RegisterUser(name, password, role)
	case 3:
		s.SaveAllData()
		fmt.Println("До свидания!")
		os.Exit(0)
	default:
		fmt.Println("Неверный выбор!")
	}
}

// Основные методы для студентов
func (s *UniversitySystem) RunStudentMenu(scanner *bufio.Scanner, student *Student) {
	for {
		fmt.Println("\n=== МЕНЮ СТУДЕНТА ===")
		fmt.Println("1. Просмотреть мои предметы")
		fmt.Println("2. Сдать задание")
		fmt.Println("3. Записаться на доклад")
		fmt.Println("4. Отказаться от доклада")
		fmt.Println("5. Посмотреть мои оценки по предметам")
		fmt.Println("6. Выход из системы")
		fmt.Print("Выберите действие: ")

		scanner.Scan()
		choiceStr := scanner.Text()
		choice, _ := strconv.Atoi(choiceStr)

		switch choice {
		case 1:
			s.showStudentSubjects(student)
		case 2:
			s.submitAssignmentMenu(scanner, student)
		case 3:
			s.signUpForReportMenu(scanner, student)
		case 4:
			s.optOutOfReportMenu(scanner, student)
		case 5:
			s.showStudentGrades(student)
		case 6:
			s.Logout()
			s.SaveAllData()
			return
		default:
			fmt.Println("Неверный выбор!")
		}
	}
}

func (s *UniversitySystem) showStudentSubjects(student *Student) {
	subjects, exists := s.studentEnrollments[student.ID]
	if !exists || len(subjects) == 0 {
		fmt.Println("Вы не зачислены ни на один предмет.")
		return
	}

	// Убираем дубликаты
	uniqueSubjects := make(map[string]bool)
	for _, subject := range subjects {
		uniqueSubjects[subject] = true
	}

	fmt.Printf("Ваши предметы (%d):\n", len(uniqueSubjects))
	for subjectName := range uniqueSubjects {
		subject := s.findSubject(subjectName)
		if subject != nil {
			fmt.Printf("- %s (код: %s)\n", subjectName, subject.Code)
		}
	}
}

func (s *UniversitySystem) submitAssignmentMenu(scanner *bufio.Scanner, student *Student) {
	subjects := s.GetStudentSubjects(student.ID)
	if len(subjects) == 0 {
		fmt.Println("Вы не зачислены ни на один предмет.")
		return
	}

	fmt.Println("Ваши предметы:")
	for _, subjectName := range subjects {
		subject := s.findSubject(subjectName)
		if subject != nil {
			fmt.Printf("- %s (код: %s)\n", subjectName, subject.Code)
		}
	}

	fmt.Print("Введите название предмета или код (например: $100): ")
	scanner.Scan()
	identifier := scanner.Text()

	subject := s.findSubjectByNameOrCode(identifier)
	if subject == nil {
		fmt.Println("Предмет не найден! Используйте название или код.")
		return
	}

	assignments := subject.GetAssignments()
	if len(assignments) == 0 {
		fmt.Println("В этом предмете нет заданий.")
		return
	}

	fmt.Println("Доступные задания:")
	for _, assignment := range assignments {
		fmt.Printf("- %s\n", assignment)
	}

	fmt.Print("Введите название задания: ")
	scanner.Scan()
	assignmentName := scanner.Text()

	if s.SubmitAssignment(student.ID, subject.Name, assignmentName) {
		fmt.Printf("Задание '%s' успешно сдано на проверку!\n", assignmentName)
	} else {
		fmt.Println("Не удалось сдать задание.")
	}
}

func (s *UniversitySystem) signUpForReportMenu(scanner *bufio.Scanner, student *Student) {
	studentSubjects := s.GetStudentSubjects(student.ID)
	if len(studentSubjects) == 0 {
		fmt.Println("Вы не зачислены ни на один предмет.")
		return
	}

	fmt.Println("Список доступных докладов:")
	var availableReports []*Report
	counter := 1

	for _, report := range s.reports {
		// Проверяем, что доклад по предмету студента
		isStudentSubject := false
		for _, subject := range studentSubjects {
			if report.SubjectName == subject {
				isStudentSubject = true
				break
			}
		}

		if isStudentSubject && !report.IsFull() && !report.HasStudent(student.ID) {
			fmt.Printf("%d. %s (Предмет: %s, Участников: %d/%d)\n",
				counter, report.Topic, report.SubjectName,
				len(report.GetSignedUpStudents()), report.MaxParticipants)
			availableReports = append(availableReports, report)
			counter++
		}
	}

	if len(availableReports) == 0 {
		fmt.Println("Нет доступных докладов по вашим предметам.")
		return
	}

	fmt.Print("Выберите номер доклада: ")
	scanner.Scan()
	reportNumStr := scanner.Text()
	reportNum, _ := strconv.Atoi(reportNumStr)

	if reportNum > 0 && reportNum <= len(availableReports) {
		report := availableReports[reportNum-1]
		if report.AddStudent(student.ID) {
			fmt.Printf("Успешно записался на доклад: %s\n", report.Topic)
			s.SaveAllData()
		} else {
			fmt.Println("Не могу записаться на доклад")
		}
	} else {
		fmt.Println("Неверный номер доклада!")
	}
}

func (s *UniversitySystem) optOutOfReportMenu(scanner *bufio.Scanner, student *Student) {
	fmt.Print("Введите тему доклада: ")
	scanner.Scan()
	reportTopic := scanner.Text()

	report := s.findReport(reportTopic)
	if report == nil {
		fmt.Println("Доклад с такой темой не найден.")
		return
	}

	if report.RemoveStudent(student.ID) {
		fmt.Printf("Отписался от доклада: %s\n", reportTopic)
		s.SaveAllData()
	} else {
		fmt.Println("Вы не записаны на этот доклад.")
	}
}

func (s *UniversitySystem) showStudentGrades(student *Student) {
	fmt.Printf("\n=== ОЦЕНКИ СТУДЕНТА %s (ID: %d) ===\n", student.Name, student.ID)

	subjects := s.GetStudentSubjects(student.ID)
	if len(subjects) == 0 {
		fmt.Println("Студент не зачислен ни на один предмет.")
		return
	}

	overallTotal := 0.0
	overallCount := 0
	uniqueSubjects := make(map[string]bool)

	for _, subjectName := range subjects {
		uniqueSubjects[subjectName] = true
	}

	for subjectName := range uniqueSubjects {
		subject := s.findSubject(subjectName)
		if subject == nil {
			continue
		}

		fmt.Printf("\nПредмет: %s (код: %s)\n", subjectName, subject.Code)

		gradesSummary := subject.GetStudentGradesSummary(student.ID)
		if len(gradesSummary) == 0 {
			fmt.Println("  Нет оценок")
			continue
		}

		subjectTotal := 0.0
		subjectCount := 0

		fmt.Println("  Оценки:")
		for _, item := range gradesSummary {
			fmt.Printf("  - %s: %.2f\n", item.Item, item.Grade)
			subjectTotal += item.Grade
			subjectCount++
		}

		if subjectCount > 0 {
			subjectAverage := subjectTotal / float64(subjectCount)
			fmt.Printf("  Средний балл: %.2f\n", subjectAverage)
			fmt.Printf("  Суммарный балл: %.2f\n", subjectTotal)

			overallTotal += subjectTotal
			overallCount += subjectCount
		}
	}

	if overallCount > 0 && len(uniqueSubjects) > 0 {
		fmt.Println("\n=== ОБЩИЕ ИТОГИ ===")
		fmt.Printf("Всего предметов: %d\n", len(uniqueSubjects))
		fmt.Printf("Всего оценок: %d\n", overallCount)
		fmt.Printf("Средний балл: %.2f\n", overallTotal/float64(len(uniqueSubjects)))
	}
}

// Основные методы для преподавателей
func (s *UniversitySystem) RunProfessorMenu(scanner *bufio.Scanner, professor *Professor) {
	for {
		fmt.Println("\n=== МЕНЮ ПРЕПОДАВАТЕЛЯ ===")
		fmt.Println("1. Создать предмет")
		fmt.Println("2. Создать задание")
		fmt.Println("3. Создать доклад")
		fmt.Println("4. Записать студента на предмет")
		fmt.Println("5. Проверить сданные работы")
		fmt.Println("6. Выставить оценку за доклад")
		fmt.Println("7. Просмотреть статистику предмета")
		fmt.Println("8. Создать итоговый отчет")
		fmt.Println("9. Выход из системы")
		fmt.Print("Выберите действие: ")

		scanner.Scan()
		choiceStr := scanner.Text()
		choice, _ := strconv.Atoi(choiceStr)

		switch choice {
		case 1:
			s.createSubjectMenu(scanner, professor)
		case 2:
			s.createAssignmentMenu(scanner, professor)
		case 3:
			s.createReportMenu(scanner, professor)
		case 4:
			s.enrollStudentMenu(scanner, professor)
		case 5:
			s.checkSubmissionsMenu(scanner, professor)
		case 6:
			s.gradeReportMenu(scanner, professor)
		case 7:
			s.showSubjectStatsMenu(scanner, professor)
		case 8:
			s.createFinalReportMenu(scanner, professor)
		case 9:
			s.Logout()
			s.SaveAllData()
			return
		default:
			fmt.Println("Неверный выбор!")
		}
	}
}

func (s *UniversitySystem) createSubjectMenu(scanner *bufio.Scanner, professor *Professor) {
	fmt.Print("Название предмета: ")
	scanner.Scan()
	name := scanner.Text()

	fmt.Print("Код предмета: ")
	scanner.Scan()
	code := scanner.Text()

	existingSubject := s.findSubject(name)
	if existingSubject != nil {
		fmt.Printf("\nВнимание: предмет '%s' уже существует!\n", name)
		fmt.Printf("Текущий преподаватель: ID %d\n", existingSubject.ProfessorID)
		fmt.Println("Вы хотите стать преподавателем этого предмета?")
		fmt.Println("1. Да, заменить преподавателя")
		fmt.Println("2. Нет, отменить создание")
		fmt.Print("Выберите: ")

		scanner.Scan()
		choiceStr := scanner.Text()
		choice, _ := strconv.Atoi(choiceStr)

		if choice == 1 {
			// Сохраняем данные старого предмета
			enrolledStudents := existingSubject.GetEnrolledStudents()
			assignments := existingSubject.GetAssignments()
			reports := existingSubject.GetReports()

			// Создаем новый предмет с новым преподавателем
			newSubject := NewSubject(name, code, professor.ID)

			// Восстанавливаем студентов
			for _, studentID := range enrolledStudents {
				newSubject.EnrollStudent(studentID)
			}

			// Восстанавливаем задания
			for _, assignment := range assignments {
				newSubject.AddAssignment(assignment)
			}

			// Восстанавливаем доклады
			for _, report := range reports {
				newSubject.AddReport(report)
			}

			// Заменяем предмет
			for i, subject := range s.subjects {
				if subject.Name == name {
					s.subjects[i] = newSubject
					break
				}
			}

			fmt.Printf("Вы теперь преподаватель предмета '%s'\n", name)
			fmt.Printf("Сохранено: %d студентов, %d заданий, %d докладов\n",
				len(enrolledStudents), len(assignments), len(reports))
			s.SaveAllData()
		} else {
			fmt.Println("Создание предмета отменено.")
		}
	} else {
		subject := professor.CreateSubject(name, code, professor.ID)
		s.subjects = append(s.subjects, subject)
		fmt.Printf("Предмет '%s' создан успешно!\n", name)
		s.SaveAllData()
	}
}

func (s *UniversitySystem) createAssignmentMenu(scanner *bufio.Scanner, professor *Professor) {
	// Показываем предметы преподавателя
	hasSubjects := false
	fmt.Println("Ваши предметы:")
	for _, subject := range s.subjects {
		if subject.IsProfessor(professor.ID) {
			fmt.Printf("- %s (код: %s)\n", subject.Name, subject.Code)
			hasSubjects = true
		}
	}

	if !hasSubjects {
		fmt.Println("У вас нет предметов. Сначала создайте предмет.")
		return
	}

	fmt.Print("Введите название предмета или код: ")
	scanner.Scan()
	identifier := scanner.Text()

	subject := s.findSubjectByNameOrCode(identifier)
	if subject == nil || !subject.IsProfessor(professor.ID) {
		fmt.Println("Предмет не найден или вы не ведете его!")
		return
	}

	fmt.Print("Название задания: ")
	scanner.Scan()
	name := scanner.Text()

	fmt.Print("Максимальный балл: ")
	scanner.Scan()
	maxScoreStr := scanner.Text()
	maxScore, _ := strconv.ParseFloat(maxScoreStr, 64)

	assignment := professor.CreateAssignment(name, "", subject)
	assignment.MaxScore = maxScore
	s.assignments = append(s.assignments, assignment)

	fmt.Printf("Задание '%s' создано с максимальным баллом: %.1f\n", name, maxScore)
	s.SaveAllData()
}

func (s *UniversitySystem) createReportMenu(scanner *bufio.Scanner, professor *Professor) {
	// Показываем предметы преподавателя
	hasSubjects := false
	fmt.Println("Ваши предметы:")
	for _, subject := range s.subjects {
		if subject.IsProfessor(professor.ID) {
			fmt.Printf("- %s (код: %s)\n", subject.Name, subject.Code)
			hasSubjects = true
		}
	}

	if !hasSubjects {
		fmt.Println("У вас нет предметов. Сначала создайте предмет.")
		return
	}

	fmt.Print("Введите название предмета или код: ")
	scanner.Scan()
	identifier := scanner.Text()

	subject := s.findSubjectByNameOrCode(identifier)
	if subject == nil || !subject.IsProfessor(professor.ID) {
		fmt.Println("Предмет не найден или вы не ведете его!")
		return
	}

	fmt.Print("Тема доклада: ")
	scanner.Scan()
	topic := scanner.Text()

	fmt.Print("Макс. участников: ")
	scanner.Scan()
	maxPartStr := scanner.Text()
	maxParticipants, _ := strconv.Atoi(maxPartStr)

	report := professor.CreateReport(topic, subject, maxParticipants)
	s.reports = append(s.reports, report)
	s.SaveAllData()
}

func (s *UniversitySystem) enrollStudentMenu(scanner *bufio.Scanner, professor *Professor) {
	// Показываем предметы преподавателя
	hasSubjects := false
	fmt.Println("Ваши предметы:")
	for _, subject := range s.subjects {
		if subject.IsProfessor(professor.ID) {
			fmt.Printf("- %s (код: %s)\n", subject.Name, subject.Code)
			hasSubjects = true
		}
	}

	if !hasSubjects {
		fmt.Println("У вас нет предметов. Сначала создайте предмет.")
		return
	}

	// Показываем всех студентов
	s.listAllStudents()

	fmt.Print("Введите ID студента: ")
	scanner.Scan()
	studentIDStr := scanner.Text()
	studentID, _ := strconv.Atoi(studentIDStr)

	fmt.Print("Введите название предмета или код: ")
	scanner.Scan()
	identifier := scanner.Text()

	subject := s.findSubjectByNameOrCode(identifier)
	if subject == nil || !subject.IsProfessor(professor.ID) {
		fmt.Println("Предмет не найден или вы не ведете его!")
		return
	}

	s.EnrollStudentInSubject(studentID, identifier)
}

func (s *UniversitySystem) checkSubmissionsMenu(scanner *bufio.Scanner, professor *Professor) {
	var professorPending []SubmissionRecord

	// Находим работы на проверку по предметам преподавателя
	for _, sub := range s.submissions {
		if sub.Status == "pending" {
			subject := s.findSubject(sub.SubjectName)
			if subject != nil && subject.IsProfessor(professor.ID) {
				professorPending = append(professorPending, sub)
			}
		}
	}

	if len(professorPending) == 0 {
		fmt.Println("Нет работ на проверку по вашим предметам.")
		return
	}

	fmt.Printf("Работы на проверку по вашим предметам (%d):\n", len(professorPending))
	for i, sub := range professorPending {
		student := s.findStudentById(sub.StudentID)
		studentName := "Неизвестный"
		if student != nil {
			studentName = student.Name
		}

		maxScore := 100.0
		for _, assignment := range s.assignments {
			if assignment.Name == sub.AssignmentName && assignment.SubjectName == sub.SubjectName {
				maxScore = assignment.MaxScore
				break
			}
		}

		fmt.Printf("%d. Студент: %s (ID: %d), Предмет: %s, Задание: %s (макс. балл: %.1f) (отправлено: %s)\n",
			i+1, studentName, sub.StudentID, sub.SubjectName, sub.AssignmentName, maxScore, sub.Timestamp)
	}

	fmt.Print("\nВыберите работу для проверки (номер): ")
	scanner.Scan()
	workNumStr := scanner.Text()
	workNum, _ := strconv.Atoi(workNumStr)

	if workNum > 0 && workNum <= len(professorPending) {
		sub := professorPending[workNum-1]
		student := s.findStudentById(sub.StudentID)
		studentName := "Неизвестный"
		if student != nil {
			studentName = student.Name
		}

		fmt.Println("\nВы выбрали работу:")
		fmt.Printf("Студент: %s (ID: %d)\n", studentName, sub.StudentID)
		fmt.Printf("Предмет: %s\n", sub.SubjectName)
		fmt.Printf("Задание: %s\n", sub.AssignmentName)

		maxScore := 100.0
		for _, assignment := range s.assignments {
			if assignment.Name == sub.AssignmentName && assignment.SubjectName == sub.SubjectName {
				maxScore = assignment.MaxScore
				break
			}
		}
		fmt.Printf("Максимальный балл: %.1f\n", maxScore)

		fmt.Println("\n1. Утвердить и выставить оценку")
		fmt.Println("2. Отклонить")
		fmt.Print("Выберите действие: ")

		scanner.Scan()
		actionStr := scanner.Text()
		action, _ := strconv.Atoi(actionStr)

		if action == 1 {
			fmt.Printf("Введите оценку (0-%.1f): ", maxScore)
			scanner.Scan()
			gradeStr := scanner.Text()
			grade, _ := strconv.ParseFloat(gradeStr, 64)

			if s.GradeAssignment(sub.StudentID, sub.SubjectName, sub.AssignmentName, grade) {
				fmt.Println("Оценка выставлена успешно!")
			} else {
				fmt.Println("Не удалось выставить оценку.")
			}
		} else if action == 2 {
			// Отклоняем работу
			for i := range s.submissions {
				if s.submissions[i].StudentID == sub.StudentID &&
					s.submissions[i].SubjectName == sub.SubjectName &&
					s.submissions[i].AssignmentName == sub.AssignmentName &&
					s.submissions[i].Status == "pending" {
					s.submissions[i].Status = "rejected"
					fmt.Println("Работа отклонена. Студент может пересдать.")
					s.SaveAllData()
					break
				}
			}
		}
	} else {
		fmt.Println("Неверный номер работы!")
	}
}

func (s *UniversitySystem) gradeReportMenu(scanner *bufio.Scanner, professor *Professor) {
	var professorReports []*Report

	// Находим доклады по предметам преподавателя, которые еще не оценены
	for _, report := range s.reports {
		subject := s.findSubject(report.SubjectName)
		if subject != nil && subject.IsProfessor(professor.ID) && !report.IsCompleted {
			// Проверяем, есть ли уже оценки за этот доклад
			hasGrades := false
			for _, grade := range s.grades {
				if grade.Type == "report" && grade.ItemName == report.Topic {
					hasGrades = true
					break
				}
			}

			if !hasGrades {
				professorReports = append(professorReports, report)
			}
		}
	}

	if len(professorReports) == 0 {
		fmt.Println("Нет докладов для оценки по вашим предметам.")
		return
	}

	fmt.Println("Доступные для оценки доклады по вашим предметам:")
	for i, report := range professorReports {
		fmt.Printf("%d. %s (Предмет: %s, Участников: %d)\n",
			i+1, report.Topic, report.SubjectName, len(report.GetSignedUpStudents()))
	}

	fmt.Print("Выберите номер доклада: ")
	scanner.Scan()
	reportNumStr := scanner.Text()
	reportNum, _ := strconv.Atoi(reportNumStr)

	if reportNum > 0 && reportNum <= len(professorReports) {
		report := professorReports[reportNum-1]
		subjectName := report.SubjectName
		reportName := report.Topic
		participants := report.GetSignedUpStudents()

		if len(participants) == 0 {
			fmt.Println("На этот доклад не записан ни один студент.")
			return
		}

		fmt.Printf("Студенты, записанные на доклад '%s':\n", reportName)
		for _, studentID := range participants {
			student := s.findStudentById(studentID)
			if student != nil {
				fmt.Printf("- %s (ID: %d)\n", student.Name, studentID)
			}
		}

		fmt.Print("Введите оценку для всех участников (0-100): ")
		scanner.Scan()
		gradeStr := scanner.Text()
		grade, _ := strconv.ParseFloat(gradeStr, 64)

		if s.GradeReport(subjectName, reportName, grade) {
			fmt.Printf("Оценка %.2f успешно выставлена за доклад '%s'\n", grade, reportName)
		} else {
			fmt.Println("Не удалось выставить оценку.")
		}
	} else {
		fmt.Println("Неверный номер доклада!")
	}
}

func (s *UniversitySystem) showSubjectStatsMenu(scanner *bufio.Scanner, professor *Professor) {
	// Показываем предметы преподавателя
	hasSubjects := false
	fmt.Println("Ваши предметы:")
	for _, subject := range s.subjects {
		if subject.IsProfessor(professor.ID) {
			fmt.Printf("- %s (код: %s)\n", subject.Name, subject.Code)
			hasSubjects = true
		}
	}

	if !hasSubjects {
		fmt.Println("У вас нет предметов.")
		return
	}

	fmt.Print("Введите название предмета или код: ")
	scanner.Scan()
	identifier := scanner.Text()

	subject := s.findSubjectByNameOrCode(identifier)
	if subject == nil || !subject.IsProfessor(professor.ID) {
		fmt.Println("Предмет не найден или вы не ведете его!")
		return
	}

	s.showSubjectStatistics(subject.Name)
}

func (s *UniversitySystem) createFinalReportMenu(scanner *bufio.Scanner, professor *Professor) {
	// Показываем предметы преподавателя
	hasSubjects := false
	fmt.Println("Ваши предметы:")
	for _, subject := range s.subjects {
		if subject.IsProfessor(professor.ID) {
			fmt.Printf("- %s (код: %s)\n", subject.Name, subject.Code)
			hasSubjects = true
		}
	}

	if !hasSubjects {
		fmt.Println("У вас нет предметов.")
		return
	}

	fmt.Print("Введите название предмета или код: ")
	scanner.Scan()
	identifier := scanner.Text()

	subject := s.findSubjectByNameOrCode(identifier)
	if subject == nil || !subject.IsProfessor(professor.ID) {
		fmt.Println("Предмет не найден или вы не ведете его!")
		return
	}

	// Собираем имена студентов
	studentNames := make(map[int]string)
	for _, studentID := range subject.GetEnrolledStudents() {
		student := s.findStudentById(studentID)
		if student != nil {
			studentNames[studentID] = student.Name
		}
	}

	// Генерируем отчет
	subject.GenerateFinalReport(studentNames)
}

// Основные системные методы
func (s *UniversitySystem) SubmitAssignment(studentID int, subjectName, assignmentName string) bool {
	subject := s.findSubject(subjectName)
	if subject == nil || !subject.IsStudentEnrolled(studentID) {
		fmt.Println("Ошибка: студент не зачислен на предмет или предмет не найден")
		return false
	}

	if !subject.HasAssignment(assignmentName) {
		fmt.Printf("Ошибка: задание '%s' не существует в предмете %s\n", assignmentName, subjectName)
		return false
	}

	// Проверяем, можно ли пересдать
	canResubmit := false
	for _, sub := range s.submissions {
		if sub.StudentID == studentID &&
			sub.SubjectName == subjectName &&
			sub.AssignmentName == assignmentName &&
			sub.Type == "assignment" {

			if sub.Status == "pending" {
				fmt.Println("Ошибка: вы уже отправили это задание и оно ожидает проверки")
				return false
			} else if sub.Status == "rejected" {
				canResubmit = true
			} else if sub.Status == "approved" {
				if subject.GetStudentAssignmentGrade(studentID, assignmentName) >= 0 {
					fmt.Println("Ошибка: за это задание уже выставлена оценка")
					return false
				}
			}
		}
	}

	if subject.GetStudentAssignmentGrade(studentID, assignmentName) >= 0 {
		fmt.Println("Ошибка: за это задание уже выставлена оценка")
		return false
	}

	if canResubmit {
		for i := range s.submissions {
			if s.submissions[i].StudentID == studentID &&
				s.submissions[i].SubjectName == subjectName &&
				s.submissions[i].AssignmentName == assignmentName &&
				s.submissions[i].Type == "assignment" &&
				s.submissions[i].Status == "rejected" {
				s.submissions[i].Status = "pending"
				s.submissions[i].Timestamp = s.dataManager.GetCurrentTimestamp()
				s.SaveAllData()
				return true
			}
		}
	}

	// Создаем новую запись о сдаче
	submission := SubmissionRecord{
		StudentID:      studentID,
		SubjectName:    subjectName,
		AssignmentName: assignmentName,
		Type:           "assignment",
		Status:         "pending",
		Timestamp:      s.dataManager.GetCurrentTimestamp(),
	}

	s.submissions = append(s.submissions, submission)
	s.SaveAllData()
	return true
}

func (s *UniversitySystem) GradeAssignment(studentID int, subjectName, assignmentName string, grade float64) bool {
	subject := s.findSubject(subjectName)
	if subject == nil || !subject.IsStudentEnrolled(studentID) {
		fmt.Println("Ошибка: студент не найден или не зачислен на предмет")
		return false
	}

	if !subject.HasAssignment(assignmentName) {
		fmt.Printf("Ошибка: задание '%s' не существует\n", assignmentName)
		return false
	}

	// Находим максимальный балл
	maxScore := 100.0
	for _, assignment := range s.assignments {
		if assignment.Name == assignmentName && assignment.SubjectName == subjectName {
			maxScore = assignment.MaxScore
			break
		}
	}

	if grade < 0 || grade > maxScore {
		fmt.Printf("Ошибка: оценка должна быть от 0 до %.1f\n", maxScore)
		return false
	}

	if subject.GetStudentAssignmentGrade(studentID, assignmentName) >= 0 {
		fmt.Println("Ошибка: оценка за это задание уже выставлена")
		return false
	}

	// Обновляем статус отправки
	found := false
	for i := range s.submissions {
		if s.submissions[i].StudentID == studentID &&
			s.submissions[i].SubjectName == subjectName &&
			s.submissions[i].AssignmentName == assignmentName &&
			s.submissions[i].Type == "assignment" {
			s.submissions[i].Status = "approved"
			found = true
			break
		}
	}

	if !found {
		submission := SubmissionRecord{
			StudentID:      studentID,
			SubjectName:    subjectName,
			AssignmentName: assignmentName,
			Type:           "assignment",
			Status:         "approved",
			Timestamp:      s.dataManager.GetCurrentTimestamp(),
		}
		s.submissions = append(s.submissions, submission)
	}

	// Выставляем оценку
	subject.GradeAssignment(studentID, assignmentName, grade)

	// Добавляем запись об оценке
	gradeRecord := GradeRecord{
		StudentID:   studentID,
		SubjectName: subjectName,
		ItemName:    assignmentName,
		Type:        "assignment",
		Score:       grade,
		Timestamp:   s.dataManager.GetCurrentTimestamp(),
	}
	s.grades = append(s.grades, gradeRecord)

	fmt.Printf("Оценка %.2f успешно выставлена за задание '%s' (макс. балл: %.1f)\n", grade, assignmentName, maxScore)

	// Уведомляем студента
	student := s.findStudentById(studentID)
	if student != nil {
		student.OnGradeUpdated(subjectName, assignmentName, grade)
	}

	s.SaveAllData()
	return true
}

func (s *UniversitySystem) GradeReport(subjectName, reportName string, grade float64) bool {
	subject := s.findSubject(subjectName)
	if subject == nil {
		fmt.Println("Ошибка: предмет не найден")
		return false
	}

	// Проверяем, существует ли такой доклад
	if !subject.HasReport(reportName) {
		fmt.Printf("Ошибка: доклад '%s' не существует\n", reportName)
		return false
	}

	// Проверяем, не выставлены ли уже оценки за этот доклад
	hasGrades := false
	for _, g := range s.grades {
		if g.Type == "report" && g.ItemName == reportName && g.SubjectName == subjectName {
			hasGrades = true
			break
		}
	}

	if hasGrades {
		fmt.Println("Ошибка: оценки за этот доклад уже выставлены")
		return false
	}

	// Проверяем диапазон оценки
	if grade < 0 || grade > 100 {
		fmt.Println("Ошибка: оценка должна быть от 0 до 100")
		return false
	}

	report := s.findReportForSubject(subjectName, reportName)
	if report == nil {
		fmt.Println("Ошибка: информация о докладе не найдена")
		return false
	}

	// Получаем участников доклада
	participants := report.GetSignedUpStudents()

	if len(participants) == 0 {
		fmt.Println("Ошибка: на этот доклад не записан ни один студент")
		return false
	}

	// Выставляем оценки всем участникам
	count := 0
	for _, studentID := range participants {
		// Проверяем, что студент зачислен на предмет
		if subject.IsStudentEnrolled(studentID) {
			subject.GradeReport(studentID, reportName, grade)

			// Добавляем запись об оценке
			gradeRecord := GradeRecord{
				StudentID:   studentID,
				SubjectName: subjectName,
				ItemName:    reportName,
				Type:        "report",
				Score:       grade,
				Timestamp:   s.dataManager.GetCurrentTimestamp(),
			}
			s.grades = append(s.grades, gradeRecord)
			count++

			// Уведомляем студента
			student := s.findStudentById(studentID)
			if student != nil {
				student.OnGradeUpdated(subjectName, reportName, grade)
			}
		}
	}

	fmt.Printf("Оценка %.2f выставлена %d студентам за доклад '%s'\n", grade, count, reportName)

	// Отмечаем доклад как завершенный
	report.MarkAsCompleted()

	// Удаляем доклад из списка активных
	s.removeReport(subjectName, reportName)
	fmt.Printf("Доклад '%s' удален из активных\n", reportName)

	s.SaveAllData()
	return true
}

func (s *UniversitySystem) EnrollStudentInSubject(studentID int, identifier string) {
	subject := s.findSubjectByNameOrCode(identifier)
	if subject == nil {
		fmt.Println("Предмет не найден! Используйте название или код (например: $100)")
		return
	}

	if s.isStudentAlreadyEnrolled(studentID, subject.Name) {
		fmt.Printf("Студент ID %d уже зачислен на предмет %s\n", studentID, subject.Name)
		return
	}

	subject.EnrollStudent(studentID)
	s.studentEnrollments[studentID] = append(s.studentEnrollments[studentID], subject.Name)
	s.subjectEnrollments[subject.Name] = append(s.subjectEnrollments[subject.Name], studentID)

	fmt.Printf("Студент ID %d зачислен на предмет %s\n", studentID, subject.Name)
	s.SaveAllData()
}

func (s *UniversitySystem) GetStudentSubjects(studentID int) []string {
	subjects, exists := s.studentEnrollments[studentID]
	if !exists {
		return []string{}
	}
	return subjects
}

// Вспомогательные методы
func (s *UniversitySystem) findSubject(name string) *Subject {
	for _, subject := range s.subjects {
		if subject.Name == name {
			return subject
		}
	}
	return nil
}

func (s *UniversitySystem) findSubjectByNameOrCode(identifier string) *Subject {
	if strings.HasPrefix(identifier, "$") {
		code := identifier[1:]
		for _, subject := range s.subjects {
			if subject.Code == code {
				return subject
			}
		}
	}
	return s.findSubject(identifier)
}

func (s *UniversitySystem) findReport(topic string) *Report {
	for _, report := range s.reports {
		if report.Topic == topic {
			return report
		}
	}
	return nil
}

func (s *UniversitySystem) findReportForSubject(subjectName, reportName string) *Report {
	for _, report := range s.reports {
		if report.Topic == reportName && report.SubjectName == subjectName {
			return report
		}
	}
	return nil
}

func (s *UniversitySystem) findStudentById(id int) *Student {
	student, exists := s.students[id]
	if !exists {
		return nil
	}
	return student
}

func (s *UniversitySystem) isStudentAlreadyEnrolled(studentID int, subjectName string) bool {
	subjects, exists := s.studentEnrollments[studentID]
	if !exists {
		return false
	}

	for _, subject := range subjects {
		if subject == subjectName {
			return true
		}
	}
	return false
}

func (s *UniversitySystem) removeReport(subjectName, reportName string) {
	var newReports []*Report
	for _, report := range s.reports {
		if !(report.Topic == reportName && report.SubjectName == subjectName) {
			newReports = append(newReports, report)
		}
	}
	s.reports = newReports

	subject := s.findSubject(subjectName)
	if subject != nil {
		subject.RemoveReport(reportName)
	}
}

func (s *UniversitySystem) showSubjectStatistics(subjectName string) {
	subject := s.findSubject(subjectName)
	if subject == nil {
		fmt.Println("Предмет не найден!")
		return
	}

	fmt.Printf("\n=== Статистика по предмету %s ===\n", subjectName)

	enrolled := subject.GetEnrolledStudents()
	fmt.Printf("Всего студентов: %d\n", len(enrolled))

	submitted := 0
	for _, sub := range s.submissions {
		if sub.SubjectName == subjectName && sub.Type == "assignment" {
			submitted++
		}
	}

	pendingCount := 0
	for _, sub := range s.submissions {
		if sub.SubjectName == subjectName && sub.Status == "pending" {
			pendingCount++
		}
	}

	fmt.Printf("Заданий сдано: %d\n", submitted)
	fmt.Printf("Заданий на проверке: %d\n", pendingCount)

	total := 0.0
	count := 0
	for _, grade := range s.grades {
		if grade.SubjectName == subjectName {
			total += grade.Score
			count++
		}
	}

	if count > 0 {
		fmt.Printf("Средняя оценка: %.2f\n", total/float64(count))
		fmt.Printf("Суммарная оценка: %.2f\n", total)
	}
}

func (s *UniversitySystem) listAllStudents() {
	fmt.Printf("\nВсе студенты (%d):\n", len(s.students))
	for id, student := range s.students {
		fmt.Printf("- %s (ID: %d)\n", student.Name, id)
	}
}
