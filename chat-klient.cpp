//
// Created by Tomáš Kohout (xkohou08) on 6.4.17.
//

#include <iostream>
#include <vector>
#include <string>
#include <locale>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>    //strlen
#include <arpa/inet.h> //inet_addr
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define OK 0
#define ERR 1

using namespace std;

struct arg{
    string ip;
    string uName;
};

struct tData {
    string uName;
    int sock;
};

tData *data = new tData;
bool sigHasOccured = false;
pthread_t threads[2];

void sigHandler(int sig){
    string msg = data->uName + " logged out\r\n";
    send(data->sock, msg.c_str(), msg.size(), 0);
    close(data->sock);
    delete data;
    exit(OK);
}

void errHandler(const char *msg){
    delete data;
    perror(msg);
    exit(ERR);
}


arg * getParams(int argc,char ** argv){
    string iArg = "-i";
    string uArg = "-u";
    if (argc != 5)
        errHandler("Nesprávný počet parametrů");

    arg *args = new arg;
    if (iArg.compare(argv[1]) == 0 && uArg.compare(argv[3]) == 0)
    {
        args->ip = argv[2];
        args->uName = argv[4];
    }
    else if (iArg.compare(argv[3]) == 0 && uArg.compare(argv[1]) == 0)
    {
        args->ip = argv[4];
        args->uName = argv[2];
    }
    else
        errHandler("Neznámé operátory");

    return args;
}

//funkce na zápis do stdout
void *writeItAll(void *sockfd){
    char buffer[1024];
    bzero(buffer, 1024);
    int n;
    string rcv;
    while (!sigHasOccured){
        bzero(buffer, 1024);
        n = (int) recv(data->sock, buffer, 1024, 0);
        if (n > 0)
           rcv += string(buffer, (unsigned ) n);
        else
            continue;
        cout << rcv;
        rcv.clear();
    }

    _exit(0);
}


//funkce na čtení ze stdin
void * getEmAll(void *sockfd) {
    string msg;
    string sendMsg;
    while (!sigHasOccured){
        cin.clear();
        getline(cin, msg);
        if (msg == "")
            continue;
        sendMsg = data->uName + ": " + msg + "\r\n";

        send(data->sock, sendMsg.c_str(), sendMsg.size(), 0);

        msg.clear();
        sendMsg.clear();
    }
    _exit(0);
}

int main(int argc , char *argv[])
{
    struct sigaction sigHand;
    sigHand.sa_handler = sigHandler;
    sigemptyset(&sigHand.sa_mask);
    sigHand.sa_flags = 0;

    sigaction(SIGINT, &sigHand, NULL);
    int pC;

    arg *str = getParams(argc, argv);
    string uName = str->uName;
    int sockfd, portno, n;
    sockaddr_in serv_addr;
    hostent *server = NULL;

    portno = 21011;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        delete str;
        errHandler( "Nevytvořil se socket");
    }

    server = gethostbyname(str->ip.c_str());

    if (server == NULL)
    {
        delete str;
        errHandler("Něco se pokazilo s IP adresou. Je zadaná správně?");
    }
    //uvolnění struktury pro parametry, dále již není potřeba.
    delete str;

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family    =   AF_INET;
    bcopy((char *) server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);

    serv_addr.sin_port  =   htons(portno);
    n = (int) connect(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr));
    if (n < 0)
    {
        close(sockfd);
        errHandler("ERROR: connecting");
    }

    data->sock = sockfd;
    data->uName = str->uName;

    string welcome = str->uName + " logged in\r\n";

    send(sockfd, welcome.c_str(), welcome.size(), 0);

    pC = pthread_create(&threads[0], NULL,getEmAll, (void *) &sockfd);

    if (pC){
        errHandler("Nevytvořilo se vlákno.");
    }

    writeItAll((void *)&sockfd);

}
