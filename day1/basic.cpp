#include<iostream>
#include<sys/types.h>
#include<unistd.h>
using namespace std;
int main()
{
	cout<<"PID : "<<getpid()<<endl;
	cout<<"PPID : "<<getppid()<<endl;
	for(;;);
	return 0;
}
