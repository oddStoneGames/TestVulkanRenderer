#include "Logger.hpp"

std::shared_ptr<Logger> Logger::s_Logger = std::make_shared<Logger>();

const char* Logger::NormalColorCode = "\x1B[0m";
const char* Logger::WhiteColorCode = "\x1B[37m";
const char* Logger::GreenColorCode = "\x1B[32m";
const char* Logger::YellowColorCode = "\x1B[33m";
const char* Logger::RedColorCode = "\x1B[31m";

Logger::Logger() : m_Priority(Logger::Priority::DebugPriority), m_InitialString(""), m_FilePath(nullptr), m_File(nullptr),
    m_TimeBuffer(), m_TimestampFormat("[%T]") {}

Logger::~Logger() { FreeFile(); }

void Logger::FreeFile()
{
    if (m_File)
    {
        std::fclose(m_File);
        m_File = 0;
    }
}

bool Logger::EnableFileOutput()
{
    FreeFile();
    m_File = std::fopen(m_FilePath, "a");

    return m_File != 0;
}

const char* Logger::MessagePriorityToString(Priority messagePriority)
{
    const char* priorityString;

    switch (messagePriority)
    {
    case Logger::TracePriority: priorityString = "[TRACE] "; break;
    case Logger::DebugPriority: priorityString = "[DEBUG] "; break;
    case Logger::InfoPriority:  priorityString = "[INFO]  "; break;
    case Logger::WarnPriority:  priorityString = "[WARN]  "; break;
    case Logger::ErrorPriority: priorityString = "[ERROR] "; break;
    default: priorityString = "[PRIORITY] "; break;
    }

    return priorityString;
}

void Logger::SetColor(Priority messagePriority)
{
    const char* color;
    switch (messagePriority)
    {
    case Logger::TracePriority: color = WhiteColorCode; break;
    case Logger::DebugPriority: color = WhiteColorCode; break;
    case Logger::InfoPriority:  color = GreenColorCode; break;
    case Logger::WarnPriority:  color = YellowColorCode; break;
    case Logger::ErrorPriority: color = RedColorCode; break;
    default: color = NormalColorCode; break;
    }

    std::printf("%s", color);
}