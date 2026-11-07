#include "testHelpers.h"

#include <iostream>

TEST( LogTests, InfoWritesAnInfoMessage )
{
	LogCapture log;

	GAIA_INFO( "hello" );

	EXPECT_TRUE( log.Contains( "info: hello" ) );
}

TEST( LogTests, WarnWritesAWarningMessage )
{
	LogCapture log;

	GAIA_WARN( "careful" );

	EXPECT_TRUE( log.Contains( "warning: careful" ) );
}

TEST( LogTests, ErrorWritesAnErrorMessage )
{
	LogCapture log;

	GAIA_ERROR( "broken" );

	EXPECT_TRUE( log.Contains( "error: broken" ) );
}

TEST( LogTests, MessagesFormatTheirArguments )
{
	LogCapture        log;
	const std::string name = "cube";

	GAIA_INFO( "Loaded {} with {} faces", name, 6 );

	EXPECT_TRUE( log.Contains( "Loaded cube with 6 faces" ) );
}

TEST( LogTests, PassingAssertWritesNothing )
{
	LogCapture log;

	GAIA_ASSERT( 1 + 1 == 2 );

	EXPECT_TRUE( log.GetOutput().empty() );
}

TEST( LogTests, GetLoggerAlwaysReturnsTheSameLogger )
{
	EXPECT_EQ( &Log::GetLogger(), &Log::GetLogger() );
}

TEST( LogDeathTests, FailedAssertLogsACriticalMessageAndStops )
{
	// Note: Death tests only read stderr, while the engine logger writes to stdout
	auto failAssert = []() {
		Log::GetLogger().sinks().push_back( new_shared<spdlog::sinks::ostream_sink_mt>( std::cerr, true ) );
		GAIA_ASSERT( 1 + 1 == 3 );
	};

	EXPECT_DEATH( failAssert(), "Assertion failed: 1 \\+ 1 == 3 \\(.*log_tests\\.cpp:\\d+\\)" );
}
