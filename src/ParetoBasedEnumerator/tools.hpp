#ifndef __TOOLS_HPP__
#define __TOOLS_HPP__

#include <string>
#include <algorithm>

/*
 * Trimming function without including boost.
 *
 * Source: https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
 *
 * By Stackoverflow user David G, Licensed under CC-BY-SA by the Terms of Usage of Stackoverflow
 */
inline std::string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}








#endif 
