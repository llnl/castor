// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef __MESSAGING
#define __MESSAGING

#include <Sawyer/Message.h>
#include <Rose/Diagnostics.h>
#include <string>
#include "settings.hxx"

enum class LogType { INFO, FATAL, WARN, ERROR };

extern Settings settings;
extern Sawyer::Message::Facility mlog;

inline void log(std::string message, LogType logType, int debugLevel)
{
	if (settings.debug)
	{
		// Disable certain unnecessary logging information
		Rose::Diagnostics::mprefix->showProgramName(true);
		Rose::Diagnostics::mprefix->showThreadId(true);
		Rose::Diagnostics::mprefix->showElapsedTime(true);
	}
	else
	{
		// Disable certain unnecessary logging information
		Rose::Diagnostics::mprefix->showProgramName(false);
		Rose::Diagnostics::mprefix->showThreadId(false);
		Rose::Diagnostics::mprefix->showElapsedTime(false);
	}

	if (settings.debug >= debugLevel) switch (logType)
	{
		case LogType::INFO:
			mlog[Sawyer::Message::Common::INFO] << message << std::endl;
			break;
		case LogType::FATAL:
			mlog[Sawyer::Message::Common::FATAL] << message << std::endl;
			break;
		case LogType::WARN:
			mlog[Sawyer::Message::Common::WARN] << message << std::endl;
			break;
		case LogType::ERROR:
			mlog[Sawyer::Message::Common::ERROR] << message << std::endl;
			break;
	}
}

#endif
