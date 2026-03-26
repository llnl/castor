// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LOGGERFACTORY_H
#define LOGGERFACTORY_H

#include <iostream>

class Logger {
public:
    bool showError = true;
    bool showWarning = true;
    bool showInfo = false;

    Logger(const bool &showError, const bool &showWarning, const bool &showInfo) :
        showError(showError), showWarning(showWarning), showInfo(showInfo) {}

    virtual ~Logger() = default;

    virtual void error(std::string message) = 0;

    virtual void info(std::string message) = 0;

    virtual void warning(std::string message) = 0;
};

using LoggerPtr = std::shared_ptr<Logger>;

// TODO: implement
class TerminalLogger final: public Logger {
public:
    TerminalLogger(const bool &showError, const bool &showWarning, const bool &showInfo) :
        Logger(showError, showWarning, showInfo) {};
    ~TerminalLogger() override;
    void error(std::string message) override {
        if (showError) {
            std::cerr << "\033[1;31m" << std::endl << "\033[0m";
        }
    };
    void info(std::string message) override {
        if (showInfo) {
            std::cerr << "\033[1;32m" << std::endl << "\033[0m";
        }
    };
    void warning(std::string message) override {
        if (showWarning) {
            std::cerr << "\033[1;33m" << std::endl << "\033[0m";
        }
    };
};

class LoggerFactory {
public:
    static LoggerPtr logger;
    static LoggerPtr createLogger() {
        if (logger == nullptr) {
            logger = std::make_shared<TerminalLogger>(true, true, false);
        }
        return logger;
    };
};

#endif // LOGGERFACTORY_H
