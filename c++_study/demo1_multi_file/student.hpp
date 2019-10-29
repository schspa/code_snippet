#ifndef __HELLO__
#define __HELLO__

#include <iostream>

namespace STUDENT {

	class Student {
	private:
		std::string m_name;
		int m_age;

	public:
		Student(std::string name, int age);
		~Student();
		void say();
	};

	extern void print(Student & stu);
};

#endif
