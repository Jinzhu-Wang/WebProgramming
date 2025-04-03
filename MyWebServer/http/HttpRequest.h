#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <string>

enum class RequestMethod{
    kInvalid = 0,
    kGet,
    kPost,
    kHead,
    kPut,
    kDelete
};

enum class RequestVersion
{
    kUnknown = 0,
    kHttp10,
    kHttp11
};


class HttpRequest
{
private:
    RequestMethod method_;
    RequestVersion version_;

    std::map<std::string, std::string> request_params_; // 请求参数
    std::string url_; // 请求路径
    std::string protocol_;
    std::map<std::string, std::string> headers_; // 请求头
    std::string body_; // 请求体


public:
    HttpRequest();
    ~HttpRequest();

    void SetVersion(const std::string &version);//http版本
    RequestVersion version() const;
    std::string GetVersionString() const;

    bool SetMethod(const std::string &method); // 设定请求方法
    RequestMethod method() const;
    std::string GetMethodString() const ;

    void SetUrl(const std::string &url); // 请求路径
    const std::string& url() const;
    
    void SetRequestParams(const std::string &key, const std::string &value);
    std::string GetRequestValue(const std::string &key) const;
    const std::map<std::string, std::string> & request_params() const;

    void SetProtocol(const std::string &str);
    const std::string & protocol() const;

    void AddHeader(const std::string &field, const std::string &value); // 添加请求体
    std::string GetHeader(const std::string &field) const;
    const std::map<std::string, std::string> & headers() const;

    void SetBody(const std::string &str);
    const std::string & body() const;

};

#endif