#include "third_party/crow_all.h"
#include <string>
#include <iostream>
#include <mysql/mysql.h>
#include "CGImysql/sql_connection_pool.h" // 使用已有连接池接口

int main()
{
    crow::SimpleApp app;

    // 示例：GET /api/hello
    CROW_ROUTE(app, "/api/hello").methods("GET"_method)([](){
        crow::json::wvalue res;
        res["message"] = "hello from crow";
        return crow::response{200, res};
    });

    // 示例：POST /api/users  接收 JSON { "username": "...", "passwd": "..." }
    CROW_ROUTE(app, "/api/users").methods("POST"_method)([](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body)
            return crow::response(400, "invalid json");

        std::string name = body["username"].s();
        std::string passwd = body["passwd"].s();

        // 使用你工作区中的连接池获取 MySQL 连接
        MYSQL *mysql = nullptr;
        connection_pool *pool = connection_pool::GetInstance(); // [`connection_pool::GetInstance`](CGImysql/sql_connection_pool.h)
        connectionRAII mysqlcon(&mysql, pool);                   // [`connectionRAII`](CGImysql/sql_connection_pool.h)

        // 简单示例：插入并返回结果（请加防注入/校验）
        char sql_buf[256];
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