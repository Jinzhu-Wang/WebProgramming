#include <iostream>
#include <sys/socket.h> // socket(), bind(), listen(), accept(), recv(), send()
#include <netinet/in.h> // sockaddr_in, INADDR_ANY, htons()
#include <cstring> // memset()
#include <unistd.h> // close()
#include <arpa/inet.h> // inet_addr()
#include <pthread.h> //
#include <fstream>
#include <sstream>

#define BUFSIZE 1024
using namespace std;


void* ClientHandler(void* arg);
void HttpRequestHandler(int cln_sock, string buf);
void SendResponse(int clnt_sock, const string& status, const string& content_type, const string& content);
string read_file(const string& filename);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <port>" << endl;
        return 1;
    }

    int serv_sock,clnt_sock;
    pthread_t thread;
    struct sockaddr_in serv_addr,clnt_addr;
    socklen_t serv_addrlen,clnt_addrlen;

    serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serv_sock == -1) {
        cerr << "socket() error!" << endl;
        return 1;
    }
    int optval = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    serv_addrlen = sizeof(serv_addr);
    if(bind(serv_sock, (struct sockaddr*) &serv_addr,serv_addrlen)==-1){
        cerr << "bind error!" << endl;
        close(serv_sock);
        return 1;
    }

    if(listen(serv_sock,10)==-1){
        cerr<<"listen error"<<endl;
        close(serv_sock);
        return 1;
    }

    // 采用Reactor模式，处理客户端请求
    clnt_addrlen = sizeof(clnt_addr);
    while(1){
        clnt_sock = accept(serv_sock,(struct sockaddr*) &clnt_addr, &clnt_addrlen);
        if(clnt_sock == -1){
            cerr<<"connect error!"<<endl;
            continue;            
        }
        cout << "Connection Request : " << inet_ntoa(clnt_addr.sin_addr) << ":" << ntohs(clnt_addr.sin_port) << std::endl;
        //多线程
        int* clnt_sock_ptr = new int(clnt_sock);
        pthread_create(&thread, NULL, ClientHandler, clnt_sock_ptr);
        pthread_detach(thread);
    }
    
    close(serv_sock);
    return 0;
}

void* ClientHandler(void* arg){
    char buf[BUFSIZE];
    string request;
    int str_len;
    int clnt_sock = *(int*)arg;
    delete (int*)arg;
    while((str_len=recv(clnt_sock, buf, BUFSIZE-1,0))>0){
        buf[str_len] = '\0';  // 确保字符串以 null 结尾
        request += buf;
        if (request.find("\r\n\r\n") != string::npos) break;
    }
    if (str_len == 0 && request.empty()) {
        cout << "Client disconnected " <<clnt_sock << endl;
        close(clnt_sock);
    } else if (str_len == -1) {
        cerr << "recv() error!" <<clnt_sock << endl;
        close(clnt_sock);
    }

    //处理httprequest
    HttpRequestHandler(clnt_sock,request);
    close(clnt_sock);
    cout << "close client: " <<clnt_sock<<endl;
    return NULL;
}
void HttpRequestHandler(int clnt_sock, string buf){
    //处理请求 GET /index.html HTTP/1.1
    size_t first_space = buf.find(' ');
    string method = buf.substr(0, first_space);
    size_t second_space = buf.find(' ', first_space + 1);
    string path = buf.substr(first_space + 1, second_space - first_space - 1);
    size_t version_end = buf.find("\r\n");
    string version = buf.substr(second_space + 1, version_end - second_space - 1);

    if (method == "GET") {
        if (path == "/index.html") {
            string filename = (path == "/") ? "index.html" : path.substr(1); // 默认返回 index.html
            string content = read_file(filename);
            SendResponse(clnt_sock, "200 OK", "text/html", content);
        } else {
            SendResponse(clnt_sock, "404 Not Found", "text/html", "<html><body><h1>404 Not Found</h1></body></html>");
        }
    } else {
        SendResponse(clnt_sock, "501 Not Implemented", "text/plain", "Method not supported");
    }
    
    return;
}
void SendResponse(int clnt_sock, const string& status, const string& content_type, const string& content) {
    string response = "HTTP/1.1 " + status + "\r\n";
    response += "Content-Type: " + content_type + "\r\n";
    response += "Content-Length: " + to_string(content.length()) + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += content;
    send(clnt_sock, response.c_str(), response.length(), 0);
}
string read_file(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return "";
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}