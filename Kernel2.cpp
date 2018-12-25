#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctime>
#include <chrono>

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
#include <time.h>

using namespace std;

key_t Up = msgget(303040, IPC_CREAT | 0644);
key_t Down = msgget(404030, IPC_CREAT | 0644);

struct msgbuff
{
	long mtype;
	char mtext[256];
};
struct pro
{
	int id;
	string text;
};

void handleSIGINT(int signum)
{
	msgctl(Up, IPC_RMID, nullptr);
	msgctl(Down, IPC_RMID, nullptr);
	cout << "Kernel Closing ......" << endl;
	exit(EXIT_SUCCESS);
}

list<pro> Qmsg2;
int main()
{
	signal(SIGINT, handleSIGINT);

	cout << " Enter Number of Processes : " << endl;
	int number;
	struct msgbuff Message__Rec;
	cin >> number;
	int *Qmsg = new int[number];
	for (int i = 0; i < number; i++)
	{
		int tempRec = -1;
		while (tempRec == -1)
		{

			tempRec = msgrcv(Up, &Message__Rec, sizeof(Message__Rec.mtext), 0, IPC_NOWAIT);
		}

		Qmsg[i] = Message__Rec.mtype;
		cout << "Process" << i << " PID " << Qmsg[i] << endl;
	}

	cout << "UP---->" << Up << endl;
	cout << "Down--->" << Down << endl;

	// get the PDI of the process from the shared memory
	//int processID, *ShmPTR;
	//key_t MyKey;
	//int ShmID;
	//MyKey = ftok("./", 'a');
	//ShmID = shmget(MyKey, sizeof(int), 0666);
	//ShmPTR = (int *)shmat(ShmID, NULL, 0);
	//processID = *ShmPTR;
	//shmdt(ShmPTR);

	// get the PID of the disk from the shared memory
	int diskID, *ShmPTR1;
	key_t MyKey1;
	int ShmID1;

	MyKey1 = ftok("./", 'b');
	ShmID1 = shmget(MyKey1, sizeof(int), 0666);
	ShmPTR1 = (int *)shmat(ShmID1, NULL, 0);
	diskID = *ShmPTR1;
	shmdt(ShmPTR1);

	//cout << "ProcessID   " << processID << endl;
	cout << "DiskID   " << diskID << endl;

	int sendSig1, sendSig2, sendSig3;
	clock_t t;
	auto start_time = std::chrono::high_resolution_clock::now();
	int flag = 0;
	while (1)
	{
		double duration;
		struct msgbuff Message_Sent;
		struct msgbuff Message_Rec;
		int processID;
		string processData;

		auto end_time = std::chrono::high_resolution_clock::now();
		auto time = end_time - start_time;
		duration = (std::chrono::duration_cast<std::chrono::seconds>(time).count());
		if (duration >= 1)
		{
			start_time = std::chrono::high_resolution_clock::now();
			//cout << "kernel increased the system clk" << endl;
			for (int i = 0; i < number; i++)
			{
				sendSig1 = kill(Qmsg[i], SIGUSR2);
			}

			sendSig2 = kill(diskID, SIGUSR2);
		}

		if (flag == 0 && Qmsg2.size() != 0)
		{

			cout << "checking the status of Disk " << endl;
			sendSig3 = kill(diskID, SIGUSR1);
			//  cout<<"chosen"<<Qmsg[i]<<endl
			flag = 1;
		}

		// Reciev Any message
		int tempRec = -1;
		cout << "kernel waiting........." << endl;
		while (tempRec == -1)
		{

			tempRec = msgrcv(Up, &Message_Rec, sizeof(Message_Sent.mtext), 0, IPC_NOWAIT);
			//check for time while waiting
			auto end_time = std::chrono::high_resolution_clock::now();
			auto time = end_time - start_time;
			duration = (std::chrono::duration_cast<std::chrono::seconds>(time).count());
			if (duration >= 1)
			{
				start_time = std::chrono::high_resolution_clock::now();
				//cout << "kernel increased the system clk" << endl;
				//sendSig1 = kill(processID, SIGUSR2);
				for (int i = 0; i < number; i++)
				{
					sendSig1 = kill(Qmsg[i], SIGUSR2);
				}
				sendSig2 = kill(diskID, SIGUSR2);
			}
		}

		//This message from process

		for (int i = 0; i < number; i++)
		{
			if (Message_Rec.mtype == Qmsg[i])
			{
				pro temp;
				temp.text = Message_Rec.mtext;
				temp.id = Message_Rec.mtype;
				Qmsg2.push_back(temp); //list conatin process with its operation
									   //processData = Message_Rec.mtext;
				cout << "Kernel Recieved from Process " << i << "Data " << temp.text << endl;

				break;
			}
		}

		//This message from process
		//if (Message_Rec.mtype == processID)
		//{
		//processData = Message_Rec.mtext;
		//			cout << "checking the status of Disk " << endl;
		//			sendSig3 = kill(diskID, SIGUSR1);

		//		}
		//**************************************************************************************************//
		//this message is the disk status
		if (Message_Rec.mtype == 1111)
		{
			processData = Qmsg2.front().text;
			processID = Qmsg2.front().id;

			int freeSlots;
			freeSlots = std::stoi(Message_Rec.mtext);
			if (processData[0] == 'A')
			{
				if (freeSlots >= 1)
				{
					cout << "Disk responded that it have a free slots " << endl;
					Message_Sent.mtype = diskID;
					strcpy(Message_Sent.mtext, processData.c_str());
					cout << "Kernel sending the needed operation to the Disk"
						 << "MSGTYP=" << Message_Sent.mtype << "  MSGTXT" << Message_Sent.mtext << endl;
					int tempSend = msgsnd(Down, &Message_Sent, sizeof(Message_Sent.mtext), IPC_NOWAIT);
				}
				else
				{
					cout << "Disk responded that it have NO free slots " << endl;
					Message_Sent.mtype = processID;
					string two = "2";
					strcpy(Message_Sent.mtext, two.c_str());
					cout << "Kernel sending the disk's response to the process " << endl;
					int tempSend = msgsnd(Down, &Message_Sent, sizeof(Message_Sent.mtext), IPC_NOWAIT);
					Qmsg2.pop_front();
					flag = 0;
				}
			}
			else if (processData[0] == 'D')
			{
				if (freeSlots == 10)
				{
					cout << "Disk responded that it have NO slots to delete " << endl;
					Message_Sent.mtype = processID;
					string three = "3";
					strcpy(Message_Sent.mtext, three.c_str());
					cout << "Kernel sending the disk's repsonse to the process " << endl;
					int tempSend = msgsnd(Down, &Message_Sent, sizeof(Message_Sent.mtext), IPC_NOWAIT);
					Qmsg2.pop_front();
					flag = 0;
				}
				else
				{
					cout << "Disk responded that it have slots " << endl;
					Message_Sent.mtype = diskID;
					strcpy(Message_Sent.mtext, processData.c_str());
					cout << "Kernel sending the ID to disk to delete its slot " << endl;
					int tempSend = msgsnd(Down, &Message_Sent, sizeof(Message_Sent.mtext), IPC_NOWAIT);
				}
			}
		}
		//***********************************************************************************************************//
		//this message from the disk
		else if (Message_Rec.mtype == diskID)
		{
			cout << "The disk finished the operation " << endl;
			//Message_Sent.mtype = processID;
			Message_Sent.mtype = processID;
			Qmsg2.pop_front();
			flag = 0;
			strcpy(Message_Sent.mtext, Message_Rec.mtext);
			int tempSend = msgsnd(Down, &Message_Sent, sizeof(Message_Sent.mtext), IPC_NOWAIT);
			cout << endl
				 << tempSend << " here" << processID << endl;
		}
	}
}