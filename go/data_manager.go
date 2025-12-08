package main

import (
	"bufio"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

const DATA_DIR = "data"

type DataSubmission struct {
	StudentID      int
	SubjectName    string
	AssignmentName string
	Status         string
	Timestamp      string
}

type DataGrade struct {
	StudentID      int
	SubjectName    string
	AssignmentName string
	Score          float64
	Type           string
	Timestamp      string
}

type DataSubjectGrades struct {
	SubjectName      string
	AssignmentGrades map[int]map[string]float64
	ReportGrades     map[int]map[string]float64
}

type DataManager struct{}

func (dm *DataManager) InitDataDirectory() {
	os.MkdirAll(DATA_DIR, 0755)
}

func (dm *DataManager) GetCurrentTimestamp() string {
	return time.Now().Format("2006-01-02 15:04:05")
}

func (dm *DataManager) SaveNextUserId(nextId int) error {
	file, err := os.Create(filepath.Join(DATA_DIR, "next_id.txt"))
	if err != nil {
		return err
	}
	defer file.Close()

	file.WriteString(fmt.Sprintf("%d", nextId))
	return nil
}

func (dm *DataManager) LoadNextUserId() int {
	file, err := os.Open(filepath.Join(DATA_DIR, "next_id.txt"))
	if err != nil {
		return 1
	}
	defer file.Close()

	var nextId int
	fmt.Fscanf(file, "%d", &nextId)
	if nextId == 0 {
		return 1
	}
	return nextId
}

func (dm *DataManager) SaveUsers(users map[string]*User) error {
	file, err := os.Create(filepath.Join(DATA_DIR, "users.txt"))
	if err != nil {
		return err
	}
	defer file.Close()

	for _, user := range users {
		file.WriteString(fmt.Sprintf("%d,%s,%s,%d\n",
			user.ID, user.Name, user.PasswordHash, user.Role))
	}
	return nil
}

func (dm *DataManager) LoadUsers() map[string]*User {
	users := make(map[string]*User)

	file, err := os.Open(filepath.Join(DATA_DIR, "users.txt"))
	if err != nil {
		return users
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		parts := strings.Split(line, ",")
		if len(parts) < 4 {
			continue
		}

		id, _ := strconv.Atoi(parts[0])
		name := parts[1]
		passwordHash := parts[2]
		roleInt, _ := strconv.Atoi(parts[3])
		role := Role(roleInt)

		user := LoadUser(name, passwordHash, id, role)
		users[name] = user
	}

	return users
}

func (dm *DataManager) SaveSubjects(subjects []*Subject) error {
	file, err := os.Create(filepath.Join(DATA_DIR, "subjects.txt"))
	if err != nil {
		return err
	}
	defer file.Close()

	for _, subject := range subjects {
		file.WriteString(fmt.Sprintf("%s,%s,%d\n",
			subject.Name, subject.Code, subject.ProfessorID))
	}
	return nil
}

func (dm *DataManager) LoadSubjects() []*Subject {
	var subjects []*Subject

	file, err := os.Open(filepath.Join(DATA_DIR, "subjects.txt"))
	if err != nil {
		return subjects
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		parts := strings.Split(line, ",")
		if len(parts) < 3 {
			continue
		}

		name := parts[0]
		code := parts[1]
		profID, _ := strconv.Atoi(parts[2])

		subject := NewSubject(name, code, profID)
		subjects = append(subjects, subject)
	}

	return subjects
}

func (dm *DataManager) SaveEnrollments(enrollments map[int][]string) error {
	file, err := os.Create(filepath.Join(DATA_DIR, "enrollments.txt"))
	if err != nil {
		return err
	}
	defer file.Close()

	for studentID, subjects := range enrollments {
		line := fmt.Sprintf("%d", studentID)
		for _, subject := range subjects {
			line += "," + subject
		}
		file.WriteString(line + "\n")
	}
	return nil
}

func (dm *DataManager) LoadEnrollments() map[int][]string {
	enrollments := make(map[int][]string)

	file, err := os.Open(filepath.Join(DATA_DIR, "enrollments.txt"))
	if err != nil {
		return enrollments
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		parts := strings.Split(line, ",")
		if len(parts) < 1 {
			continue
		}

		studentID, _ := strconv.Atoi(parts[0])
		var subjects []string
		for i := 1; i < len(parts); i++ {
			subjects = append(subjects, parts[i])
		}

		enrollments[studentID] = subjects
	}

	return enrollments
}

func (dm *DataManager) SaveSubjectGrades(subjects []*Subject) error {
	file, err := os.Create(filepath.Join(DATA_DIR, "subject_grades.txt"))
	if err != nil {
		return err
	}
	defer file.Close()

	for _, subject := range subjects {
		subjectName := subject.Name
		if subjectName == "" {
			continue
		}

		// Сохраняем оценки за задания
		for studentID, assignmentGrades := range subject.AssignmentGrades {
			for assignmentName, grade := range assignmentGrades {
				file.WriteString(fmt.Sprintf("ASSIGNMENT,%s,%d,%s,%.2f\n",
					subjectName, studentID, assignmentName, grade))
			}
		}

		// Сохраняем оценки за доклады
		for studentID, reportGrades := range subject.ReportGrades {
			for reportName, grade := range reportGrades {
				file.WriteString(fmt.Sprintf("REPORT,%s,%d,%s,%.2f\n",
					subjectName, studentID, reportName, grade))
			}
		}
	}
	return nil
}

func (dm *DataManager) LoadSubjectGrades() map[string]*DataSubjectGrades {
	subjectGradesMap := make(map[string]*DataSubjectGrades)

	file, err := os.Open(filepath.Join(DATA_DIR, "subject_grades.txt"))
	if err != nil {
		return subjectGradesMap
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		parts := strings.Split(line, ",")
		if len(parts) < 5 {
			continue
		}

		typeStr := parts[0]
		subjectName := parts[1]
		studentID, _ := strconv.Atoi(parts[2])
		itemName := parts[3]
		grade, _ := strconv.ParseFloat(parts[4], 64)

		// Создаем запись для предмета, если ее нет
		if subjectGradesMap[subjectName] == nil {
			subjectGradesMap[subjectName] = &DataSubjectGrades{
				SubjectName:      subjectName,
				AssignmentGrades: make(map[int]map[string]float64),
				ReportGrades:     make(map[int]map[string]float64),
			}
		}

		if typeStr == "ASSIGNMENT" {
			if subjectGradesMap[subjectName].AssignmentGrades[studentID] == nil {
				subjectGradesMap[subjectName].AssignmentGrades[studentID] = make(map[string]float64)
			}
			subjectGradesMap[subjectName].AssignmentGrades[studentID][itemName] = grade
		} else if typeStr == "REPORT" {
			if subjectGradesMap[subjectName].ReportGrades[studentID] == nil {
				subjectGradesMap[subjectName].ReportGrades[studentID] = make(map[string]float64)
			}
			subjectGradesMap[subjectName].ReportGrades[studentID][itemName] = grade
		}
	}

	return subjectGradesMap
}

func (dm *DataManager) LoadSubmissions() []SubmissionRecord {
	var submissions []SubmissionRecord

	file, err := os.Open(filepath.Join(DATA_DIR, "submissions.txt"))
	if err != nil {
		return submissions
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		parts := strings.Split(line, ",")
		if len(parts) < 6 {
			continue
		}

		studentID, _ := strconv.Atoi(parts[0])
		submission := SubmissionRecord{
			StudentID:      studentID,
			SubjectName:    parts[1],
			AssignmentName: parts[2],
			Type:           parts[3],
			Status:         parts[4],
			Timestamp:      parts[5],
		}
		submissions = append(submissions, submission)
	}

	return submissions
}

func (dm *DataManager) LoadGrades() []GradeRecord {
	var grades []GradeRecord

	file, err := os.Open(filepath.Join(DATA_DIR, "grades.txt"))
	if err != nil {
		return grades
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		parts := strings.Split(line, ",")
		if len(parts) < 6 {
			continue
		}

		studentID, _ := strconv.Atoi(parts[0])
		score, _ := strconv.ParseFloat(parts[4], 64)

		grade := GradeRecord{
			StudentID:   studentID,
			SubjectName: parts[1],
			ItemName:    parts[2],
			Type:        parts[3],
			Score:       score,
			Timestamp:   parts[5],
		}
		grades = append(grades, grade)
	}

	return grades
}

func (dm *DataManager) SaveAssignments(assignments []*Assignment) error {
	file, err := os.Create(filepath.Join(DATA_DIR, "assignments.txt"))
	if err != nil {
		return err
	}
	defer file.Close()

	for _, assignment := range assignments {
		file.WriteString(fmt.Sprintf("%s,,%.2f,%s\n",
			assignment.Name, assignment.MaxScore, assignment.SubjectName))
	}
	return nil
}

func (dm *DataManager) LoadAssignments() []*Assignment {
	var assignments []*Assignment

	file, err := os.Open(filepath.Join(DATA_DIR, "assignments.txt"))
	if err != nil {
		return assignments
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		parts := strings.Split(line, ",")
		if len(parts) < 4 {
			continue
		}

		name := parts[0]
		maxScore, _ := strconv.ParseFloat(parts[2], 64)
		subjectName := parts[3]

		assignment := NewAssignment(name, subjectName, maxScore)
		assignments = append(assignments, assignment)
	}

	return assignments
}

func (dm *DataManager) SaveReports(reports []*Report) error {
	file, err := os.Create(filepath.Join(DATA_DIR, "reports.txt"))
	if err != nil {
		return err
	}
	defer file.Close()

	for _, report := range reports {
		completed := "0"
		if report.IsCompleted {
			completed = "1"
		}

		line := fmt.Sprintf("%s,%s,%d,%s",
			report.Topic, report.SubjectName, report.MaxParticipants, completed)

		students := report.GetSignedUpStudents()
		for _, studentID := range students {
			line += fmt.Sprintf(",%d", studentID)
		}

		file.WriteString(line + "\n")
	}
	return nil
}

func (dm *DataManager) LoadReports() []*Report {
	var reports []*Report

	file, err := os.Open(filepath.Join(DATA_DIR, "reports.txt"))
	if err != nil {
		return reports
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		parts := strings.Split(line, ",")
		if len(parts) < 4 {
			continue
		}

		topic := parts[0]
		subjectName := parts[1]
		maxParticipants, _ := strconv.Atoi(parts[2])
		completedStr := parts[3]

		report := NewReport(topic, subjectName, maxParticipants)

		if completedStr == "1" {
			report.MarkAsCompleted()
		}

		// Загружаем студентов
		for i := 4; i < len(parts); i++ {
			if parts[i] != "" {
				studentID, _ := strconv.Atoi(parts[i])
				report.AddStudent(studentID)
			}
		}

		reports = append(reports, report)
	}

	return reports
}

func (dm *DataManager) SaveSubmissions(submissions []SubmissionRecord) error {
	file, err := os.Create(filepath.Join(DATA_DIR, "submissions.txt"))
	if err != nil {
		return err
	}
	defer file.Close()

	for _, submission := range submissions {
		file.WriteString(fmt.Sprintf("%d,%s,%s,%s,%s,%s\n",
			submission.StudentID, submission.SubjectName, submission.AssignmentName,
			submission.Type, submission.Status, submission.Timestamp))
	}
	return nil
}

func (dm *DataManager) SaveGrades(grades []GradeRecord) error {
	file, err := os.Create(filepath.Join(DATA_DIR, "grades.txt"))
	if err != nil {
		return err
	}
	defer file.Close()

	for _, grade := range grades {
		file.WriteString(fmt.Sprintf("%d,%s,%s,%s,%.2f,%s\n",
			grade.StudentID, grade.SubjectName, grade.ItemName,
			grade.Type, grade.Score, grade.Timestamp))
	}
	return nil
}

func (dm *DataManager) SaveAllData(system *UniversitySystem) error {
	dm.InitDataDirectory()

	// Сохраняем пользователей
	users := make(map[string]*User)
	for name, user := range system.users {
		users[name] = user
	}
	dm.SaveUsers(users)

	// Сохраняем предметы
	dm.SaveSubjects(system.subjects)

	// Сохраняем задания
	dm.SaveAssignments(system.assignments)

	// Сохраняем доклады
	dm.SaveReports(system.reports)

	// Сохраняем записи на предметы
	dm.SaveEnrollments(system.studentEnrollments)

	// Сохраняем отправки
	dm.SaveSubmissions(system.submissions)

	// Сохраняем оценки
	dm.SaveGrades(system.grades)

	// Сохраняем оценки в предметах
	dm.SaveSubjectGrades(system.subjects)

	// Сохраняем следующий ID
	dm.SaveNextUserId(GetNextId())

	return nil
}
