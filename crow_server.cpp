#include "./third_party/crow_all.h"
#include <string>
#include <iostream>
#include <mysql/mysql.h>
#include "CGImysql/sql_connection_pool.h" // 使用已有连接池接口

int main()
{
    string USER = "root";
    string PASSWORD = "root"; 
    string DATABASENAME = "yourdb";
    int SQL_NUM = 8; 
    int CLOSE_LOG = 0;
    int LOG_WRITE = 1; //假设日志写入方式为异步
    //启动日志系统
    if (0 == CLOSE_LOG)
    {
        //初始化日志
        if (1 == LOG_WRITE)
            Log::get_instance()->init("./ServerLog", CLOSE_LOG, 2000, 800000, 800);
        else
            Log::get_instance()->init("./ServerLog", CLOSE_LOG, 2000, 800000, 0);
    }
    
    // 使用你工作区中的连接池获取 MySQL 连接
    connection_pool *pool = connection_pool::GetInstance();
    if (!pool) {
        std::cerr << "ERROR: connection_pool::GetInstance() returned nullptr\n";
        //return crow::response(500, "db pool not initialized");
        return 0;
    }
    pool->init("localhost", USER.c_str(), PASSWORD.c_str(), DATABASENAME.c_str(), 3306, SQL_NUM, CLOSE_LOG);

    crow::SimpleApp app;
    // 示例：GET /api/hello
    CROW_ROUTE(app, "/api/hello").methods("GET"_method)([](){
        crow::json::wvalue res;
        res["message"] = "hello from crow";
        return crow::response{200, res};
    });

    // 示例：POST /api/users  接收 JSON { "username": "...", "passwd": "..." }
    CROW_ROUTE(app, "/api/users").methods("POST"_method)([pool](const crow::request& req){
        

        auto body = crow::json::load(req.body);
        if (!body)
            return crow::response(400, "invalid json");

        std::string name = body["username"].s();
        std::string passwd = body["passwd"].s();

        
        MYSQL *mysql = nullptr;
        // 构造 connectionRAII 之前已确保 pool 非空，防止直接崩溃
        connectionRAII mysqlcon(&mysql, pool);
        if (!mysql) {
            std::cerr << "ERROR: failed to obtain MYSQL* from pool\n";
            return crow::response(500, "db connection failed");
        }

        // 注意：示例直接拼接可能有 SQL 注入风险，生产需使用参数化/转义
        char sql_buf[512];
        snprintf(sql_buf, sizeof(sql_buf),
                 "INSERT INTO user(username, passwd) VALUES('%s','%s')",
                 name.c_str(), passwd.c_str());
        int res = mysql_query(mysql, sql_buf);
        if (res == 0)
            return crow::response(201, "created");
        else
            return crow::response(500, mysql_error(mysql));
    });

    app.port(18080).multithreaded().run();
    return 0;
}