#include <iostream>
#include <vector>
#include <string>
#include "common.h"

std::vector<std::string> splitString(std::string data, std::string delimiter)
{
    std::vector<std::string> results;
    size_t pos = 0;
    while ((pos = data.find(delimiter)) != std::string::npos)
    {
        std::string token;
        token = data.substr(0, pos);
        results.push_back(token);
        data.erase(0, pos + delimiter.length());
    }
    results.push_back(data);

    return results;
}

std::string getCmdResult(std::string cmd)
{
    std::string data;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");

    FILE* f = popen(cmd.c_str(), "r");
    if (f)
    {
        while (!feof(f))
            if (fgets(buffer, max_buffer, f) != NULL) {
                data.append(buffer);
            }
        pclose(f);
    }
    data.pop_back();
    return data;
}