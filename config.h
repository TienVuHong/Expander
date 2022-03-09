#ifndef DEFINITION_H
#define DEFINITION_H

#define SOFTWARE_VERSION "1.1.4.6"
#define HARDWARE_VERSION "1.0.1"

#ifdef RASPBERRY_PI
#define WORKSPACE               std::string("/var/www/html/")
#define FOLDER_AIVEX            std::string(WORKSPACE + "AIVEX/")
#define FOLDER_LOG              std::string(WORKSPACE + "AIVEX/Log/")
#define FOLDER_DATABASE         std::string(WORKSPACE + "AIVEX/Database/")
#define FOLDER_PROFILES         std::string(WORKSPACE + "AIVEX/Database/Profiles/")
#define FOLDER_IMAGES           std::string(WORKSPACE + "AIVEX/Database/Images/")
#define PATH_SAVE_PROFILES      std::string("AIVEX/Database/Profiles/") //for API to webUI
#define PATH_SAVE_IMAGES        std::string("AIVEX/Database/Images/")   //for API to webUI
#else
#define WORKSPACE               std::string("/media/tienvh/Workspace/1.AIView/AIVEX/source/trunk/build/")
#define FOLDER_AIVEX            std::string(WORKSPACE + "AIVEX/")
#define FOLDER_LOG              std::string(WORKSPACE + "AIVEX/Log/")
#define FOLDER_DATABASE         std::string(WORKSPACE + "AIVEX/Database/")
#define FOLDER_PROFILES         std::string(WORKSPACE + "AIVEX/Database/Profiles/")
#define FOLDER_IMAGES           std::string(WORKSPACE + "AIVEX/Database/Images/")
#define PATH_SAVE_PROFILES      std::string(WORKSPACE + "AIVEX/Database/Profiles/") //for API to webUI
#define PATH_SAVE_IMAGES        std::string(WORKSPACE + "AIVEX/Database/Images/")   //for API to webUI
#endif /* RASPBERRY_PI */

#define DATABASE_NAME           std::string("Expander_v1142.db")
#define DATABASE_EVENT_NAME     std::string("Event_v1144.db")
#define DATABASE_FORWARD_NAME   std::string("Forward_v1143.db")

#define IMAGE_NAME(x)            std::string(std::string(x) + ".jpeg")
#define VIDEO_NAME(x)            std::string(std::string(x) + ".mp4")

#endif /* DEFINITION_H */