#include "student.hpp"

// C++类，多文件编译，多namespace应用demo

int main(int argc, char *argv[])
{
	using namespace STUDENT;

	Student stu = Student{"lily", 18};
	stu.say();
	print(stu);
	return 0;
}
