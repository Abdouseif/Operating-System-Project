#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>

#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <iostream>
#include <fstream>
#include <iostream>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/msg.h>
#include <string>
#include <string.h>
#include <list>
#include <fstream>
#include <sstream>
#include <signal.h>

using namespace std;

int ShmID;
int *ShmPTR;

struct msgbuff
{
	long mtype;
	char mtext[256];
};
int clk = 0;

key_t Up;
key_t Down;

struct Msgp
{
	int time;
	string operation;
	string data;
};

list<Msgp> Qmsg;

void handlerUser(int signum)
{

	clk++;
	cout << " Process clk = " << clk << endl;
}

int main()
{
	Up = msgget(303040, IPC_CREAT | 0644);
	Down = msgget(404030, IPC_CREAT | 0644);
	cout << "UP---->" << Up << endl;
	cout << "down--->" << Down << endl;
	char *FileName = new char[50];

	cout << " Enter file name : " << endl;
	cin.getline(FileName, 50);
	signal(SIGUSR2, handlerUser);

	int pid = getpid();
	key_t MyKey;
	MyKey = ftok("./", 'a');
	ShmID = shmget(MyKey, sizeof(int), IPC_CREAT | 0666);
	ShmPTR = (int *)shmat(ShmID, NULL, 0);
	*ShmPTR = pid;
	Msgp temp;

	ifstream input(FileName, ios::in);
	ifstream myfile;
	myfile.open(FileName);
	cout << "ProcessID" << pid << endl;

	struct msgbuff Process_Starter;
	//int pid = getpid();
	Process_Starter.mtype = pid;

	key_t temp2 = msgsnd(Up, &Process_Starter, sizeof(Process_Starter.mtext), IPC_NOWAIT); //send pid
	if (myfile)
	{

		while (!myfile.eof())
		{

			myfile >> temp.time;

			myfile >> temp.operation;

			getline(myfile, temp.data);
			std::istringstream parse(temp.data);

			Qmsg.push_back(temp);

			cout << temp.time << endl;
		}
		Qmsg.pop_back();
	}
	myfile.close();

	struct msgbuff Message_Sent;
	struct msgbuff Message_Rec;

	while (Qmsg.size() != 0)
	{

		if (clk > 0)
		{
			struct Msgp temp;
			temp = Qmsg.front();
			string Text;

			if (temp.time <= clk)
			{
				Message_Sent.mtype = pid;
				if (temp.operation == "ADD")
				{

					Text = temp.data;

					Text = 'A' + Text;
				}
				else if (temp.operation == "DEL")
				{

					Text = temp.data;

					Text = 'D' + Text;
				}
				strcpy(Message_Sent.mtext, Text.c_str());
				int tempSend;
				tempSend = msgsnd(Up, &Message_Sent, sizeof(Message_Sent.mtext), IPC_NOWAIT);

				if (tempSend == -1)
					perror("Errror in send");
				else
				{
					cout << "Process Message to kernel type:  " << Message_Sent.mtype << "  "
						 << "Data:" << Message_Sent.mtext << endl;
				}
				Qmsg.pop_front();
				int tempRec = -1;
				cout << "waiting for the kernel..." << endl;
				cout << "Message type: " << Message_Sent.mtype << "Message text: " << Message_Sent.mtext << endl;
				while (tempRec == -1)
				{
					tempRec = msgrcv(Down, &Message_Rec, sizeof(Message_Sent.mtext), pid, IPC_NOWAIT);
				}
				if (strcmp(Message_Rec.mtext, "0") == 0)
					cout << "Successful ADD " << endl;
				else if (strcmp(Message_Rec.mtext, "1") == 0)
					cout << "Successful DEL " << endl;
				else if (strcmp(Message_Rec.mtext, "2") == 0)
					cout << "unable to ADD " << endl;
				else if (strcmp(Message_Rec.mtext, "3") == 0)
					cout << "unable to DEL " << endl;
			}
		}
	}
}
