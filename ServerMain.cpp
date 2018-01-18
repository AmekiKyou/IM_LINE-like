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
#include<string>
#include"PassiveSocket.h"
#define VERSION 0.01
#define NAME_LEN 19
#define MAX_PACKET 163
#define MAX_CLIENTS 100
#define TIME_OUT 600 //10 minutes
using namespace std;
void bcast(CActiveSocket **clients, int clientCount, string data, int except);
void sendto(CActiveSocket *client, string data);
int main (void){
  CPassiveSocket server;
	CActiveSocket *clients[MAX_CLIENTS], *incoming=NULL;
  for (int i=0;i<MAX_CLIENTS;++i) clients[i]=NULL;
  int clientCount;
  string names[MAX_CLIENTS],msg[MAX_CLIENTS];
	time_t time_of_last_send[MAX_CLIENTS],presentTime;
  struct tm* tmptime;
  string tmpdata,tmpnick,tmptarget;
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
			if (clientCount<MAX_CLIENTS){
        clients[clientCount]=incoming;
        clients[clientCount]->SetNonblocking();
        names[clientCount]="User#"+to_string(clientCount);
				msg[clientCount]="";
        time_of_last_send[clientCount]=time(NULL);
        tmptime=localtime(&time_of_last_send[clientCount]);
        cout<<"On time: "<<asctime(tmptime)<<endl;
        cout<<names[clientCount]<<" has joined."<<endl;
        time(&presentTime);
        tmptime=localtime(&presentTime);
        tmpdata="\n> Welcome "+names[clientCount]+"!!\n> "+names[clientCount]+": "+asctime(tmptime)+"\n> Type .help for help information.\n> ";
        sendto(clients[clientCount],tmpdata);
        tmpdata="\n> "+string(asctime(tmptime))+"> User "+names[clientCount]+"("+to_string(clientCount)+") has joined the chat.\n> ";
        bcast(clients,clientCount,tmpdata,clientCount);
        ++clientCount;
				cout<<"Number of clients: "<<clientCount<<endl;
      }
      else{
        string server_full="\n> Sorry, the server is currently full. Please try connection again later.\n";
        incoming->Send((unsigned char *)server_full.c_str(),server_full.size());
        incoming->Close();
      }
    }
    time(&presentTime);
    tmptime=localtime(&presentTime);
    for (int i=0;i<clientCount;++i){
      if (clients[i]->Receive(MAX_PACKET)>0){
        char msgBuffer[MAX_PACKET+20];
        strcpy(msgBuffer, "");
				strncat(msgBuffer, (const char *) clients[i]->GetData(), clients[i]->GetBytesReceived());
				tmpdata=string(msgBuffer);
        cout<<"Server catch data from client "<<names[i]<<": "<<tmpdata<<endl;
        msg[i]+=tmpdata;
        time_of_last_send[i]=presentTime;
      }
      else if (difftime(presentTime,time_of_last_send[i])>TIME_OUT){
        sendto(clients[i],"\n> Your connection is automatically refused due to long time no response.\n");
        tmpdata="\n> Server removed user "+names[i]+" due to no response.\n";
        bcast(clients,clientCount,tmpdata,i);
        cout<<tmpdata<<endl;
        clients[i]->Close();
        --clientCount;
        for (int j=i;j<clientCount;++j){
          msg[j]=msg[j+1];
          names[j]=names[j+1];
          time_of_last_send[j]=time_of_last_send[j+1];
          clients[j]=clients[j+1];
        }
        cout<<"Number of clients: "<<clientCount<<endl;
      }
    }
    for (int i=0;i<clientCount;++i){
      if (msg[i].size()>0&&msg[i].find('\n')!=std::string::npos){
        msg[i].erase(msg[i].find('\n')-1);
        cout<<"Received message from "<<names[i]<<": "<<msg[i]<<endl;
        if (msg[i].size()>0){
          if (msg[i][0]!='.'){
            //group chat message
            cout<<"On time: "<<asctime(tmptime)<<endl;
            cout<<names[i]<<" wrote: "<<msg[i]<<endl;
            tmpdata="> "+names[i]+": "+msg[i]+"\n> ";
            bcast(clients,clientCount,"\n> "+string(asctime(tmptime)),-1);
            bcast(clients,clientCount,tmpdata,-1);
          }
          else{
            if (msg[i].size()>=6&&msg[i].substr(0,6)==".nick "){
              //change nickname
              tmpnick=msg[i].substr(6);
              tmpdata="\n> User "+names[i]+"("+to_string(i)+") is now known as "+tmpnick+".\n> ";
              cout<<tmpdata<<endl;
              bcast(clients,clientCount,tmpdata,i);
              names[i]=tmpnick;
              sendto(clients[i],"\n> ");
            }
            else if (msg[i].size()>=5&&msg[i].substr(0,5)==".quit"){
              //quit chat
              sendto(clients[i],"\n> Goodbye.\n");
              tmpdata="\n> User "+names[i]+"("+to_string(i)+") has left the chat.\n> ";
              cout<<tmpdata<<endl;
              bcast(clients,clientCount,tmpdata,i);
              clients[i]->Close();
              --clientCount;
              for (int j=i;j<clientCount;++j){
                msg[j]=msg[j+1];
                names[j]=names[j+1];
                time_of_last_send[j]=time_of_last_send[j+1];
                clients[j]=clients[j+1];
              }
              cout<<"Number of clients: "<<clientCount<<endl;
              goto afterdel;
            }
            else if (msg[i].size()>=4&&msg[i].substr(0,4)==".pm "){
              //private message
              tmpdata=msg[i].substr(4);
              if (tmpdata.find(' ')!=std::string::npos){
                tmptarget=tmpdata.substr(0,tmpdata.find(' '));
                tmpdata=tmpdata.substr(tmpdata.find(' '));
                if (tmpdata.size()<=1) goto workup;
                bool findflag=false;
                for (int j=0;j<clientCount;++j)
                  if (names[j]==tmptarget){
                    tmpdata="\n> "+string(asctime(tmptime))+"> User "+names[i]+"("+to_string(i)+") sent you a message:\n> "+tmpdata+"\n> ";
                    cout<<"Sent to user: "<<tmptarget<<endl;
                    cout<<tmpdata<<endl;
                    sendto(clients[j],tmpdata);
                    time_of_last_send[j]=presentTime;
                    findflag=true;
                    break;
                  }
                if (!findflag){
                  tmpdata="\n> No match found for username: "+tmptarget+"\n> ";
                  cout<<tmpdata<<endl;
                  sendto(clients[i],tmpdata);
                }
                else sendto(clients[i],"\n> ");
              }
              else{
                tmpdata="\n> Receipient client username undeclared. Please check. \n> ";
                cout<<tmpdata<<endl;
                sendto(clients[i],tmpdata);
              }
            }
            else if (msg[i].size()>=9&&msg[i].substr(0,9)==".userlist"){
              //print user list
              tmpdata="\n> Currently active userlist\n> ";
              for (int j=0;j<clientCount;++j)
                tmpdata=tmpdata+" "+names[j]+"("+to_string(j)+")\n> ";
              cout<<tmpdata<<endl;
              sendto(clients[i],tmpdata);
            }
            else if (msg[i].size()>=5&&msg[i].substr(0,5)==".time"){
              //print current time
              tmpdata="\n> Current system time: "+string(asctime(tmptime))+"\n> ";
              cout<<tmpdata<<endl;
              sendto(clients[i],tmpdata);
            }
            else if (msg[i].size()>=5&&msg[i].substr(0,5)==".help"){
              //print manual/help information
              sendto(clients[i],"\n> Type a message less than 160 words to start chat. Type a command starting with \'.\' to command orders.\n> ");
              sendto(clients[i],"\n> Commands:\n> ");
              sendto(clients[i],"\n>  (1) .nick [Your Nickname]: Change your nickname. You are automatically allocated with a name, replace it with a word less than 18 characters.\n> ");
              sendto(clients[i],"\n>  (2) .userlist: Print the current active user list.\n> ");
              sendto(clients[i],"\n>  (3) .pm [Receipient Nickname] [Message]: Send a private message.\n> ");
              sendto(clients[i],"\n>  (4) .time: Print the current system time on server.\n> ");
              sendto(clients[i],"\n>  (5) .quit: Exit the system.\n> ");
            }
            else{
              tmpdata="\n> Undesignated command.\n> ";
              cout<<tmpdata<<endl;
              sendto(clients[i],tmpdata);
            }
          }
        }
        workup:
          msg[i]="";
        afterdel:
          ;
      }
    }
  }
  server.Close();
  return 0;
}
void bcast(CActiveSocket **clients,int clientCount,string data,int except){
  for (int i=0;i<clientCount;++i)
    if (i!=except) clients[i]->Send((unsigned char *)data.c_str(),data.size());
}
void sendto(CActiveSocket *client,string data){
  client->Send((unsigned char *)data.c_str(),data.size());
}
