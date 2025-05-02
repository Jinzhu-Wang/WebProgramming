#include <iostream>
#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "EventLoop.h"
#include "Logging.h"
#include "AsyncLogging.h"
#include "MySQLConnection.h"

#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <dirent.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <libgen.h>

std::string GetCurrentDir() ;

void HandleLogin(const std::string& username, const std::string& password, HttpResponse* response) {
    MYSQL* conn = MySQLConnectionPool::getInstance().getConnection();
    std::string body ;
    if (!conn) {
        LOG_ERROR << "无法获取数据库连接";
        body="无法获取数据库连接";
    }
    MySQLRAII raii(conn, MySQLConnectionPool::getInstance()); //自动归还链接
    std::string query = "SELECT * FROM user WHERE username='" + username + "' AND passwd='" + password + "'"; //有数据注入的风险
    
    // std::cout << "SQL: " << query << std::endl;
    if (mysql_query(conn, query.c_str()) != 0) {
        LOG_ERROR << "Database query failed: " << mysql_error(conn);
        body = "db error!\n";
    } else {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result && mysql_num_rows(result) > 0) {
            body = "login ok!\n";
        } else {
            LOG_ERROR << "Login failed for username: " << username;
            body = "login failed!\n";
        }
        mysql_free_result(result);
    }
    response->SetBody(body);
    response->SetContentLength(body.size());
    response->SetStatusCode(HttpStatusCode::k200K);
    response->SetStatusMessage("OK");
    response->SetContentType("text/plain");
}

void HandleRegister(const std::string& username, const std::string& password, HttpResponse* response) {
    MYSQL* conn = MySQLConnectionPool::getInstance().getConnection();
    if (!conn) {
        LOG_ERROR << "无法获取数据库连接";
        std::string body = "数据库连接失败!\n";
        response->SetStatusCode(HttpStatusCode::k500InternalServerError);
        response->SetStatusMessage("Internal Server Error");
        response->SetBody(body);
        response->SetContentType("text/plain");
        response->SetContentLength(body.size());
        response->AddHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->AddHeader("Pragma", "no-cache");
        response->AddHeader("Expires", "0");
        return;
    }
    MySQLRAII raii(conn, MySQLConnectionPool::getInstance());

    // 转义用户名和密码以防止 SQL 注入
    char escaped_username[username.length() * 2 + 1];
    char escaped_password[password.length() * 2 + 1];
    mysql_real_escape_string(conn, escaped_username, username.c_str(), username.length());
    mysql_real_escape_string(conn, escaped_password, password.c_str(), password.length());

    // 检查用户名是否已存在
    std::string check_query = "SELECT * FROM user WHERE username = '" + std::string(escaped_username) + "'";
    if (mysql_query(conn, check_query.c_str()) != 0) {
        LOG_ERROR << "检查用户名查询失败: " << mysql_error(conn);
        std::string body = "数据库错误!\n";
        response->SetStatusCode(HttpStatusCode::k500InternalServerError);
        response->SetStatusMessage("Internal Server Error");
        response->SetBody(body);
        response->SetContentType("text/plain");
        response->SetContentLength(body.size());
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    std::string body;
    if (result && mysql_num_rows(result) > 0) {
        LOG_ERROR << "用户名已存在: " << username;
        body = "用户名已存在!\n";
        response->SetStatusCode(HttpStatusCode::k400BadRequest);
        response->SetStatusMessage("Bad Request");
    } else {
        // 插入新用户
        std::string insert_query = "INSERT INTO user (username, passwd) VALUES ('" + 
                                  std::string(escaped_username) + "', '" + 
                                  std::string(escaped_password) + "')";
        if (mysql_query(conn, insert_query.c_str()) != 0) {
            LOG_ERROR << "插入用户失败: " << mysql_error(conn);
            body = "注册失败，数据库错误!\n";
            response->SetStatusCode(HttpStatusCode::k500InternalServerError);
            response->SetStatusMessage("Internal Server Error");
        } else {
            body = "注册成功!\n";
            response->SetStatusCode(HttpStatusCode::k200K);
            response->SetStatusMessage("OK");
        }
    }
    mysql_free_result(result);

    response->SetBody(body);
    response->SetContentType("text/plain; charset=UTF-8");
    response->SetContentLength(body.size());
    response->AddHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->AddHeader("Pragma", "no-cache");
    response->AddHeader("Expires", "0");
}
// 读取文件
std::string ReadFile(const std::string& path){
    std::string complete_path = GetCurrentDir()+"/"+path;
    // std::cout<<"complete_read: "<<complete_path<<std::endl;
    std::ifstream is(complete_path.c_str(), std::ifstream::in);

    if (!is) {
        std::cerr << "Failed to open file: " << complete_path << std::endl;
        return "";  // 或者 throw/记录日志
    }

    // 寻找文件末端
    is.seekg(0, is.end);

    // 获取长度
    int flength = is.tellg();
    if (flength <= 0) {
        std::cerr << "File is empty or invalid: " << complete_path << std::endl;
        return "";
    }

    //重新定位
    is.seekg(0, is.beg);
    char * buffer = new char[flength];

    // 读取文件
    is.read(buffer, flength);
    std::string msg(buffer, flength);
    return msg;
}


// 获取当前目录下所有文件的名字
void FindAllFiles(const std::string& path, std::vector<std::string> &filelist){
    DIR *dir;
    struct dirent *dir_entry = NULL;
    std::string complete_path = GetCurrentDir()+"/"+path;
    // std::cout<<"complete_find: "<<complete_path<<std::endl;
    if((dir = opendir(complete_path.c_str())) == NULL){
        LOG_ERROR << "Opendir " << path << " failed";
        return;
    }
    
    while((dir_entry = readdir(dir))!= NULL){
        std::string filename = dir_entry->d_name;
        if (filename != "." && filename != ".."){
            filelist.push_back(filename);
        }
    }
}

// 构建filelist.html
std::string BuildFileHtml(){
    std::vector<std::string> filelist;
    // 以/files文件夹为例
    FindAllFiles("../files", filelist);

    // 为文件生成模板
    std::string file = "";
    for (auto filename : filelist)
    {
        //将fileitem中的所有filename替换成
        file += "<tr><td>" + filename + "</td>" +
                "<td>" +
                "<a href=\"/download/" + filename + "\">下载</a>" +
                "<a href=\"/delete/" + filename + "\">删除</a>" +
                "</td></tr>" + "\n";
    }


    //生成html页面
    // 主要通过将<!--filelist-->直接进行替换实现
    std::string tmp = "<!--filelist-->";
    std::string filehtml = ReadFile("../static/fileserver.html");
    filehtml = filehtml.replace(filehtml.find(tmp), tmp.size(), file);
    return filehtml;
}


void RemoveFile(const std::string & filename,HttpResponse *response){
    std::string dir_path = GetCurrentDir()+"/../files/";
    // std::cout<<"Removefilename: "<<filename<<std::endl;
    // std::cout<<"---------------------"<<std::endl;
    int ret = remove(( dir_path + filename).c_str());
    if(ret != 0){
        LOG_ERROR << "删除文件 " << filename << " 失败";
        response->SetStatusCode(HttpStatusCode::k500InternalServerError);
        response->SetStatusMessage("Internal Server Error");
        response->SetBody("Failed to delete file\n");
    }else{
        LOG_INFO << "删除文件 " << filename << " 成功";
        // 设置响应头字段
        response->SetStatusCode(HttpStatusCode::k302K);
        response->SetStatusMessage("Moved Temporarily");
        response->SetContentType("text/html");
        response->AddHeader("Location", "/fileserver");

        // 设置文件
        response->SetFileFd(-1);
    }
}

void DownloadFile(const std::string &filename, HttpResponse *response){
    std::string dir_path = GetCurrentDir()+"/../files/";
    // std::cout<<"Downfilename: "<<filename<<std::endl;
    // std::cout<<"---------------------"<<std::endl;
    int filefd = ::open((dir_path + filename).c_str(), O_RDONLY);
    if(filefd == -1){
        LOG_ERROR << "OPEN FILE ERROR";
        response->SetStatusCode(HttpStatusCode::k302K);
        response->SetStatusMessage("Moved Temporarily");
        response->SetContentType("text/html");
        response->AddHeader("Location", "/fileserver");
    }else{
        // 获取文件信息
        struct stat fileStat;
        fstat(filefd, &fileStat);
        // 设置响应头字段
        response->SetStatusCode(HttpStatusCode::k200K);
        response->SetContentLength(fileStat.st_size);
        response->SetContentType("application/octet-stream");
        
        response->SetBodyType(HttpBodyType::FILE_TYPE);
        // response->AddHeader("Transfer-Encoding", "chunked");

        // 设置文件
        response->SetFileFd(filefd);
    }
}

std::string GetCurrentDir() {
    char current_dir[1024];
    if (realpath(__FILE__, current_dir) == NULL) {
        std::cerr << "Error retrieving the current file path." << std::endl;
        return "";
    }
    std::string dir_path(dirname(current_dir));
    return dir_path;
}
    

void HttpResponseCallback(const HttpRequest &request, HttpResponse *response)
{
    // LOG_INFO << request.GetMethodString() << " " << request.url();
    std::string url = request.url();
    if(request.method() == RequestMethod::kGet){
        
        if(url == "/"){
            std::string body = ReadFile("../static/index.html");
            response->SetStatusCode(HttpStatusCode::k200K);
            response->SetContentLength(body.size());
            response->SetBody(body);
            response->SetContentType("text/html");
        }else if(url == "/mhw"){
            std::string body = ReadFile("../static/mhw.html");
            response->SetStatusCode(HttpStatusCode::k200K);
            response->SetContentLength(body.size());
            response->SetBody(body);
            response->SetContentType("text/html");
        }else if(url == "/cat.jpg"){
            std::string body = ReadFile("../static/cat.jpg");
            response->SetContentLength(body.size());
            response->SetStatusCode(HttpStatusCode::k200K);
            response->SetBody(body);
            response->SetContentType("image/jpeg");
        }else if(url == "/fileserver"){
            std::string body = BuildFileHtml();
            response->SetContentLength(body.size());
            response->SetStatusCode(HttpStatusCode::k200K);
            response->SetBody(body);
            response->SetContentType("text/html");
        }else if(url.substr(0, 7) == "/delete") {
            // 删除特定文件，由于使用get请求，并且会将相应删掉文件的名称放在url中
            RemoveFile(url.substr(8),response);
            // 发送重定向报文，删除后返回自身应在的位置
            // response->SetStatusCode(HttpStatusCode::k302K);
            // response->SetStatusMessage("Moved Temporarily");
            // response->SetContentType("text/html");
            // response->AddHeader("Location", "/fileserver");

        }else if(url.substr(0, 9) == "/download"){
            DownloadFile(url.substr(10), response);
            //response->SetStatusCode(HttpResponse::HttpStatusCode::k200K);
        }else if(url == "/favicon.ico"){
            std::string body = ReadFile("../static/cat.jpg");
            response->SetStatusCode(HttpStatusCode::k200K);
            response->SetBody(body);
            response->SetContentType("image/jpeg");
        }else
        {
            response->SetStatusCode(HttpStatusCode::k404NotFound);
            response->SetStatusMessage("Not Found");
            response->SetBody("Sorry Not Found\n");
            response->SetCloseConnection(true);
        }
    }
    else if( request.method() == RequestMethod::kPost){
        if(url == "/login"){
            // 进入登陆界面
            std::string rqbody = request.body();

            // 解析
            int usernamePos = rqbody.find("username=");
            int passwordPos = rqbody.find("password=");

            usernamePos += 9; // "username="的长度
            passwordPos += 9; // 

            // 找到中间分割符
            size_t usernameEndPos = rqbody.find('&', usernamePos);
            size_t passwordEndPos = rqbody.length();

            // Extract the username and password substrings
            std::string username = rqbody.substr(usernamePos, usernameEndPos - usernamePos);
            std::string password = rqbody.substr(passwordPos, passwordEndPos - passwordPos);
            // std::cout<<"username: "<<username<<" passwd: "<<password<<std::endl;
            HandleLogin(username,password,response);

        }else if (url == "/register") {
            std::string rqbody = request.body();
            std::string::size_type usernamePos = rqbody.find("username=");
            std::string::size_type passwordPos = rqbody.find("password=");
            if (usernamePos == std::string::npos || passwordPos == std::string::npos) {
                LOG_ERROR << "无效的注册表单数据";
                std::string body = "无效的注册表单数据";
                response->SetStatusCode(HttpStatusCode::k400BadRequest);
                response->SetStatusMessage("Bad Request");
                response->SetBody(body);
                response->SetContentType("text/plain");
                response->SetContentLength(body.size());
            }
            usernamePos += 9;
            passwordPos += 9;
            std::string::size_type usernameEndPos = rqbody.find('&', usernamePos);
            std::string::size_type passwordEndPos = rqbody.length();
            std::string username = rqbody.substr(usernamePos, usernameEndPos - usernamePos);
            std::string password = rqbody.substr(passwordPos, passwordEndPos - passwordPos);
            // std::cout << "注册用户: " << username << " 密码: " << password << std::endl;
            HandleRegister(username, password, response);

        }else if(url == "/upload"){
            response->SetStatusCode(HttpStatusCode::k302K);
            response->SetStatusMessage("Moved Temporarily");
            response->SetContentType("text/html");
            response->AddHeader("Location", "/fileserver");
        }
    }

    //LOG_INFO << response->message();
    return;
}


// 异步日志库相关
std::unique_ptr<AsyncLogging> asynclog;
void AsyncOutputFunc(const char *data, int len)
{
    asynclog->Append(data, len);
}
void AsyncFlushFunc() {
    asynclog->Flush();
}

int main(int argc, char *argv[]){
    int port;
    if (argc <= 1)
    {
        port = 1234;
    }else if (argc == 2){
        port = atoi(argv[1]);
    }else{
        printf("error");
        exit(0);
    }
    // 开发阶段暂时不适用异步日志
    asynclog = std::make_unique<AsyncLogging>();
    Logger::setOutput(AsyncOutputFunc);
    Logger::setFlush(AsyncFlushFunc);

    asynclog->Start();
    //初始化数据库连接池
    std::string user = "wangjz";
    std::string passwd = "111111";
    std::string database_name = "webdb";

    MySQLConnectionPool& connection_pool = MySQLConnectionPool::getInstance();
    connection_pool.init("localhost", user, passwd, database_name, 3306, 10);

    int size = std::thread::hardware_concurrency() - 1;
    std::cout<<"thread_nums: "<<size<<std::endl;
    EventLoop *loop = new EventLoop();
    HttpServer *server = new HttpServer(loop, "127.0.0.1", port, true);
    server->SetHttpCallback(HttpResponseCallback);
    server->SetThreadNums(size);
    server->start();
    
    //delete loop;
    //delete server;
    return 0;
}