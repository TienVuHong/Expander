/*
* Authenticate.hpp
*
* Created by DuNTK on 11/01/2021.
* Copyright © 2021 BKAV. All rights reserved.
*/

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include "jwt/jwt.hpp"
#include "../json.h"

class Authenticate {
public:
    Authenticate();
    ~Authenticate();

    int state;
    std::vector<json> userList;

    std::string tokenGenerate(std::string id, std::string pass);
    int tokenDecode(std::string token);
    int loginAuthen(std::string token);
    int addUser(std::string user, std::string pass);
    int removeUser(std::string user, std::string pass);
    int changePassword(std::string user, std::string passOld, std::string passNew);

private:
    std::string key = "Expander2021";
};