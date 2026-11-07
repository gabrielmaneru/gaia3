#pragma once

#include <gtest/gtest.h>

#include <core/log.h>
#include <core/types.h>

#include <spdlog/sinks/ostream_sink.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

// Note: Messages are captured as "level: message" while this object is alive
class LogCapture
{
public:
	LogCapture()
	{
		m_Sink = new_shared<spdlog::sinks::ostream_sink_mt>( m_Output );
		m_Sink->set_pattern( "%l: %v" );
		Log::GetLogger().sinks().push_back( m_Sink );
	}

	~LogCapture() { Log::GetLogger().sinks().pop_back(); }

	std::string GetOutput() const { return m_Output.str(); }
	bool        Contains( std::string_view text ) const { return m_Output.str().find( text ) != std::string::npos; }

private:
	std::ostringstream                     m_Output;
	shared<spdlog::sinks::ostream_sink_mt> m_Sink;
};

// Note: Named after the running test so tests running in parallel never share a directory
class ScopedTempDir
{
public:
	ScopedTempDir()
	{
		const testing::TestInfo* testInfo = testing::UnitTest::GetInstance()->current_test_info();

		m_Path = std::filesystem::temp_directory_path() / ( std::string( "gaia3_tests_" ) + testInfo->test_suite_name() + "_" + testInfo->name() );
		std::filesystem::remove_all( m_Path );
		std::filesystem::create_directories( m_Path );
	}

	~ScopedTempDir()
	{
		std::error_code ec;
		std::filesystem::remove_all( m_Path, ec );
	}

	std::string GetPath() const { return m_Path.string(); }
	std::string PathTo( std::string_view fileName ) const { return ( m_Path / fileName ).string(); }

	std::string Write( std::string_view fileName, std::string_view text ) const
	{
		const std::filesystem::path filePath = m_Path / fileName;
		std::filesystem::create_directories( filePath.parent_path() );

		std::ofstream file( filePath, std::ios::binary );
		file << text;
		return filePath.string();
	}

private:
	std::filesystem::path m_Path;
};
