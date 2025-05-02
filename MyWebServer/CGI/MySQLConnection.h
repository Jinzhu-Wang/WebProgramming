#ifndef MYSQLCONNECTION_H
#define MYSQLCONNECTION_H

#include "common.h"
#include <string>
#include <mysql/mysql.h>
#include <queue>
#include <mutex>
#include <condition_variable>
class MySQLConnectionPool{
    /**
     * @function getInstance: 获取单例对象
     */
public:
    static MySQLConnectionPool& getInstance(); 
    void init(const std::string& host, const std::string& user,
        const std::string& password, const std::string& db,
        int port, int maxConnections = 10);

    MYSQL* getConnection();
    void releaseConnection(MYSQL* conn);
    ~MySQLConnectionPool();
private:
    MySQLConnectionPool(){};
    DISALLOW_COPY_AND_MOVE(MySQLConnectionPool);

    std::string host_, user_, password_, db_;
    int port_, max_conn_;

    std::queue<MYSQL*> conn_pool_;
    std::mutex pool_mutex_;
    std::condition_variable pool_cv_;
};

class MySQLRAII {
    public:
        MySQLRAII(MYSQL*& conn, MySQLConnectionPool& pool)
            : conn_(conn), pool_(pool) {}
    
        ~MySQLRAII() {
            if (conn_) {
                pool_.releaseConnection(conn_);
            }
        }
    
    private:
        MYSQL*& conn_;
        MySQLConnectionPool& pool_;
    };


#endif