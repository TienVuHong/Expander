#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <vector>
#include <string>

std::vector<std::string> splitString(std::string data, std::string delimiter);
std::string getCmdResult(std::string cmd);

#endif /* COMMON_H */