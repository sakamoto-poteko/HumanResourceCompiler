from typing import Union
from tokenizer import *
import exceptions

# EBNF for it
# syntax       = { production } ;
# production   = ID EQUALS expression SEMICOLON ;
# expression   = term { PIPE term } ;
# Term         = Factor, { COMMA Factor } ;
# Factor       = Identifier
#              | Literal
#              | Optional
#              | Repeated
#              | Grouped ;
# Optional     = "[", Expression, "]" ;
# Repeated     = "{", Expression, "}" ;
# Grouped      = "(", Expression, ")" ;


# AST node classes
class ASTNode:
    def to_graphviz(self) -> str:
        """
        Generates a Graphviz representation of the AST.
        """
        graphviz_lines = ["digraph AST {", "node [shape=box];"]
        self._to_graphviz(graphviz_lines)
        graphviz_lines.append("}")
        return "\n".join(graphviz_lines)

    def _to_graphviz(
        self, lines: List[str], parent_id: str = "", node_id: int = 0
    ) -> int:
        """
        Recursive helper method to generate the Graphviz lines.
        """
        current_id = node_id
        node_label = type(self).__name__
        if isinstance(self, Token):
            node_label = f"{self.type}: {self.value}"
        lines.append(f'  node{current_id} [label="{node_label}"];')
        if parent_id:
            lines.append(f"  {parent_id} -> node{current_id};")

        # Traverse children if any
        if hasattr(self, "__dict__"):
            for key, value in self.__dict__.items():
                if isinstance(value, list):
                    for item in value:
                        if isinstance(item, ASTNode):
                            current_id += 1
                            current_id = item._to_graphviz(
                                lines, f"node{node_id}", current_id
                            )
                elif isinstance(value, ASTNode):
                    current_id += 1
                    current_id = value._to_graphviz(lines, f"node{node_id}", current_id)

        return current_id


# region Nodes
class Syntax(ASTNode):
    def __init__(self, productions: List["Production"]):
        self.productions = productions


class Production(ASTNode):
    def __init__(self, id: Token, expression: "Expression"):
        self.id = id
        self.expression = expression

    def __repr__(self):
        return f"{self.id.value} ::= {self.expression};"

    def find_identifiers_literals(self) -> set[str]:
        return self.expression.find_identifiers_literals()

    def get_unexploded_first_set(self) -> set[str]:
        return self.expression.get_unexploded_first_set()


class Expression(ASTNode):
    def __init__(self, terms: List["Term"]):
        self.terms = terms

    def __repr__(self):
        return f"{' | '.join([term.__repr__() for term in self.terms])}"

    def find_identifiers_literals(self) -> set[str]:
        identifiers = set()
        for term in self.terms:
            identifiers.update(term.find_identifiers_literals())
        return identifiers

    def get_unexploded_first_set(self) -> set[str]:
        first_set_unexploded: set[str] = set()
        for term in self.terms:
            first_set_unexploded.update(term.get_unexploded_first_set())
        return first_set_unexploded


class Term(ASTNode):
    def __init__(self, factors: List["Factor"]):
        self.factors = factors

    def __repr__(self):
        return f"{' '.join([factor.__repr__() for factor in self.factors])}"

    def find_identifiers_literals(self) -> set[str]:
        identifiers = set()
        for factor in self.factors:
            identifiers.update(factor.find_identifiers_literals())
        return identifiers

    def get_unexploded_first_set(self) -> set[str]:
        return self.factors[0].get_unexploded_first_set()


class Factor(ASTNode):
    def __init__(self, value: Union["Expression", Token]):
        self.value = value

    def __repr__(self):
        return f"{self.value}"

    def find_identifiers_literals(self) -> set[str]:
        if isinstance(self.value, Token):
            return set([self.value.value])
            # exploded.add()
            # return exploded
        else:
            # Derived factors only. Won't use this base class
            assert False

    def get_unexploded_first_set(self) -> set[str]:
        if isinstance(self.value, Token):
            return set([self.value.value])
        else:
            # Derived factors only. Won't use this base class
            assert False


class Optional(Factor):
    def __init__(self, expression: "Expression"):
        self.expression = expression

    def __repr__(self):
        return f"[{self.expression}]"

    def find_identifiers_literals(self) -> set[str]:
        return self.expression.find_identifiers_literals()

    def get_unexploded_first_set(self) -> set[str]:
        return self.expression.get_unexploded_first_set()


class Repeated(Factor):
    def __init__(self, expression: "Expression"):
        self.expression = expression

    def __repr__(self):
        return f"{{{self.expression}}}"

    def find_identifiers_literals(self) -> set[str]:
        return self.expression.find_identifiers_literals()

    def get_unexploded_first_set(self) -> set[str]:
        return self.expression.get_unexploded_first_set()


class Grouped(Factor):
    def __init__(self, expression: "Expression"):
        self.expression = expression

    def __repr__(self):
        return f"({self.expression})"

    def find_identifiers_literals(self) -> set[str]:
        return self.expression.find_identifiers_literals()

    def get_unexploded_first_set(self) -> set[str]:
        return self.expression.get_unexploded_first_set()


# endregion


# Parser class
class Parser:
    SKIPPED = "([SKIPPED])"

    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.current_token_index = 0

        self.rules: List[Production] = []
        self.terminals: set[Token] = set()
        self.identifiers: set[Token] = set()
        self.literals: set[Token] = set()

        self.first_set: dict[str, list[str]] = dict()

    def current_token(self) -> Token:
        token = self.tokens[self.current_token_index]
        return token

    def consume_token(self, expected_type: str) -> Token:
        token = self.current_token()
        if token.type != expected_type:
            raise SyntaxError(
                f"Expected {expected_type} but got {token.type} at line {token.line}, column {token.column}"
            )
        self.current_token_index += 1
        return token

    def parse(self) -> List[Production]:
        productions: List[Production] = []
        while self.current_token_index < len(self.tokens):
            productions.append(self.parse_production())

        self.terminals.clear()

        production_source_set: set[str] = set()
        for rule in productions:
            production_source_set.update(rule.find_identifiers_literals())

        terminals = set()
        terminals.update(production_source_set)
        for rule in productions:
            if rule.id.value in terminals:
                terminals.remove(rule.id.value)

        self.rules = productions
        self.terminals = terminals
        return productions

    def compute_first_follow(self, skip: set[str] = set()) -> None:
        # must parse first
        first_set_unexploded: dict[str, set[str]] = dict()
        # Key is the production id, value is the set of FIRST but not exploded
        first_set: dict[str, set[str]] = dict()
        for rule in self.rules:
            first_set_unexploded[rule.id.value] = rule.get_unexploded_first_set()

        for rule in first_set_unexploded:
            print(f"DBG: PROD RULE '{rule}'")
            if rule in skip:
                first_set[rule] = [self.SKIPPED]
            else:
                first_set[rule] = Parser.get_terminal_first(
                    rule, first_set_unexploded, self.terminals, set(), skip, None
                )

        self.first_set = first_set

    def get_terminal_first(
        rule_id: str,
        first_set_unexploded: dict[str, set[str]],
        terminals: set[str],
        rule_id_seen: set[str],
        skip: set[str],
        root_rule_for_terminal: str | None,
    ) -> set[str]:
        if rule_id in rule_id_seen:
            raise exceptions.LeftRecursionError(
                rule_id, f"'{rule_id}' is already seen. Left recursion?"
            )
        rule_id_seen.add(rule_id)

        result: set[str] = set()
        if rule_id in terminals:
            # if rule_id is terminal
            # FIXME: invoke_rule is the parsing method we want to jump to in the table
            result.add(rule_id)
            print(f"DBG: {root_rule_for_terminal}->{rule_id}")
        else:
            # it's not terminal. we lookup every FIRST of it, and return
            unexploded_first = first_set_unexploded[rule_id]
            for first in unexploded_first:
                if first in skip:
                    result.update([Parser.SKIPPED])
                    continue

                this_rule_id_seen = set(rule_id_seen)
                f = Parser.get_terminal_first(
                    first,
                    first_set_unexploded,
                    terminals,
                    this_rule_id_seen,
                    skip,
                    first if root_rule_for_terminal == None else root_rule_for_terminal,
                )
                if len(result.intersection(f)) > 0:
                    raise exceptions.FirstFirstConflictError(
                        rule_id, f"'{rule_id}->{first} has FIRST/FIRST conflict"
                    )
                result.update(f)
        return result

    # region Parsing
    def parse_production(self) -> Production:
        id_token = self.consume_token("ID")
        self.consume_token("EQUALS")
        expression = self.parse_expression()
        self.consume_token("SEMICOLON")
        return Production(id_token, expression)

    def parse_expression(self) -> Expression:
        terms = [self.parse_term()]
        while self.current_token().type == "PIPE":
            self.consume_token("PIPE")
            terms.append(self.parse_term())
        return Expression(terms)

    def parse_term(self) -> Term:
        factors = [self.parse_factor()]

        while self.current_token().type == "COMMA":
            self.consume_token("COMMA")
            factors.append(self.parse_factor())
        return Term(factors)

    def parse_factor(self) -> Factor:
        token = self.current_token()
        if token.type == "ID":
            self.consume_token("ID")
            return Factor(token)
        elif token.type == "LITERAL":
            self.consume_token("LITERAL")
            return Factor(token)
        elif token.type == "LBRACKET":
            return self.parse_optional()
        elif token.type == "LBRACE":
            return self.parse_repeated()
        elif token.type == "LPAREN":
            return self.parse_grouped()
        else:
            raise SyntaxError(
                f"Unexpected token {token.type} at line {token.line}, column {token.column}"
            )

    def parse_optional(self) -> Optional:
        self.consume_token("LBRACKET")
        expression = self.parse_expression()
        self.consume_token("RBRACKET")
        return Optional(expression)

    def parse_repeated(self) -> Repeated:
        self.consume_token("LBRACE")
        expression = self.parse_expression()
        self.consume_token("RBRACE")
        return Repeated(expression)

    def parse_grouped(self) -> Grouped:
        self.consume_token("LPAREN")
        expression = self.parse_expression()
        self.consume_token("RPAREN")
        return Grouped(expression)


# endregion
