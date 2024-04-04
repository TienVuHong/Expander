/*
 * Database.cpp
 *
 *  Created on: 24/05/2021
 *      Author: tienvh
 * 
 */

#include <fstream>
#include "service/json/json.hpp"
#include "service/common/common.hpp"
#include "service/base64/base64.hpp"
#include <sqlite3.h>
#include "config.h"
#include "Database.hpp"

Database::Database()
{
    int ret = 0;
    char *errMsg = 0;
    std::string cmd;

    std::string databasePath = FOLDER_DATABASE + DATABASE_NAME;
    if(sqlite3_open(databasePath.c_str(), &db)){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Check USER
    cmd = "SELECT * from USER;";
    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        createUserTable();
    else
        std::cout << "Database USER OK" << std::endl;

    // Check PROFILE_TABLE
    for (int channel = 1; channel <= MAX_CHANNEL; channel++)
    {
        cmd = "SELECT * from PROFILE_TABLE_CHANNEL_" + std::to_string(channel) + ";";
        ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
        if (ret != SQLITE_OK)
            createProfileTable(channel);
        else 
            std::cout << "Database PROFILE_TABLE_CHANNEL_" << channel << " OK" << std::endl;
    }

    // Check SERVERTABLE
    cmd = "SELECT * from SERVER_TABLE;";
    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        createServerTable();
    else
        std::cout << "Database SERVER_TABLE OK" << std::endl;
    
    // Check CONFIG_TABLE
    cmd = "SELECT * from CONFIG_TABLE;";
    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        createConfigTable();
    else
        std::cout << "Database CONFIG_TABLE OK" << std::endl;

    // Check DEVICE_CONFIG_TABLE
    cmd = "SELECT * from DEVICE_CONFIG_TABLE;";
    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        createDeviceConfig();
    else
        std::cout << "Database DEVICE_CONFIG_TABLE OK" << std::endl;

    sqlite3_free(errMsg);
}

Database::~Database()
{
    sqlite3_close(db);
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   printf("argc = %d\n", argc);
   for(i = 0; i<argc; i++)
   {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

//=================================================================================================================================================== USER
void Database::createUserTable()
{
    const char *cmd = "CREATE TABLE USER("\
                "USERID     TEXT    NOT NULL,"\
                "PASSWORD   TEXT    NOT NULL);";
    char *errMsg = 0;
    int ret = sqlite3_exec(db, cmd, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "DatabaseCreateUserTable: SQL error: " << errMsg << std::endl;
    else 
        std::cout << "Create USER table OK" << std::endl;
    addUser("bkav", "Bkav@2021");
    sqlite3_free(errMsg);
}

int Database::addUser(std::string userid, std::string password)
{
    if (hasUserExisted(userid))
        return 0;

    int ret = 0;
    char *errMsg = NULL;
    std::string cmd = "INSERT INTO USER (USERID, PASSWORD) VALUES ('" + userid + "', '" + password + "');";

    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "DatabaseAddUser SQL error: " << errMsg << std::endl;
    sqlite3_free(errMsg); 
    return (ret == SQLITE_OK);
}

int Database::removeUser(std::string userid)
{
    if (!hasUserExisted(userid))
        return 0;

    int ret = 0;
    char *errMsg = 0;
    std::string cmd = "DELETE from USER where USERID = '" + userid + "';";

    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "DatabaseRemoveUser SQL error: " << errMsg << std::endl; 
    sqlite3_free(errMsg);
    return (ret == SQLITE_OK);
}

int Database::changePassword(std::string userid, std::string passwordOld, std::string passwordNew)
{
    if (!checkLoginInfo(userid, passwordOld))
        return 0;

    int ret = 0;
    char *errMsg = NULL;
    std::string cmd = "UPDATE USER set PASSWORD = '" + passwordNew + "' where USERID = '" + userid + "';";

    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "DatabaseChangePassword SQL error: " << errMsg << std::endl; 
    sqlite3_free(errMsg);
    return (ret == SQLITE_OK);
}

bool Database::hasUserExisted(std::string userid)
{
    int ret = 0;
    sqlite3_stmt *stmt;
    std::string cmd = "SELECT USERID from USER WHERE USERID = '" + userid + "';";
    int error = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK) 
    {
        ret = false;
    }
    else 
    {
        while (sqlite3_step(stmt) != SQLITE_DONE) {
            if (userid == std::string((const char*) (sqlite3_column_text(stmt,0))))
                ret = true;
        }
    }
    sqlite3_finalize(stmt);
    return ret;
}

bool Database::checkLoginInfo(std::string userid, std::string password)
{
    bool ret = false;
    sqlite3_stmt *stmt;
    std::string cmd = "SELECT PASSWORD from USER WHERE USERID = '" + userid + "';";
    int error = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK) 
    {
        ret = false;
    } 
    else 
    {
        while (sqlite3_step(stmt) != SQLITE_DONE) {
            if (password == std::string((const char*) (sqlite3_column_text(stmt,0))))
                ret = true;
        }
    }

    printf("DatabaseCheckLoginInfo: %s\n", ret ? "OK" : "WRONG PASS!!!!");
    sqlite3_finalize(stmt);
    return ret;
}

std::vector<std::string> Database::loadUser()
{
    std::vector<std::string> users;
    sqlite3_stmt *stmt;
    std::string sql = "SELECT USERID FROM USER;";
    int error = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK) {
        return users;
    } 
    else {
        while (sqlite3_step(stmt) != SQLITE_DONE) {
            std::string user = std::string((const char*) (sqlite3_column_text(stmt,0)));
            users.push_back(user);
        }
        return users;
    }
}
//============================================================================================================================================== END USER



//========================================================================================================================================= PROFILE TABLE

/**
 * function to decode base64 and save into an image file using the given pathToFolder and fileName (increasingly)
    @return path to recently created image file
*/
void imageToFile(std::string strImage, std::string filePath)
{
    /* data:image/jpeg;base64,/9j/4AAQSkZJR.....    ---> split ","
    |- data:image/jpeg;base64                       ---> split ";"
    |   |- data:image/jpeg                          ---> split "/"
    |   |   |- data:image
    |   |   |
    |   |   |- jpeg                                 ---> fileType
    |   |- base64
    |- /9j/4AAQSkZJR.....                           ---> Image
    */
    std::vector<std::string> vtc = splitString(strImage, ",");
    std::vector<std::string> vtc1 = splitString(vtc[0], ";");
    std::vector<std::string> vtc2 = splitString(vtc1[0], "/");
    std::string base64Image = vtc[1];
    std::string fileType = vtc2[1];

    std::ofstream outfile(filePath, std::ofstream::binary | std::ofstream::trunc);
    std::string imageDecode = base64_decode(base64Image, true);
    outfile.write(imageDecode.c_str(), imageDecode.length());
    outfile.close();
}

/**
 * function create folder to save all profileImages into a folder which is named after profileid
 * @ profileid: profile name
 * @ jProfileImages: json array which store all images in base64 format
 * @ return: json array which store all path to each image file after converting and saving to folder
*/
std::string createProfileImageFolder(std::string profileid, std::string profileImages)
{
    std::string folderPath = FOLDER_PROFILES + profileid + "/";
    std::string folderWebReadPath = PATH_SAVE_PROFILES + profileid + "/";
    std::string cmd = std::string("mkdir -v ") + folderPath;
    system(cmd.c_str());

    std::string fileName, imagePath, webReadPath;
    json jProfileImages = json::parse(profileImages);
    json jResult = json::array();
    int i = 0;
    for (std::string strImage : jProfileImages)
    {
        fileName    = profileid + std::to_string(++i);
        imagePath   = folderPath + fileName;
        imageToFile(strImage, imagePath);
        webReadPath = folderWebReadPath + fileName;
        jResult.push_back(webReadPath);
    }
    return jResult.dump();
}

void deleteProfileImageFolder(std::string profileid)
{
    std::string cmd = std::string("rm -rf -v ") + FOLDER_PROFILES + profileid + "/";
    system(cmd.c_str());
}

void Database::createProfileTable(int channel)
{
    std::string cmd =   "CREATE TABLE PROFILE_TABLE_CHANNEL_" + std::to_string(channel) + "("\
                        "PROFILEID  TEXT    NOT NULL,"\
                        "SCHEDULE   TEXT    NOT NULL,"\
                        "IMAGES     TEXT    NOT NULL,"\
                        "STATUS     INT     NOT NULL);";
    char *errMsg = 0;
    int ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "Create PROFILE_TABLE_CHANNEL_" << channel << ": SQL error: " << errMsg << std::endl;
    else 
        std::cout << "Create PROFILE_TABLE_CHANNEL_" << channel << " OK" << std::endl;
    sqlite3_free(errMsg);
}

int Database::hasProfileExisted(std::string profileid, int channel)
{
    int ret = 0;
    sqlite3_stmt *stmt;
    std::string cmd = "SELECT PROFILEID from PROFILE_TABLE_CHANNEL_" + std::to_string(channel) + " WHERE PROFILEID = '" + profileid + "';";
    int error = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK)
    {
        ret = false;
    }
    else
    {
        while (sqlite3_step(stmt) != SQLITE_DONE) {
            if (profileid == std::string((const char*) (sqlite3_column_text(stmt,0))))
                ret = true;
        }
    }
    sqlite3_finalize(stmt);
    return ret;
}

int Database::addProfile(profile_t profile, int channel)
{
    if (hasProfileExisted(profile.profileid, channel))
        return 0;

    int ret = 0;
    char *errMsg = NULL;
    std::string imagePath = createProfileImageFolder(profile.profileid, profile.images);
    std::string cmd = "INSERT INTO PROFILE_TABLE_CHANNEL_" + std::to_string(channel) + 
        " (PROFILEID, SCHEDULE, IMAGES, STATUS) VALUES ('" + 
        profile.profileid + "', '" + 
        profile.schedule + "', '" + 
        imagePath + "'," + 
        std::to_string(profile.status) + ");";

    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "DatabaseAddProfile SQL error: " << errMsg << std::endl;
    sqlite3_free(errMsg);
    return (ret == SQLITE_OK);
}

int Database::removeProfile(std::string profileid, int channel)
{
    if (!hasProfileExisted(profileid, channel))
        return 0;

    int ret = 0;
    char *errMsg = 0;
    deleteProfileImageFolder(profileid);
    std::string cmd = "DELETE from PROFILE_TABLE_CHANNEL_" + std::to_string(channel) + " where PROFILEID = '" + profileid + "'";

    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "DatabaseRemoveProfile SQL error: " << errMsg << std::endl; 
    sqlite3_free(errMsg);
    return (ret == SQLITE_OK);
}

int Database::changeProfileSchedule(profile_t profile, int channel)
{
    if (!hasProfileExisted(profile.profileid, channel))
        return 0;
    
    int ret = 0;
    char *errMsg = NULL;
    std::cout << "profileid: " << profile.profileid << std::endl;
    std::cout << "schedule: " << profile.schedule << std::endl;
    std::string cmd = "update PROFILE_TABLE_CHANNEL_" + std::to_string(channel) + 
        " set SCHEDULE = '" + profile.schedule + "'" +
        " where PROFILEID = '" + profile.profileid + "';";

    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "DatabaseChangeProfileSchedule SQL error: " << errMsg << std::endl; 
    sqlite3_free(errMsg);
    return (ret == SQLITE_OK);
}

json Database::getProfileSchedule(std::string profileid, int channel)
{
    json jSchedule;
    if (!hasProfileExisted(profileid, channel))
        return jSchedule;

    bool ret = false;
    sqlite3_stmt *stmt;
    std::string str;
    std::string cmd = "select SCHEDULE from PROFILE_TABLE_CHANNEL_" + std::to_string(channel) + 
        " where PROFILEID = '" + profileid + "';";

    int error = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK)
    {
        ret = false;
    }
    else
    {
        while (sqlite3_step(stmt) != SQLITE_DONE) {
            jSchedule = json::parse(std::string((const char*) (sqlite3_column_text(stmt,0))));
            ret = true;
        }
    }
    sqlite3_finalize(stmt);
    return jSchedule;
}

int Database::changeProfileStatus(profile_t profile, int channel)
{
    if (!hasProfileExisted(profile.profileid, channel))
        return 0;
    
    int ret = 0;
    char *errMsg = NULL;
    std::cout << "profileid: " << profile.profileid << std::endl;
    std::cout << "status: " << profile.status << std::endl;
    std::string cmd = "update PROFILE_TABLE_CHANNEL_" + std::to_string(channel) + 
        " set STATUS = " + std::to_string(profile.status) +
        " where PROFILEID = '" + profile.profileid + "';";

    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "DatabaseChangeProfileStatus SQL error: " << errMsg << std::endl;
    sqlite3_free(errMsg);

    createProfileTable(channel);
    return (ret == SQLITE_OK);
}

int Database::getProfileStatus(std::string profileid, int channel)
{
    int status = -1;
    if (!hasProfileExisted(profileid, channel))
        return -1;

    bool ret = false;
    sqlite3_stmt *stmt;
    std::string str;
    std::string cmd = "select STATUS from PROFILE_TABLE_CHANNEL_" + std::to_string(channel) + 
        " where PROFILEID = '" + profileid + "';";

    int error = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK)
    {
        ret = false;
    }
    else
    {
        while (sqlite3_step(stmt) != SQLITE_DONE) {
            status = stoi(std::string((const char*) (sqlite3_column_text(stmt,0))));
            ret = true;
        }
    }
    sqlite3_finalize(stmt);
    return status;
}

json Database::getAllProfile(int channel)
{
    json jProfiles = json::array();
    json jTemp;
    sqlite3_stmt *stmt;
    std::string sql = "SELECT * FROM PROFILE_TABLE_CHANNEL_" + std::to_string(channel);
    int error = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK) {
        return jProfiles;
    } 
    else {
        while (sqlite3_step(stmt) != SQLITE_DONE) {
            jTemp["profileid"]  =            (std::string((const char*) (sqlite3_column_text(stmt,0))));
            jTemp["schedule"]   = json::parse(std::string((const char*) (sqlite3_column_text(stmt,1))));
            jTemp["images"]     = json::parse(std::string((const char*) (sqlite3_column_text(stmt,2))));
            jTemp["status"]     =        stoi(std::string((const char*) (sqlite3_column_text(stmt,3))));
            jProfiles.push_back(jTemp);
        }
    }
    return jProfiles;
}

int Database::deleteAllProfile(int channel)
{
    int ret = 0;
    char *errMsg = NULL;
    std::string sql = "DROP TABLE IF EXISTS PROFILE_TABLE_CHANNEL_" + std::to_string(channel);
    ret = sqlite3_exec(db, sql.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "DeleteAllProfile channel" << channel << " SQL error: " << errMsg << std::endl; 
    sqlite3_free(errMsg);
    createProfileTable(channel);
    return (ret == SQLITE_OK);
}
//===================================================================================================================================== END PROFILE TABLE

//=========================================================================================================================================== SERVERTABLE
void Database::createServerTable()
{
    const char *cmd = "CREATE TABLE SERVER_TABLE("\
                "NAME   TEXT    NOT NULL,"\
                "USE    TEXT    NOT NULL,"\
                "URL    TEXT    NOT NULL,"\
                "TOKEN  TEXT    NOT NULL);";
    char *errMsg = NULL;
    int ret = sqlite3_exec(db, cmd, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "Create SERVER_TABLE: SQL error: " << errMsg << std::endl; 
    else 
        std::cout << "Create SERVER_TABLE OK" << std::endl;
    sqlite3_free(errMsg);

    std::string cmdInsertDefault = "INSERT INTO SERVER_TABLE (NAME, USE, URL, TOKEN) VALUES ('EMS', 'false', '', '');";

    ret = sqlite3_exec(db, cmdInsertDefault.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "Add default serverIP Error" << errMsg << std::endl;
    else
        std::cout << "Add default serverIP OK" << std::endl;
    sqlite3_free(errMsg);
}

int Database::changeServer(server_t server)
{
    int ret = 0;
    char *errMsg = NULL;

    std::string cmd = "UPDATE SERVER_TABLE set (USE, URL, TOKEN) = ('" + 
        std::string((server.use ? "true" : "false")) + "', '" + 
        server.url + "', '" + 
        server.token + "');";
    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "DatabaseChangeServer SQL error: " << errMsg << std::endl;
    sqlite3_free(errMsg);
    return (ret == SQLITE_OK);    
}

server_t Database::getServer()
{
    int error = 0;
    std::string str;
    server_t server = {false, "" , ""};
    sqlite3_stmt *stmt;
    // std::string cmd = "SELECT URL from SERVER_TABLE WHERE NAME = 'EMS';";
    std::string cmd = "SELECT * from SERVER_TABLE;";
    error = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK)
        return server;
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        server.use      = std::string((const char*) (sqlite3_column_text(stmt,1))) == "true" ? true : false;
        server.url      = std::string((const char*) (sqlite3_column_text(stmt,2)));
        server.token    = std::string((const char*) (sqlite3_column_text(stmt,3)));
    }
    if (error != SQLITE_OK)
        std::cout << "DatabaseGetServer SQL error: " << std::endl;
    sqlite3_finalize(stmt);
    return server;
}
//======================================================================================================================================= END SERVERTABLE


//========================================================================================================================================== CONFIG_TABLE
void Database::createConfigTable()
{
    const char *cmd = "CREATE TABLE CONFIG_TABLE("\
                    "CHANNEL    INT    NOT NULL,"\
                    "TIME_OPEN  INT    NOT NULL,"\
                    "TYPE_EVENT INT    NOT NULL,"\
                    "CAMERA_IP  TEXT    NOT NULL);";
    char *errMsg = NULL;
    int ret = sqlite3_exec(db, cmd, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "Create CONFIG_TABLE: SQL error: " << errMsg << std::endl; 
    else 
        std::cout << "Create CONFIG_TABLE OK" << std::endl;

    std::string sql;
    for (int channel = 1; channel <= MAX_CHANNEL; channel++)
    {
        sql = "INSERT INTO CONFIG_TABLE (CHANNEL, TIME_OPEN, TYPE_EVENT, CAMERA_IP) VALUES (" + std::to_string(channel) + ", 1, 1, '[]');";
        ret = sqlite3_exec(db, sql.c_str(), NULL, NULL, &errMsg);
        if (ret != SQLITE_OK)
            std::cout << "Add Default Config Channel" << channel << "  SQL error: " << errMsg << std::endl;
        else
            std::cout << "Add Default Config Channel" << channel << " OK" << std::endl;
    }

    sqlite3_free(errMsg);
}

int Database::changeTimeOpen(int channel, int second)
{
    int ret = 0;
    char *errMsg = NULL;

    std::string cmd = "UPDATE CONFIG_TABLE set TIME_OPEN = " + std::to_string(second) + " where CHANNEL = " + std::to_string(channel) + ";";
    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "changeTimeOpenChannel" << channel << " SQL error: " << errMsg << std::endl;
    else
        std::cout << "changeTimeOpenChannel" << channel << ", time: " << std::to_string(second) << " OK" << std::endl;

    sqlite3_free(errMsg);
    return (ret == SQLITE_OK);
}

int Database::getTimeOpen(int channel)
{
    int error = 0;
    std::string resultString;
    int timeOpen = 3;
    sqlite3_stmt *stmt;
    std::string cmd = "SELECT TIME_OPEN from CONFIG_TABLE where CHANNEL = " + std::to_string(channel) + ";";
    error = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK)
        return -1;
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        resultString = std::string((const char*) (sqlite3_column_text(stmt,0)));
    }
    timeOpen = stoi(resultString);
    sqlite3_finalize(stmt);
    return timeOpen;
}

int Database::changeTypeEvent(int channel, int typeEvent)
{
    int ret = 0;
    char *errMsg = NULL;

    std::string cmd = "UPDATE CONFIG_TABLE set TYPE_EVENT = " + std::to_string(typeEvent) + " where CHANNEL = " + std::to_string(channel) + ";";
    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "changeTypeEventChannel" << channel << " SQL error: " << errMsg << std::endl;
    else
        std::cout << "changeTypeEventChannel" << channel << ", type: " << std::to_string(typeEvent) << " OK" << std::endl;

    sqlite3_free(errMsg);
    return (ret == SQLITE_OK);
}

int Database::getTypeEvent(int channel)
{
    int error = 0;
    int typeEvent = 0;
    sqlite3_stmt *stmt;
    std::string cmd = "SELECT TYPE_EVENT from CONFIG_TABLE where CHANNEL = " + std::to_string(channel) + ";";
    error = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK)
        return -1;
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        typeEvent = stoi(std::string((const char*) (sqlite3_column_text(stmt,0))));
    }
    sqlite3_finalize(stmt);
    return typeEvent;
}

int Database::changeCameraIp(int channel, std::string cameraIp)
{
    int ret = 0;
    char *errMsg = NULL;

    std::string cmd = "UPDATE CONFIG_TABLE set CAMERA_IP = '" + cameraIp + "' where CHANNEL = " + std::to_string(channel) + ";";
    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "changeCameraIpChannel" << channel << " SQL error: " << errMsg << std::endl;
    else
        std::cout << "changeCameraIpChannel" << channel << ", Value: " << cameraIp << " OK" << std::endl;

    sqlite3_free(errMsg);
    return (ret == SQLITE_OK);
}

int Database::getCameraIp(int channel, std::string &cameraIp)
{
    int error = 0;
    sqlite3_stmt *stmt;
    std::string cmd = "SELECT CAMERA_IP from CONFIG_TABLE where CHANNEL = " + std::to_string(channel) + ";";
    error = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK)
        return -1;
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        cameraIp = std::string((const char*) (sqlite3_column_text(stmt,0)));
    }
    printf("getCameraIpChannel%d: %s %s\n", channel, cameraIp.c_str(), (error == SQLITE_OK) ? "OK" : "Failed");
    sqlite3_finalize(stmt);
    return 1;
}
//========================================================================================================================================== CONFIG_TABLE

//=================================================================================================================================== DEVICE_CONFIG_TABLE
void Database::createDeviceConfig()
{
    const char *cmd = "CREATE TABLE DEVICE_CONFIG_TABLE("\
                    "PARAM  TEXT    NOT NULL,"\
                    "VALUE  INT     NOT NULL);";
    char *errMsg = NULL;
    int ret = sqlite3_exec(db, cmd, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "Create DEVICE_CONFIG_TABLE: SQL error: " << errMsg << std::endl;
    else
        std::cout << "Create DEVICE_CONFIG_TABLE OK" << std::endl;
    
    std::string sql = "INSERT INTO DEVICE_CONFIG_TABLE (PARAM, VALUE) VALUES ('MODE', 0);";
    ret = sqlite3_exec(db, sql.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "insert MODE SQL error: " << errMsg << std::endl;

    sqlite3_free(errMsg);
}

int Database::changeMode(int mode)
{
    int ret = 0;
    char *errMsg = NULL;
    std::string cmd = "UPDATE DEVICE_CONFIG_TABLE set VALUE = " + std::to_string(mode) + " where PARAM = 'MODE';";
    ret = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
        std::cout << "changeMode SQL error: " << errMsg << std::endl;
    else
        std::cout << "changeMode, value: " << mode << " OK" << std::endl;

    sqlite3_free(errMsg);
    return (ret == SQLITE_OK);
}

int Database::getMode()
{
    int syncMode = 0;
    int error = 0;
    sqlite3_stmt *stmt;
    std::string cmd = "SELECT VALUE from DEVICE_CONFIG_TABLE where PARAM = 'MODE';";
    error = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    if (error != SQLITE_OK){
        std::cout << "getMode error" << std::endl;
        return -1;
    }
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        syncMode = stoi(std::string((const char*) (sqlite3_column_text(stmt,0))));
    }
    sqlite3_finalize(stmt);
    return syncMode;
}