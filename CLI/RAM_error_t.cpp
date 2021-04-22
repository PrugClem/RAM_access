#include "RAM_access.hpp"

RAM::error_t::error_t(const std::string& context)
{
	char strMsg[256];
	std::stringstream ss;

	this->ecode = GetLastError();
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, this->ecode, 0,
		strMsg, sizeof(strMsg), NULL);
	ss << context << " (code " << this->ecode << "): " << strMsg;
	this->emessage = ss.str();
}

RAM::error_t& RAM::error_t::operator=(const error_t &other)
{
	this->ecode = other.ecode;
	this->emessage = other.emessage;
	return *this;
}

const char* RAM::error_t::what() const throw()
{
	return this->emessage.c_str();
}

void RAM::error_t::cause(RAM::error_t* err, const std::string& context)
{
	if (err == nullptr) throw error_t(context);
	else *err = error_t(context);
}
