/*
* Authenticate.cpp
*
* Created by DuNTK on 11/01/2021.
* Copyright © 2021 BKAV. All rights reserved.
*/

#include "Authenticate.hpp"
using namespace jwt::params;

Authenticate::Authenticate() { 
    this->state = 0;
    if(this->userList.empty() == true) {
        json jUserDefault = {
            {"user" , "bkav"},
            {"password", "Bkav@2021"}
        };
        this->userList.push_back(jUserDefault);
    }
}

Authenticate::~Authenticate() {}

std::string Authenticate::tokenGenerate(std::string id, std::string pass) {
    jwt::jwt_object obj{algorithm("HS256"), 
                        payload({{"user", id}, {"password", pass}}),
                        secret(this->key)};
    // obj.add_claim("exp", std::chrono::system_clock::now() + std::chrono::seconds{3600*12});
    std::string token = obj.signature();
    return token;
}

int Authenticate::loginAuthen(std::string token) {
    std::error_code ec;
    auto dec_obj = jwt::decode(token, algorithms({"HS256"}), ec,
                                secret(this->key), verify(true));
    // if (ec.value() != 0) {
    //     std::cerr << ec.category().name() << " - " << ec.value() << std::endl;
    // }
    // } catch (const jwt::TokenExpiredError& e) {
    //     // Handle Token expired exception here
    //     std::cerr << "Token expired exception" << std::endl;
    // } catch (const jwt::SignatureFormatError& e) {
    //     // Handle invalid signature format error
    //     std::cerr << "Invalid signature format error" << std::endl;
    // } catch (const jwt::DecodeError& e) {
    //     // Handle all kinds of other decode errors
    //     std::cerr << "Decode error" << std::endl;
    // } catch (const jwt::VerificationError& e) {
    //     // Handle the base verification error.
    //     std::cerr << "Verification error" << std::endl;
    // } catch (...) {
    //     std::cerr << "Caught unknown exception" << std::endl;
    // }
    return ec.value();
}

int Authenticate::addUser(std::string user, std::string pass) {
    int re;
    json jTemp;
    jTemp["user"] = user;
    jTemp["password"] = pass;
    for (int i = 0; i < this->userList.size(); i++) {
        if (this->userList[i]["user"] == jTemp["user"]) {
            re = 0;
            break;
        } else
            re = 1;
    }
    if(re) {
        this->userList.push_back(jTemp);
    }
    return re;
}

int Authenticate::removeUser(std::string user, std::string pass) {
    int re;
    json jTemp;
    jTemp["user"] = user;
    jTemp["password"] = pass;
    for (int i = 0; i < this->userList.size(); i++) {
        if (this->userList[i]["user"] == jTemp["user"] &&
            this->userList[i]["password"] == jTemp["password"]) {
            this->userList.erase(this->userList.begin() + i);
            re = 0;
            break;
        } else
            re = 1;
    }
    return re;
}

int Authenticate::changePassword(std::string user, std::string passOld, std::string passNew) {
    int re;
    json jTemp;
    jTemp["user"] = user;
    jTemp["password"] = passOld;
    for (int i = 0; i < this->userList.size(); i++) {
        if (this->userList[i]["user"] == jTemp["user"] &&
            this->userList[i]["password"] == jTemp["password"]) {
            this->userList[i]["password"] = passNew;
            re = 1;
            break;
        } else
            re = 0;
    }
    return re;
}