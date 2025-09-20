/*
 * Copyright (C) 2020 Emeric Poupon
 *
 * This file is part of fileshelter.
 *
 * fileshelter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fileshelter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fileshelter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils/String.hpp"

#include <iomanip>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>

namespace StringUtils
{
    bool readList(const std::string& str, const std::string& separators, std::list<std::string>& results)
    {
        std::string curStr;

        for (char c : str)
        {
            if (separators.find(c) != std::string::npos)
            {
                if (!curStr.empty())
                {
                    results.push_back(curStr);
                    curStr.clear();
                }
            }
            else
            {
                if (curStr.empty() && std::isspace(c))
                    continue;

                curStr.push_back(c);
            }
        }

        if (!curStr.empty())
            results.push_back(curStr);

        return !str.empty();
    }

    template<>
    std::optional<std::string> readAs(const std::string& str)
    {
        return str;
    }

    std::vector<std::string> splitString(const std::string& string, const std::string& separators)
    {
        std::string str{ stringTrim(string, separators) };

        std::vector<std::string> res;
        boost::algorithm::split(res, str, boost::is_any_of(separators), boost::token_compress_on);

        return res;
    }

    std::string joinStrings(const std::vector<std::string>& strings, const std::string& delimiter)
    {
        return boost::algorithm::join(strings, delimiter);
    }

    std::string stringTrim(const std::string& str, const std::string& whitespace)
    {
        const auto strBegin = str.find_first_not_of(whitespace);
        if (strBegin == std::string::npos)
            return ""; // no content

        const auto strEnd = str.find_last_not_of(whitespace);
        const auto strRange = strEnd - strBegin + 1;

        return str.substr(strBegin, strRange);
    }

    std::string stringTrimEnd(const std::string& str, const std::string& whitespace)
    {
        return str.substr(0, str.find_last_not_of(whitespace) + 1);
    }

    std::string stringToLower(std::string_view str)
    {
        std::string res;
        res.reserve(str.size());

        std::transform(std::cbegin(str), std::cend(str), std::back_inserter(res), [](char c) { return std::tolower(c); });

        return res;
    }

    std::string stringToUpper(const std::string& str)
    {
        std::string res;
        res.reserve(str.size());

        std::transform(std::cbegin(str), std::cend(str), std::back_inserter(res), [](char c) { return std::toupper(c); });

        return res;
    }

    std::string bufferToString(const std::vector<unsigned char>& data)
    {
        std::ostringstream oss;

        for (unsigned char c : data)
        {
            oss << std::setw(2) << std::setfill('0') << std::hex << (int)c;
        }

        return oss.str();
    }

    std::string replaceInString(const std::string& str, const std::string& from, const std::string& to)
    {
        std::string res{ str };
        size_t pos = 0;

        while ((pos = res.find(from, pos)) != std::string::npos)
        {
            res.replace(pos, from.length(), to);
            pos += to.length();
        }

        return res;
    }

    std::string jsEscape(const std::string& str)
    {
        static const std::unordered_map<char, std::string_view> escapeMap{
            { '\\', "\\\\" },
            { '\n', "\\n" },
            { '\r', "\\r" },
            { '\t', "\\t" },
            { '"', "\\\"" },
        };

        std::string escaped;
        escaped.reserve(str.length());

        for (const char c : str)
        {
            auto it{ escapeMap.find(c) };
            if (it == std::cend(escapeMap))
            {
                escaped += c;
                continue;
            }

            escaped += it->second;
        }

        return escaped;
    }

    bool stringEndsWith(const std::string& str, const std::string& ending)
    {
        return boost::algorithm::ends_with(str, ending);
    }

    std::optional<std::string> stringFromHex(const std::string& str)
    {
        static const char lut[]{ "0123456789ABCDEF" };

        if (str.length() % 2 != 0)
            return std::nullopt;

        std::string res;
        res.reserve(str.length() / 2);

        auto it{ std::cbegin(str) };
        while (it != std::cend(str))
        {
            unsigned val{};

            auto itHigh{ std::lower_bound(std::cbegin(lut), std::cend(lut), std::toupper(*(it++))) };
            auto itLow{ std::lower_bound(std::cbegin(lut), std::cend(lut), std::toupper(*(it++))) };

            if (itHigh == std::cend(lut) || itLow == std::cend(lut))
                return {};

            val = std::distance(std::cbegin(lut), itHigh) << 4;
            val += std::distance(std::cbegin(lut), itLow);

            res.push_back(static_cast<char>(val));
        }

        return res;
    }
} // namespace StringUtils
