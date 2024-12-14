#pragma once
#include <iostream>
#include <map>

namespace ARG
{
    struct Arg {
        std::string opt = "";
        std::string longOpt = "";
        std::string value = "";
        std::string description = "";
        bool isMandatory = false;
        bool isBoolean = false;
        bool isSet = false;
    };

    class ArgParser {
        public:
            ArgParser() {
                Add("Executable", { .description="Specifies the currently running executable.", .isMandatory=true });
            }

            void Add(std::string name, Arg arg) {
                m_args[name] = {
                    .opt = arg.opt,
                    .longOpt = arg.longOpt,
                    .value = arg.value,  // Default value, replaced by actual value if it exists
                    .description = arg.description,
                    .isMandatory = arg.isMandatory,
                    .isBoolean = arg.isBoolean
                };
            }

            bool Parse(int argc, char** argv) {
                m_args["Executable"].value = std::string(argv[0]);
                m_args["Executable"].isSet = true;
                
                Arg* arg = nullptr;
                for (int i = 1; i < argc; i++) {
                    auto argStr = std::string(argv[i]);

                    if (argStr == "-h" || argStr == "--help") {
                        PrintHelp();
                        return false;
                    }

                    if (arg) {
                        arg->value = argStr;
                        arg->isSet = true;
                    } else if (argStr.starts_with("--")) {
                        arg = FindArg(argStr);
                        if (arg->isBoolean) {
                            arg->isSet = true;
                        } else continue;
                    } else if (argStr[0] == '-') {
                        auto opt = std::string_view(&argStr[0], 2);
                        arg = FindArg(opt);
                        if (arg->isBoolean) {
                            arg->isSet = true;
                        } else if (argStr.length() > 2) {
                            arg->value = std::string_view(&argStr[2], argStr.length() - 2);
                            arg->isSet = true;
                        } else continue;
                    }
                    arg = nullptr;
                }
                
                // TODO AE: Optionally handle unknown options
                if (!CheckArgs()) {
                    PrintHelp();
                    return false;
                }
                return true;
            }

            bool IsSet(const std::string& name) { return m_args.contains(name) && m_args[name].isSet; }

            std::string GetStr(const std::string& name) { return m_args[name].value; }
            auto operator[](const std::string& name) { return m_args[name].value; }

            // TODO AE: Handle unset and non-existing args
            int GetInt(const std::string& name) { return std::atoi(m_args[name].value.c_str()); }
            double GetDouble(const std::string& name) { return std::stod(m_args[name].value.c_str()); }
            bool GetBool(const std::string& name) { return m_args[name].isBoolean && m_args[name].isSet; }

        private:
            Arg* FindArg(std::string_view opt) {
                for (auto& [name, arg]: m_args) {
                    if (arg.opt == opt)
                        return &arg;
                    if (arg.longOpt == opt)
                        return &arg;
                }
                return nullptr;
            }

            void PrintHelp() {
                int i = 0;
                std::cout << "Options:\n";
                for (const auto& [name, arg]: m_args) {
                    i += 1;
                    if (name == "Executable")
                        continue;

                    if (arg.opt.length() > 0)
                        std::cout << "\t" << arg.opt;
                    if (arg.longOpt.length() > 0)
                        if (arg.opt.length() > 0)
                            std::cout << ", ";
                        else
                            std::cout << "\t";
                        std::cout << arg.longOpt;
                    if (arg.isMandatory)
                        std::cout << "\tMANDATORY";
                    if (arg.value.length() > 0)
                        std::cout << "\tDEFAULT = " << arg.value;
                    if (arg.description.length() > 0)
                        std::cout << "\n\t\t" << arg.description;

                    std::cout << '\n';
                    if (i != m_args.size())
                        std::cout << '\n';
                }
            }

            bool CheckArgs() {
                for (const auto& [name, arg]: m_args)
                    if (arg.isMandatory && !arg.isSet)
                        return false;
                return true;
            }

        private:
            std::map<std::string, Arg> m_args;
    };
}
