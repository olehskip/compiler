#ifndef PRINTABLE_HPP
#define PRINTABLE_HPP

#include <sstream>

class Printable
{
public:
    virtual void pretty(std::stringstream &stream) const = 0;
    virtual void refPretty(std::stringstream &stream) const = 0;
};
#endif // PRINTABLE_HPP
