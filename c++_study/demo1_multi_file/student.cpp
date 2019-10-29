#include <iostream>
#include "student.hpp"

namespace STUDENT {
	// constructor function
	Student::Student(std::string name, int age)
	{
		m_name = name;
		m_age = age;
	}
	// destructor function
	Student::~Student() {

	}
	// other method
	void Student::say() {
		std::cout << m_name << "的a年龄是" << m_age << std::endl;
		//std::cout << this->m_name << "的b年龄是" << this->m_age << std::endl;
	}

	// normal function
	void print(Student & stu) {
		std::cout << "print():";
		stu.say();
	}
}
