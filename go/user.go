package main

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"os"
)

type Role int

const (
	RoleStudent Role = iota
	RoleProfessor
)

var nextUserId = 1

type User struct {
	Name         string
	PasswordHash string
	ID           int
	Role         Role
}

func NewUser(name, password string, role Role) *User {
	user := &User{
		Name:         name,
		PasswordHash: HashPassword(password),
		ID:           nextUserId,
		Role:         role,
	}
	nextUserId++
	return user
}

func LoadUser(name, passwordHash string, id int, role Role) *User {
	if id >= nextUserId {
		nextUserId = id + 1
	}
	return &User{
		Name:         name,
		PasswordHash: passwordHash,
		ID:           id,
		Role:         role,
	}
}

func (u *User) CheckPassword(password string) bool {
	return u.PasswordHash == HashPassword(password)
}

func (u *User) GetRoleString() string {
	switch u.Role {
	case RoleStudent:
		return "Студент"
	case RoleProfessor:
		return "Преподаватель"
	default:
		return "Неизвестно"
	}
}

func (u *User) DisplayInfo() {
	switch u.Role {
	case RoleStudent:
		fmt.Printf("Студент: %s (ID: %d)\n", u.Name, u.ID)
	case RoleProfessor:
		fmt.Printf("Преподаватель: %s (ID: %d)\n", u.Name, u.ID)
	}
}

func (u *User) Save(file *os.File) {
	file.WriteString(fmt.Sprintf("%d,%s,%s,%d\n", u.ID, u.Name, u.PasswordHash, u.Role))
}

func HashPassword(password string) string {
	hasher := sha256.New()
	hasher.Write([]byte(password + "university_salt_2024"))
	return hex.EncodeToString(hasher.Sum(nil))
}

func UpdateNextId(newNextId int) {
	nextUserId = newNextId
}

func GetNextId() int {
	return nextUserId
}
