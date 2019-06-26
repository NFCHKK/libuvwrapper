#ifndef _LOG4_CPLUS_
#define _LOG4_CPLUS_
#include <sstream>
#include <log4cplus/logger.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <string>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define  MKDIR(x1, x2) _mkdir(x1)
#define  ACCESS(x1, x2) _access(x1, x2)
#ifdef _DEBUG
#pragma  comment(lib, "log4cplusUD.lib")
#else
#pragma  comment(lib, "log4cplusU.lib")
#endif
#else
#include <unistd.h>
#include <sys/stat.h>
#define MKDIR(x1, x2) mkdir(x1, x2)
#define ACCESS(x1, x2) access(x1, x2)
#endif

using namespace log4cplus;
using namespace log4cplus::helpers;


class tlogger
{
public:
	tlogger()
	{
		log4cplus::initialize();
		m_rootLoger = Logger::getRoot();
		m_logger = Logger::getInstance(LOG4CPLUS_TEXT("log"));
		try
		{
			std::string logdir("log");
			if (ACCESS(logdir.c_str(), 0) == -1)
			{
				MKDIR(logdir.c_str(), 755);
			}
		}
		catch (...)
		{
			
		}
		PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("./conf/log4cplus.properties"));
	};
	Logger m_logger;
	~tlogger()
	{
	
	};
private:
	Logger m_rootLoger;
	
};

static tlogger g_logger;

//#define LOG_TRACE(X)  LOG4CPLUS_TRACE(g_logger.m_logger, (X));
//#define LOG_DEBUG(X)  LOG4CPLUS_DEBUG(g_logger.m_logger, (X));
//#define LOG_INFO(X)   LOG4CPLUS_INFO(g_logger.m_logger, (X));
//#define LOG_WARN(X)  LOG4CPLUS_WARN(g_logger.m_logger, (X));
//#define LOG_ERROR(X)  LOG4CPLUS_ERROR(g_logger.m_logger, (X));
//#define LOG_FATAL(X)  LOG4CPLUS_FATAL(g_logger.m_logger, (X));

#define LOG_TRACE(X)   \
{                           \
	std::stringstream sstr; \
	sstr << X << std::ends; \
	LOG4CPLUS_TRACE(g_logger.m_logger, sstr.str().c_str());\
}
#define LOG_DEBUG(X)   \
{                           \
	std::stringstream sstr; \
	sstr << X << std::ends; \
	LOG4CPLUS_DEBUG(g_logger.m_logger, sstr.str().c_str());\
}
#define LOG_INFO(X)   \
{                           \
	std::stringstream sstr; \
	sstr << X << std::ends; \
	LOG4CPLUS_INFO(g_logger.m_logger, sstr.str().c_str());\
}
#define LOG_WARN(X)   \
{                           \
	std::stringstream sstr; \
	sstr << X << std::ends; \
	LOG4CPLUS_WARN(g_logger.m_logger, sstr.str().c_str());\
}
#define LOG_ERROR(X)   \
{                           \
	std::stringstream sstr; \
	sstr << X << std::ends; \
	LOG4CPLUS_ERROR(g_logger.m_logger, sstr.str().c_str());\
}
#define LOG_FATAL(X)   \
{                           \
	std::stringstream sstr; \
	sstr << X << std::ends; \
	LOG4CPLUS_FATAL(g_logger.m_logger, sstr.str().c_str());\
}
#endif
