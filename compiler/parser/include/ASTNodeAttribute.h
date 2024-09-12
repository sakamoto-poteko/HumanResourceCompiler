#ifndef ASTNODEATTRIBUTE_H
#define ASTNODEATTRIBUTE_H

#include <concepts>
#include <memory>

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
};

CLOSE_PARSER_NAMESPACE

#endif // !ASTNODEATTRIBUTE_H