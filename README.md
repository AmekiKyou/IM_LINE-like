# IM_LINE-like

------------------------------------------------------------------------------------------
* How to use
------------------------------------------------------------------------------------------
This program is using clsocket, make sure you have them make and built in your lib

* To compile, use

  g++ ServerMain.cpp -D_LINUX -std=c++11 -lclsocket
  
* To run, use

  LD_LIBRARY_PATH=/usr/local/lib ./a.out
  
And this establish the server.

Fill config.ini to adjust address and port.

For clients, use:

  telnet [IP Address] [Port] 

  (As written in config.ini)
 
 to start a dialog.

------------------------------------------------------------------------------------------
* Purpose
------------------------------------------------------------------------------------------

Internship test: making an instant messenger like LINE

Compulsory: 1-1 instant message function

Optional: File Transfer/Picture Share/Video Share/Voice Chat/Group message

Form: Source code + Others

課題内容「LINEを作る」
- クライアントはWebでもモバイルでもOK

- 1対1でメッセージをやりとり出来るように
- 他に自分が必須と思う機能を自由に開発してください

ソースコードと、
あれば実際に動作する環境（URL等）を準備してください。
