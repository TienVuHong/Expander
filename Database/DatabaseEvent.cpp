#include "../service/common/common.h"
#include "../config.h"
#include "Database.hpp"

DatabaseEvent::DatabaseEvent()
{
    std::string databaseEventPath = FOLDER_DATABASE + DATABASE_EVENT_NAME;
    if (sqlite3_open(databaseEventPath.c_str(), &sqlEvent))
        fprintf(stderr, "Can't open database event: %s\n", sqlite3_errmsg(sqlEvent));

    int ret = 0;
    char *errMsg = 0;
    std::string cmd = "SELECT * from EVENT_TABLE;";

    ret = sqlite3_exec(sqlEvent, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        createEventTable();
    else
        std::cout << "Database EVENT_TABLE OK" << std::endl;
}

DatabaseEvent::~DatabaseEvent()
{
    sqlite3_close(sqlEvent);
}

void DatabaseEvent::createEventTable()
{
    const char *cmd = "CREATE TABLE EVENT_TABLE("\
                    "ID         INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"\
                    "EVENT      TEXT    NOT NULL,"\
                    "DATE       TEXT    NOT NULL,"\
                    "TIME       TEXT    NOT NULL,"\
                    "TYPE       TEXT    NOT NULL,"\
                    "CAMERA     TEXT    NOT NULL,"\
                    "CONTENT    TEXT    NOT NULL);";
    char *errMsg = NULL;
    int ret = sqlite3_exec(sqlEvent, cmd, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "Create EVENT_TABLE: SQL error: " << errMsg << std::endl;
    else
        std::cout << "Create EVENT_TABLE OK" << std::endl;
    sqlite3_free(errMsg);
}

int DatabaseEvent::push(json event)
{
    int ret = 0;
    char *errMsg = NULL;

    std::string cmd = "INSERT INTO EVENT_TABLE (EVENT, DATE, TIME, TYPE, CAMERA, CONTENT) VALUES ('" + event.dump() + "','" + 
                                                                                            std::string(event["date"]) + "','" +
                                                                                            std::string(event["time"]) + "','" +
                                                                                            std::string(event["type"]) + "','" +
                                                                                            std::string(event["camera"]) + "','" +
                                                                                            std::string(event["content"]) + "');";
    ret = sqlite3_exec(sqlEvent, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "Database Push Event SQL error: " << errMsg << std::endl;
    else
        std::cout << "Database Push Event OK" << std::endl;
    sqlite3_free(errMsg);
    return 1;
}

int DatabaseEvent::get(json &event, json condition)
{
    sqlite3_stmt *stmt;
    event.clear();
    event = json::array();
    bool isFirst = true;
    json jResult;
    std::string sql = "SELECT (EVENT) FROM EVENT_TABLE ";
    if (condition.contains("date") && condition["date"].is_string()) {
        if (isFirst)
            sql += "where ( ";
        else
            sql += "AND ";
        sql += "DATE = '" + std::string(condition["date"]) + "' ";
        isFirst = false;

        if (condition.contains("time") && condition["time"].is_string()) {
            sql += "AND ";
            sql += "(TIME = '" + std::string(condition["time"]) + "' OR TIME > '" + std::string(condition["time"]) + "') ";
        }
    }
    if (condition.contains("type") && condition["type"].is_string()) {
        if (isFirst)
            sql += "where ( ";
        else
            sql += "AND ";
        sql += "TYPE = '" + std::string(condition["type"]) + "' ";
        isFirst = false;
    }
    if (condition.contains("camera") && condition["camera"].is_string()) {
        if (isFirst)
            sql += "where ( ";
        else
            sql += "AND ";
        sql += "CAMERA = '" + std::string(condition["camera"]) + "' ";
        isFirst = false;
    }
    if (condition.contains("content") && condition["content"].is_string()) {
        if (isFirst)
            sql += "where ( ";
        else
            sql += "AND ";
        sql += "CONTENT = '" + std::string(condition["content"]) + "' ";
        isFirst = false;
    }
    if (!isFirst) {
        sql += ");";
    }

    int error = sqlite3_prepare_v2(sqlEvent, sql.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK) {
        return 0;
    }
    else {
        while(sqlite3_step(stmt) != SQLITE_DONE)
        {
            event.push_back(json::parse(std::string((const char*) (sqlite3_column_text(stmt,0)))));
        }
    }
    return 1;
}
