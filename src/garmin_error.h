// $Id$
// garmin_error.h
// Douglas S. J. De Couto
// September 9, 1998

// Copyright (C) 1998 Douglas S. J. De Couto
// <decouto@lcs.mit.edu>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#ifndef _garerrh
#define _garerrh

#include <string>

namespace garmin {

class error 
{
public:
	error() : m_msg("<no msg>") { }
	error(const error &err) : m_msg(err.m_msg) { }
	error(const std::string &msg) : m_msg(msg) { }
	
	std::string m_msg;
};

class not_possible : public error 
{
public:
	not_possible() : error("The operation was not possible") { }
	not_possible(const not_possible &err) : error(err) { }
	not_possible(const std::string &msg) : error(msg) { }
};

class timeout : public error 
{
public:
	timeout() : error("The operation timed out") { }
	timeout(const timeout &err) : error(err) { }
	timeout(const std::string &msg) : error(msg) { }
};

class bad_time : public error 
{
public:
	bad_time() : error("The supplied time value is invalid") { }
	bad_time(const bad_time &err) : error(err) { }
	bad_time(const std::string &msg) : error(msg) { }
};


class unsupported_protocol : public error 
{
public:
	unsupported_protocol() : error("The selected protocol is not supported") { }
	unsupported_protocol(const unsupported_protocol &err) : error(err) { }
	unsupported_protocol(const std::string &msg) : error(msg) { }
};

class unsupported_datatype : public error 
{
public:
	unsupported_datatype() : error("The required datatype is not supported") { }
	unsupported_datatype(const unsupported_protocol &err) : error(err) { }
	unsupported_datatype(const std::string &msg) : error(msg) { }
};
} // namespace

#endif // _garerrh
