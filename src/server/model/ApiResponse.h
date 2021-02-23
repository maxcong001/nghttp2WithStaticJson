/**
* OpenAPI Petstore
* This is a sample server Petstore server. For this sample, you can use the api key `special-key` to test the authorization filters.
*
* The version of the OpenAPI document: 1.0.0
* 
*
* NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
* https://openapi-generator.tech
* Do not edit the class manually.
*/
/*
 * ApiResponse.h
 *
 * Describes the result of uploading an image resource
 */

#ifndef ApiResponse_H_
#define ApiResponse_H_


#include <string>
#include <nlohmann/json.hpp>

namespace org {
namespace openapitools {
namespace server {
namespace model {

/// <summary>
/// Describes the result of uploading an image resource
/// </summary>
class  ApiResponse
{
public:
    ApiResponse();
    virtual ~ApiResponse();

    void validate();

    /////////////////////////////////////////////
    /// ApiResponse members

    /// <summary>
    /// 
    /// </summary>
    int32_t getCode() const;
    void setCode(int32_t const value);
    bool codeIsSet() const;
    void unsetCode();
    /// <summary>
    /// 
    /// </summary>
    std::string getType() const;
    void setType(std::string const& value);
    bool typeIsSet() const;
    void unsetType();
    /// <summary>
    /// 
    /// </summary>
    std::string getMessage() const;
    void setMessage(std::string const& value);
    bool messageIsSet() const;
    void unsetMessage();

    friend void to_json(nlohmann::json& j, const ApiResponse& o);
    friend void from_json(const nlohmann::json& j, ApiResponse& o);
protected:
    int32_t m_Code;
    bool m_CodeIsSet;
    std::string m_Type;
    bool m_TypeIsSet;
    std::string m_Message;
    bool m_MessageIsSet;
};

}
}
}
}

#endif /* ApiResponse_H_ */
