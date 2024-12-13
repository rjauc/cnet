#pragma once
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <format>
#include <unordered_map>
#include <unordered_set>

namespace LOG
{
    enum class Severity: char {
        Debug = 'D', Info = 'I', Warning = 'W', Error = 'E'
    };

    namespace internal
    {
        static inline const std::unordered_map<Severity, const char*> SEVERITY_TERM_COLORS = {
            { Severity::Debug,   "\033[90m" },
            { Severity::Info,    "\033[96m" },
            { Severity::Warning, "\033[33m" },
            { Severity::Error,   "\033[91m" },
        };
        static inline const char* RESET_TERM_COLOR = "\033[0m";
        static inline std::unordered_set<std::filesystem::path> g_initializedFiles;
    }

    class Logger {
        public:
            Logger(std::string_view name = "Root", std::string fileName = "runtime") : m_name{name} {
                SetFileName(fileName);
            }

            template <typename... Args>
            void SetName(std::string_view fmt, Args &&...args) {
                m_name = std::vformat(fmt, std::make_format_args(args...));
            }

            void SetFileName(std::string fileName) {
                m_filePath = std::filesystem::path("./logs/");
                if (!std::filesystem::exists(m_filePath))
                    std::filesystem::create_directory(m_filePath);
                
                m_filePath += fileName + ".log";
                if (!internal::g_initializedFiles.contains(m_filePath)) {
                    if (std::filesystem::exists(m_filePath))
                        std::filesystem::remove(m_filePath);
                    internal::g_initializedFiles.insert(m_filePath);
                }
            }
            
            template <typename... Args>
            void Log(Severity severity, std::string_view fmt, Args &&...args) {
                auto formattedTime = GetFormattedTime();
                auto severityChar = static_cast<char>(severity);
                auto header = std::format("({})[{}][{}]: ", formattedTime, severityChar, m_name);
                auto message = std::vformat(fmt, std::make_format_args(args...)) + "\n";

                // FIXME AE: Handle simultaneous file access error
                auto fileStream = std::ofstream(m_filePath, std::ios_base::app);
                std::cout << internal::SEVERITY_TERM_COLORS.at(severity);
                Log(fileStream, header);
                std::cout << internal::RESET_TERM_COLOR;
                Log(fileStream, message);
                fileStream.close();
            }

            template <typename... Args> void Debug(std::string_view fmt, Args &&...args)   { Log(Severity::Debug, fmt, args...);   }
            template <typename... Args> void Info(std::string_view fmt, Args &&...args)    { Log(Severity::Info, fmt, args...);    }
            template <typename... Args> void Warning(std::string_view fmt, Args &&...args) { Log(Severity::Warning, fmt, args...); }
            template <typename... Args> void Error(std::string_view fmt, Args &&...args)   { Log(Severity::Error, fmt, args...);   }

        private:
            void Log(std::ofstream& fileStream, std::string_view data) {
                std::cout << data;
                fileStream << data;
            }

            std::string GetFormattedTime() {
                auto now = std::chrono::system_clock::now();
                auto nowTime = std::chrono::system_clock::to_time_t(now);
                std::stringstream ss;
                ss << std::put_time(std::localtime(&nowTime), "%T");
                return ss.str();
            }

        private:   
            std::string m_name = "Root";
            std::filesystem::path m_filePath;
    };
}

#ifndef NDEBUG
    #define LOG_DECLARE() LOG::Logger _m_logger
    #define LOG_MOVE(_objDstPtr, _objSrc) (_objDstPtr)->_m_logger = _objSrc._m_logger
    #define LOG_SET_NAME(...) _m_logger.SetName(__VA_ARGS__)
    #define LOG_SET_FILE_NAME(_fileName) _m_logger.SetFileName(_fileName)
    #define LOG_DEBUG(...)   _m_logger.Debug(__VA_ARGS__)
    #define LOG_INFO(...)    _m_logger.Info(__VA_ARGS__)
    #define LOG_WARNING(...) _m_logger.Warning(__VA_ARGS__)
    #define LOG_ERROR(...)   _m_logger.Error(__VA_ARGS__)
#else
    #define LOG_DECLARE()
    #define LOG_MOVE(_objDstPtr, _objSrc) do {} while (0)
    #define LOG_SET_NAME(...) do {} while (0)
    #define LOG_SET_FILE_NAME(_fileName) do {} while (0)
    #define LOG_DEBUG(...)   do {} while (0)
    #define LOG_INFO(...)    do {} while (0)
    #define LOG_WARNING(...) do {} while (0)
    #define LOG_ERROR(...)   do {} while (0)
#endif
