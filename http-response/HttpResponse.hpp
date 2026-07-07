#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>

class HttpResponse
{
public:
    HttpResponse();

    void setStatus(int code); 
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);

    void setDefaultErrorPage(int code);

    std::string toString() const;

private:
    int                                 _statusCode;
    std::string                         _statusText;
    std::map<std::string, std::string>  _headers;
    std::string                         _body;

    std::string getStatusText(int code) const;
};

#endif
