#include "gaia3_pch.h"
#include "log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

spdlog::logger& Log::GetLogger()
{
	// Note: Created on first use so logging works before App::Init and in the tests
	static shared<spdlog::logger> logger = []() {
		shared<spdlog::logger> newLogger = spdlog::stdout_color_mt( "gaia3" );
		newLogger->set_pattern( "[%T] [%^%l%$] %v" );
		newLogger->flush_on( spdlog::level::err );
		return newLogger;
	}();

	return *logger;
}
