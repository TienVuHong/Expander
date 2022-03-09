#include "../service/common/common.h"
#include "../config.h"
#include "Database.hpp"

DatabaseForward::DatabaseForward()
{
    std::string databaseEventPath = FOLDER_DATABASE + DATABASE_FORWARD_NAME;
    if (sqlite3_open(databaseEventPath.c_str(), &sqlForward))
        fprintf(stderr, "Can't open database event: %s\n", sqlite3_errmsg(sqlForward));

    int ret = 0;
    char *errMsg = 0;
    std::string cmd = "SELECT * from FORWARD_EVENT_TABLE;";

    ret = sqlite3_exec(sqlForward, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        createForwardTable();
    else
        std::cout << "Database FORWARD_EVENT_TABLE OK" << std::endl;
}

DatabaseForward::~DatabaseForward()
{
    sqlite3_close(sqlForward);
}

void DatabaseForward::createForwardTable()
{
    const char *cmd = "CREATE TABLE FORWARD_EVENT_TABLE("\
                        "ID     INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"\
                        "EVENT  TEXT    NOT NULL);";
    char *errMsg = NULL;
    int ret = sqlite3_exec(sqlForward, cmd, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "Create FORWARD_EVENT_TABLE: SQL error: " << errMsg << std::endl;
    else
        std::cout << "Create FORWARD_EVENT_TABLE OK" << std::endl;
    sqlite3_free(errMsg);
}

int DatabaseForward::push(json event)
{
    int ret = 0;
    char *errMsg = NULL;
    std::string cmd = "INSERT INTO FORWARD_EVENT_TABLE (EVENT) VALUES ('" + event.dump() + "');";
    ret = sqlite3_exec(sqlForward, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "Database Push Forward SQL error: " << errMsg << std::endl;
    else
        std::cout << "Database Push Forward OK" << std::endl;
    sqlite3_free(errMsg);
    return 1;
}

void DatabaseForward::resetEventCounter()
{
    int ret;
    char *errMsg;
    std::string cmd = "UPDATE `sqlite_sequence` SET `seq` = 0 WHERE `name` = 'FORWARD_EVENT_TABLE'";
    ret = sqlite3_exec(sqlForward, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "Reset Event Counter SQL error: " << errMsg << std::endl;
    sqlite3_free(errMsg);
}

int DatabaseForward::peak(json &event)
{
    sqlite3_stmt *stmt;
    std::string sql = "SELECT * FROM FORWARD_EVENT_TABLE";
    int error = sqlite3_prepare_v2(sqlForward, sql.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK) {
        return 0;
    }
    else {
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            event = json::parse((std::string((const char*) (sqlite3_column_text(stmt,1))))); // 0 is ID, 1 is event
        }
        if (sqlite3_step(stmt) == SQLITE_DONE){
            resetEventCounter();
        }
    }
    return 1;
}

int DatabaseForward::exist()
{
    sqlite3_stmt *stmt;
    std::string sql = "SELECT * FROM FORWARD_EVENT_TABLE";
    int error = sqlite3_prepare_v2(sqlForward, sql.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK){
        return 0;
    }
    if (sqlite3_step(stmt) != SQLITE_DONE){
        return 1;
    }
    return 0; 
}

int DatabaseForward::pop()
{
    std::string id;
    sqlite3_stmt *stmt;
    char *errMsg = NULL;
    std::string sql = "SELECT * FROM FORWARD_EVENT_TABLE";
    int error = sqlite3_prepare_v2(sqlForward, sql.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK) {
        return 0;
    }
    else {
        if (sqlite3_step(stmt) != SQLITE_DONE);
            id = (std::string((const char*) (sqlite3_column_text(stmt,0)))); // 0 is ID, 1 is event
    }

    sql = "DELETE FROM FORWARD_EVENT_TABLE where ID = '" + id + "'";
    error = sqlite3_exec(sqlForward, sql.c_str(), NULL, NULL, &errMsg);
    if (error != SQLITE_OK)
        return 0;
    return 1;
}
