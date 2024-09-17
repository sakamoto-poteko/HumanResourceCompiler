#ifndef ASTNODEATTRIBUTE_H
#define ASTNODEATTRIBUTE_H

#include <concepts>
#include <memory>
#include <type_traits>

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ASTNodeAttribute : public std::enable_shared_from_this<ASTNodeAttribute> {
public:
    ASTNodeAttribute() = default;
    virtual ~ASTNodeAttribute() = default;

    // virtual int get_type() = 0;
    virtual std::string to_string() = 0;
};

enum ParserASTNodeAttributeId : int {
    ATTR_PARSER_BEGIN = 0,
    ATTR_PARSER_END = 999,
    // range 0000-0999
    // No attributes
};

using ASTNodeAttributePtr = std::shared_ptr<ASTNodeAttribute>;

template <typename T>
concept ConvertibleToASTNodeAttributePtr = requires {
    typename T::element_type;
    requires std::convertible_to<T, ASTNodeAttribute> && std::is_same_v<T, std::shared_ptr<typename T::element_type>>;
};

template <typename AttrT>
concept HasASTNodeGetAttributeId = requires {
    {
        AttrT::get_attribute_id()
    } -> std::same_as<int>;
};

template <typename T>
class GetSetAttribute {
public:
    template <typename ASTNodePtrT = ASTNodePtr>
        requires HasASTNodeGetAttributeId<T>
    static std::shared_ptr<T> get_from(const ASTNodePtrT &node)
    {
        std::shared_ptr<ASTNodeAttribute> result;
        node->get_attribute(static_cast<T *>(nullptr)->get_attribute_id(), result);
        return std::static_pointer_cast<T>(result);
    }

    template <typename ASTNodePtrT = ASTNodePtr>
        requires HasASTNodeGetAttributeId<T>
    static void set_to(const ASTNodePtrT &node, const std::shared_ptr<T> &attr)
    {
        node->set_attribute(static_cast<T *>(nullptr)->get_attribute_id(), attr);
    }

    template <typename ASTNodePtrT = ASTNodePtr>
        requires HasASTNodeGetAttributeId<T> && std::is_base_of_v<ASTNodeAttribute, T>
    void set_to(const ASTNodePtrT &node)
    {
        node->set_attribute(static_cast<T *>(nullptr)->get_attribute_id(), static_cast<T *>(this)->shared_from_this());
    }
};

CLOSE_PARSER_NAMESPACE

#endif // !ASTNODEATTRIBUTE_H