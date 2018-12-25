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
string diskSlots[10];
int ShmID;
int *ShmPTR;

struct msgbuff
{
	long mtype;
	char mtext[256];
};
int clk = 0;

key_t Up = msgget(303040, IPC_CREAT | 0644);
key_t Down = msgget(404030, IPC_CREAT | 0644);

struct MsgD
{
	string operation;
	string data;
};

void handlerUser(int signum)
{

	clk++;
	cout << " Disk clk = " << clk << endl;
}

bool AddSlot(string slotdata)
{

	for (int i = 0; i < 10; i++)
	{
		if (diskSlots[i] == "")
		{
			diskSlots[i] = slotdata;

			return true;
		}
	}
	return false;
}
bool removeSlot(string sid)
{
	int id = std::stoi(sid);
	if (diskSlots[id] == "")
		return false;
	else
	{
		diskSlots[id] = "";
		return true;
	}
}
string freeSlots()
{
	int count = 0;
	for (int i = 0; i < 10; i++)
	{
		if (diskSlots[i] == "")
			count++;
	}
	string s = std::to_string(count);
	return s;
}
struct msgbuff Message_Sent;
struct msgbuff Message_Rec;

void handlerUser1(int signum)
{
	Message_Sent.mtype = 1111;
	string s = freeSlots();
	strcpy(Message_Sent.mtext, s.c_str());
	int tempSend = msgsnd(Up, &Message_Sent, sizeof(Message_Sent.mtext), IPC_NOWAIT);
}

int main()
{
	cout << "UP---->" << Up << endl;
	cout << "Down--->" << Down << endl;
	signal(SIGUSR1, handlerUser1);
	signal(SIGUSR2, handlerUser);
	int pid = getpid();
	key_t MyKey;
	MyKey = ftok("./", 'b');
	ShmID = shmget(MyKey, sizeof(int), IPC_CREAT | 0666);
	ShmPTR = (int *)shmat(ShmID, NULL, 0);
	*ShmPTR = pid;

	for (int i = 0; i < 10; i++)
	{
		diskSlots[i] = "";
	}
	cout << "DiskID " << pid << endl;

	while (1)
	{
		if (clk > 0)
		{
			struct MsgD DiskData;
			//int tempSend = msgsnd(Up, &Message_Sent, sizeof(Message_Sent.mtext), IPC_NOWAIT);
			int tempRec = -1;
			while (tempRec == -1)
			{

				tempRec = msgrcv(Down, &Message_Rec, sizeof(Message_Sent.mtext), pid, IPC_NOWAIT);
			}

			if (Message_Rec.mtype == pid)
			{
				string temp = Message_Rec.mtext;
				cout << "KERNEL Data=" << temp << endl;
				DiskData.operation = temp[0];
				cout << "KERNEL OPERATION=" << DiskData.operation << endl;
				temp.erase(0, 2);
				DiskData.data = temp;
				if (DiskData.operation == "A")
				{
					string s;
					int enter_clk = clk;
					bool addPass = AddSlot(DiskData.data);
					if (addPass)
					{
						s = "0";
						cout << "Disk adding the slot  " << endl;
					}
					else
					{
						s = "2";
						cout << "Disk failed to add the slot" << endl;
					}
					Message_Sent.mtype = pid;
					strcpy(Message_Sent.mtext, s.c_str());
					while ((clk - enter_clk) < 3)
					{
					};
					int tempSend = msgsnd(Up, &Message_Sent, sizeof(Message_Sent.mtext), IPC_NOWAIT);
					cout << endl
						 << tempSend << " here" << pid << endl;
				}
				else if (DiskData.operation == "D")
				{
					int enter_clk = clk;
					string s;
					bool removePass = removeSlot(DiskData.data);
					if (removePass)
					{
						s = "1";
						cout << "Disk removing the slot" << endl;
					}
					else
					{
						s = "3";
						cout << "Disk failed to remove the slot" << endl;
					}
					Message_Sent.mtype = pid;
					strcpy(Message_Sent.mtext, s.c_str());
					while ((clk - enter_clk) < 1)
					{
					};
					int tempSend = msgsnd(Up, &Message_Sent, sizeof(Message_Sent.mtext), IPC_NOWAIT);
				}
			}
		}
	}
}