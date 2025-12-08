package main

import (
	"fmt"
	"sort"
	"time"
)

type Subject struct {
	Name               string
	Code               string
	ProfessorID        int
	EnrolledStudentIDs map[int]bool
	AssignmentGrades   map[int]map[string]float64
	ReportGrades       map[int]map[string]float64
	Assignments        []string
	Reports            []string
}

func NewSubject(name, code string, professorID int) *Subject {
	return &Subject{
		Name:               name,
		Code:               code,
		ProfessorID:        professorID,
		EnrolledStudentIDs: make(map[int]bool),
		AssignmentGrades:   make(map[int]map[string]float64),
		ReportGrades:       make(map[int]map[string]float64),
		Assignments:        make([]string, 0),
		Reports:            make([]string, 0),
	}
}

func (s *Subject) EnrollStudent(studentID int) {
	s.EnrolledStudentIDs[studentID] = true
}

func (s *Subject) IsStudentEnrolled(studentID int) bool {
	return s.EnrolledStudentIDs[studentID]
}

func (s *Subject) AddAssignment(assignmentName string) {
	s.Assignments = append(s.Assignments, assignmentName)
}

func (s *Subject) AddReport(reportName string) {
	s.Reports = append(s.Reports, reportName)
}

func (s *Subject) RemoveReport(reportName string) bool {
	for i, r := range s.Reports {
		if r == reportName {
			s.Reports = append(s.Reports[:i], s.Reports[i+1:]...)
			return true
		}
	}
	return false
}

func (s *Subject) GradeAssignment(studentID int, assignmentName string, grade float64) {
	if s.IsStudentEnrolled(studentID) && s.HasAssignment(assignmentName) {
		if s.AssignmentGrades[studentID] == nil {
			s.AssignmentGrades[studentID] = make(map[string]float64)
		}
		s.AssignmentGrades[studentID][assignmentName] = grade
	}
}

func (s *Subject) GradeReport(studentID int, reportName string, grade float64) {
	if s.IsStudentEnrolled(studentID) {
		if s.ReportGrades[studentID] == nil {
			s.ReportGrades[studentID] = make(map[string]float64)
		}
		s.ReportGrades[studentID][reportName] = grade
	}
}

func (s *Subject) GradeAllReports(reportName string, grade float64, participants []int) {
	if !s.HasReport(reportName) {
		return
	}

	if len(participants) == 0 {
		for studentID := range s.EnrolledStudentIDs {
			if s.ReportGrades[studentID] == nil {
				s.ReportGrades[studentID] = make(map[string]float64)
			}
			s.ReportGrades[studentID][reportName] = grade
		}
	} else {
		for _, studentID := range participants {
			if s.IsStudentEnrolled(studentID) {
				if s.ReportGrades[studentID] == nil {
					s.ReportGrades[studentID] = make(map[string]float64)
				}
				s.ReportGrades[studentID][reportName] = grade
			}
		}
	}
}

func (s *Subject) GetStudentAssignmentGrade(studentID int, assignmentName string) float64 {
	if grades, ok := s.AssignmentGrades[studentID]; ok {
		if grade, ok := grades[assignmentName]; ok {
			return grade
		}
	}
	return -1.0
}

func (s *Subject) GetStudentReportGrade(studentID int, reportName string) float64 {
	if grades, ok := s.ReportGrades[studentID]; ok {
		if grade, ok := grades[reportName]; ok {
			return grade
		}
	}
	return -1.0
}

func (s *Subject) GetEnrolledStudents() []int {
	students := make([]int, 0, len(s.EnrolledStudentIDs))
	for id := range s.EnrolledStudentIDs {
		students = append(students, id)
	}
	sort.Ints(students)
	return students
}

func (s *Subject) IsProfessor(professorID int) bool {
	return s.ProfessorID == professorID
}

func (s *Subject) HasAssignment(assignmentName string) bool {
	for _, a := range s.Assignments {
		if a == assignmentName {
			return true
		}
	}
	return false
}

func (s *Subject) HasReport(reportName string) bool {
	for _, r := range s.Reports {
		if r == reportName {
			return true
		}
	}
	return false
}

func (s *Subject) GetAssignments() []string {
	return s.Assignments
}

func (s *Subject) GetReports() []string {
	return s.Reports
}

func (s *Subject) GenerateFinalReport(studentNames map[int]string) {
	fmt.Printf("\n=== ИТОГОВЫЙ ОТЧЕТ: %s (%s) ===\n", s.Name, s.Code)
	fmt.Printf("ID преподавателя: %d\n", s.ProfessorID)
	fmt.Printf("Зачисленных студентов: %d\n", len(s.EnrolledStudentIDs))
	fmt.Println("==============================================")

	students := s.GetEnrolledStudents()
	for _, studentID := range students {
		studentName := studentNames[studentID]
		if studentName == "" {
			studentName = "Неизвестный"
		}

		fmt.Printf("\nСтудент: %s (ID: %d)\n", studentName, studentID)

		total := 0.0
		count := 0

		if assignmentGrades, ok := s.AssignmentGrades[studentID]; ok {
			fmt.Println("Оценки за задания:")
			// Сортируем названия заданий для красивого вывода
			var assignments []string
			for assignment := range assignmentGrades {
				assignments = append(assignments, assignment)
			}
			sort.Strings(assignments)

			for _, assignment := range assignments {
				grade := assignmentGrades[assignment]
				fmt.Printf("  %-20s: %6.2f\n", assignment, grade)
				total += grade
				count++
			}
		}

		if reportGrades, ok := s.ReportGrades[studentID]; ok {
			fmt.Println("Оценки за доклады:")
			// Сортируем названия докладов
			var reports []string
			for report := range reportGrades {
				reports = append(reports, report)
			}
			sort.Strings(reports)

			for _, report := range reports {
				grade := reportGrades[report]
				fmt.Printf("  %-20s: %6.2f\n", report, grade)
				total += grade
				count++
			}
		}

		if count > 0 {
			fmt.Printf("  Среднее: %.2f\n", total/float64(count))
			fmt.Printf("  Суммарное: %.2f\n", total)
		} else {
			fmt.Println("  Нет оценок")
		}
	}
	fmt.Println()
}

func (s *Subject) SetAssignmentGrades(grades map[int]map[string]float64) {
	s.AssignmentGrades = grades
}

// SetReportGrades устанавливает оценки за доклады
func (s *Subject) SetReportGrades(grades map[int]map[string]float64) {
	s.ReportGrades = grades
}

// GetAllAssignmentGrades возвращает все оценки за задания
func (s *Subject) GetAllAssignmentGrades() map[int]map[string]float64 {
	return s.AssignmentGrades
}

// GetAllReportGrades возвращает все оценки за доклады
func (s *Subject) GetAllReportGrades() map[int]map[string]float64 {
	return s.ReportGrades
}

// GetEnrolledStudentIds возвращает ID всех зачисленных студентов
func (s *Subject) GetEnrolledStudentIds() []int {
	students := make([]int, 0, len(s.EnrolledStudentIDs))
	for id := range s.EnrolledStudentIDs {
		students = append(students, id)
	}
	sort.Ints(students)
	return students
}

// GetAssignmentList возвращает список всех заданий
func (s *Subject) GetAssignmentList() []string {
	return s.Assignments
}

// GetReportList возвращает список всех докладов
func (s *Subject) GetReportList() []string {
	return s.Reports
}

func (s *Subject) GetStudentGradesSummary(studentID int) []struct {
	Item  string
	Grade float64
} {
	var result []struct {
		Item  string
		Grade float64
	}

	if assignmentGrades, ok := s.AssignmentGrades[studentID]; ok {
		for assignment, grade := range assignmentGrades {
			result = append(result, struct {
				Item  string
				Grade float64
			}{
				Item:  "Задание: " + assignment,
				Grade: grade,
			})
		}
	}

	if reportGrades, ok := s.ReportGrades[studentID]; ok {
		for report, grade := range reportGrades {
			result = append(result, struct {
				Item  string
				Grade float64
			}{
				Item:  "Доклад: " + report,
				Grade: grade,
			})
		}
	}

	return result
}

type Assignment struct {
	Name        string
	SubjectName string
	MaxScore    float64
}

func NewAssignment(name, subjectName string, maxScore float64) *Assignment {
	return &Assignment{
		Name:        name,
		SubjectName: subjectName,
		MaxScore:    maxScore,
	}
}

type Report struct {
	Topic              string
	SubjectName        string
	SignedUpStudentIDs map[int]bool
	Date               time.Time
	MaxParticipants    int
	IsCompleted        bool
}

func NewReport(topic, subjectName string, maxParticipants int) *Report {
	return &Report{
		Topic:              topic,
		SubjectName:        subjectName,
		SignedUpStudentIDs: make(map[int]bool),
		Date:               time.Now(),
		MaxParticipants:    maxParticipants,
		IsCompleted:        false,
	}
}

func (r *Report) AddStudent(studentID int) bool {
	if r.IsFull() || r.IsCompleted {
		return false
	}

	if !r.SignedUpStudentIDs[studentID] {
		r.SignedUpStudentIDs[studentID] = true
		return true
	}
	return false
}

func (r *Report) RemoveStudent(studentID int) bool {
	if r.SignedUpStudentIDs[studentID] {
		delete(r.SignedUpStudentIDs, studentID)
		return true
	}
	return false
}

func (r *Report) IsFull() bool {
	return len(r.SignedUpStudentIDs) >= r.MaxParticipants
}

func (r *Report) HasStudent(studentID int) bool {
	return r.SignedUpStudentIDs[studentID]
}

func (r *Report) GetSignedUpStudents() []int {
	students := make([]int, 0, len(r.SignedUpStudentIDs))
	for id := range r.SignedUpStudentIDs {
		students = append(students, id)
	}
	sort.Ints(students)
	return students
}

func (r *Report) MarkAsCompleted() {
	r.IsCompleted = true
}
