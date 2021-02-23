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
 * Order.h
 *
 * An order for a pets from the pet store
 */

#ifndef Order_H_
#define Order_H_


#include <string>
#include <nlohmann/json.hpp>

namespace org {
namespace openapitools {
namespace server {
namespace model {

/// <summary>
/// An order for a pets from the pet store
/// </summary>
class  Order
{
public:
    Order();
    virtual ~Order();

    void validate();

    /////////////////////////////////////////////
    /// Order members

    /// <summary>
    /// 
    /// </summary>
    int64_t getId() const;
    void setId(int64_t const value);
    bool idIsSet() const;
    void unsetId();
    /// <summary>
    /// 
    /// </summary>
    int64_t getPetId() const;
    void setPetId(int64_t const value);
    bool petIdIsSet() const;
    void unsetPetId();
    /// <summary>
    /// 
    /// </summary>
    int32_t getQuantity() const;
    void setQuantity(int32_t const value);
    bool quantityIsSet() const;
    void unsetQuantity();
    /// <summary>
    /// 
    /// </summary>
    std::string getShipDate() const;
    void setShipDate(std::string const& value);
    bool shipDateIsSet() const;
    void unsetShipDate();
    /// <summary>
    /// Order Status
    /// </summary>
    std::string getStatus() const;
    void setStatus(std::string const& value);
    bool statusIsSet() const;
    void unsetStatus();
    /// <summary>
    /// 
    /// </summary>
    bool isComplete() const;
    void setComplete(bool const value);
    bool completeIsSet() const;
    void unsetComplete();

    friend void to_json(nlohmann::json& j, const Order& o);
    friend void from_json(const nlohmann::json& j, Order& o);
protected:
    int64_t m_Id;
    bool m_IdIsSet;
    int64_t m_PetId;
    bool m_PetIdIsSet;
    int32_t m_Quantity;
    bool m_QuantityIsSet;
    std::string m_ShipDate;
    bool m_ShipDateIsSet;
    std::string m_Status;
    bool m_StatusIsSet;
    bool m_Complete;
    bool m_CompleteIsSet;
};

}
}
}
}

#endif /* Order_H_ */
