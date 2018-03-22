/* 
 * File:   Logger.cpp
 * Author: Jan Dufek
 */

#include "Logger.hpp"

Logger::Logger(string name) {

    general_log_file.open(name + ".txt");

}

Logger::Logger(const Logger& orig) {
}

Logger::~Logger() {
    close();
}

/**
 * Log string message into general log.
 * 
 * @param message
 */
void Logger::log_general(string message) {
    general_log_file << message;
}

/**
 * Log double value into general log.
 * 
 * @param message
 */
void Logger::log_general(double message) {
    general_log_file << message;
}

/**
 * Close log file.
 * 
 */
void Logger::close() {
    general_log_file.close();
}