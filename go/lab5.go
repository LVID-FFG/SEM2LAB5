package main

import "fmt"

func main() {
	system := NewUniversitySystem()

	fmt.Println("========================================")
	fmt.Println("    УНИВЕРСИТЕТСКАЯ СИСТЕМА")
	fmt.Println("========================================")

	system.Run()
}
