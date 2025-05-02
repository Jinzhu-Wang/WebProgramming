#include "MySQLConnection.h"
#include <stdexcept>

MySQLConnectionPool& MySQLConnectionPool::getInstance(){
    static MySQLConnectionPool instance;
    return instance;    
}

void MySQLConnectionPool::init(const std::string& host, const std::string& user, const std::string& password, 
                        const std::string& db, int port, int maxConnections){
    std::unique_lock<std::mutex> lock(pool_mutex_);
    host_ = host;
    user_ = user;
    password_ = password;
    db_ = db;
    port_ = port;
    
    for (int i = 0; i < maxConnections; ++i) {
        MYSQL* conn = mysql_init(NULL);
        if (!conn) throw std::runtime_error("MySQL init failed");

        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),
                                db.c_str(), port, NULL, 0)) {
            throw std::runtime_error(mysql_error(conn));
        }

        conn_pool_.push(conn);
    }
    max_conn_ = maxConnections;
}

MYSQL* MySQLConnectionPool::getConnection(){
    std::unique_lock<std::mutex> lock(pool_mutex_);
    while (conn_pool_.empty()) {
        pool_cv_.wait(lock);
    }
    MYSQL* conn = conn_pool_.front();
    conn_pool_.pop();
    return conn;
}

void MySQLConnectionPool::releaseConnection(MYSQL* conn){
    std::unique_lock<std::mutex> lock(pool_mutex_);
    conn_pool_.push(conn);
    pool_cv_.notify_one();
}

MySQLConnectionPool::~MySQLConnectionPool(){
    std::unique_lock<std::mutex> lock(pool_mutex_);
        while (!conn_pool_.empty()) {
            MYSQL* conn = conn_pool_.front();
            mysql_close(conn);
            conn_pool_.pop();
        }
}