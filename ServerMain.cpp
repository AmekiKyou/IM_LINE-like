/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Servermain.cpp - Establish a server to host group chat,                    */
/*                  together with 1-1 private messenger via telnet            */
/*                                                                            */
/* Author: Yusheng Jiang (amadeus@yama.info.waseda.ac.jp)                     */
/*                                                                            */
/* Refenrencing code: 1. clsocket via github                                  */
/*                    https://github.com/DFHack/clsocket                      */
/*                    2. anonymous code via cppreference                      */
/*                    http://www.cplusplus.com/forum/lounge/14504/            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include<iostream>
#include<fstream>
#include<ctime>
#include"PassiveSocket.h"
#define VERSION 0.01
#define NAME_LEN 19
#define MAX_PACKET 163
#define MAX_CLIENTS 100
#define TIME_OUT 600 //10 minutes
using namespace std;
void bcast(CActiveSocket **clients, int clientCount, string data, int except);
void sendto(CActiveSocket *client, string data);
string new_nick(string names[MAX_CLIENTS], int clientCount);
int main (void){
  CPassiveSocket server;
	CActiveSocket *clients[MAX_CLIENTS], *incoming=NULL;
  for (int i=0;i<MAX_CLIENTS;++i) clients[i]=NULL;
  int clientCount;
  string names[MAX_CLIENTS],msg[MAX_CLIENTS];
	time_t time_of_last_send[MAX_CLIENTS],presentTime;
  struct tm* tmptime;
  string tmpdata,tmpnick;
  string HOST;
  int PORT;
  ifstream fin ("Config.ini");
  fin>>HOST>>PORT;
	server.Initialize();
	server.SetNonblocking();
	server.Listen(HOST.c_str(),PORT);
	cout<<"TALKER v"<<VERSION<<endl;
	cout<<"Address: "<<HOST<<" Port: "<<PORT<<endl;
	clientCount=0;
	cout<<"Number of clients: "<<clientCount<<endl;
  while (1){
    if ((incoming=server.Accept())!=NULL){
			if (clientCount < MAX_CLIENTS){
        clients[clientCount] = incoming;
        clients[clientCount]->SetNonblocking();
        names[clientCount]=new_nick(names, clientCount);
				msg[clientCount]="";
        time_of_last_send[clientCount]=time(NULL);
        tmptime=localtime(&time_of_last_send[clientCount]);
        cout<<"On time: "<<asctime(tmptime)<<endl;
        cout<<names[clientCount]<<" has joined."<<endl;
        time(&presentTime);
        tmptime=localtime(&presentTime);
        tmpdata="Welcome "+names[clientCount]+"!!\n"+names[clientCount]+" "+asctime(tmptime)+" >";
        sendto(clients[clientCount],tmpdata);
        clientCount++;
				cout<<"Number of clients: "<<clientCount<<endl;
      }
    }
  }
}
