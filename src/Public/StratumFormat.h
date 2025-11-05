#pragma once
#include "Stratum.h"

namespace Stratum::Format {

    template<typename... Formatters>
    class Formatter final {
    public:
        constexpr Formatter() noexcept = default;

        template<typename... Params>
        static std::string format(std::string_view pattern, Params&&... u_Params) {
            static_assert(sizeof...(Params) == sizeof...(Formatters),
                "Number of formatters and parameters must match");

            std::array<std::string, sizeof...(Formatters)> parts{
                Formatters::format(std::forward<Params>(u_Params))...
            };

            std::string result;
            result.reserve(pattern.size() + 64);

            size_t lastPos = 0;
            size_t tokenPos = 0;
            size_t argIndex = 0;

            while ((tokenPos = pattern.find("{}", lastPos)) != std::string::npos) {
                result.append(pattern.substr(lastPos, tokenPos - lastPos));
                if (argIndex < parts.size())
                    result.append(parts[argIndex++]);
                else
                    result += "{}";
                lastPos = tokenPos + 2;
            }

            result.append(pattern.substr(lastPos));
            return result;
        }                                                                                       
    };
}
