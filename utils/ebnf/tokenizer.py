import re
from typing import List


# Define token types
class TokenType:
    ID = "ID"
    EQUALS = "EQUALS"
    SEMICOLON = "SEMICOLON"
    LBRACKET = "LBRACKET"
    RBRACKET = "RBRACKET"
    LPAREN = "LPAREN"
    RPAREN = "RPAREN"
    LBRACE = "LBRACE"
    RBRACE = "RBRACE"
    PIPE = "PIPE"
    COMMA = "COMMA"
    COMMENT = "COMMENT"
    WHITESPACE = "WHITESPACE"
    NEWLINE = "NEWLINE"
    LITERAL = "LITERAL"


# Token structure
class Token:
    def __init__(self, type: str, value: str, line: int, column: int):
        self.type = type
        self.value = value
        self.line = line
        self.column = column

    def __repr__(self):
        return self.value
        # return f"Token({self.type}, '{self.value}', line={self.line}, column={self.column})"

    def full_repr(self):
        return f"Token({self.type}, '{self.value}', line={self.line}, column={self.column})"
        # return self.value


# Tokenizer function
def tokenize(input_text: str) -> List[Token]:
    token_specification = [
        (TokenType.COMMENT, r"\(\*[^*]*\*\)"),  # Comments (* ... *)
        (TokenType.ID, r"[a-zA-Z_][a-zA-Z_0-9]*"),  # Identifiers
        (TokenType.LITERAL, r"\'[^\']*\'|\"[^\"]*\""),  # Literal
        (TokenType.EQUALS, r"="),  # Equals sign
        (TokenType.SEMICOLON, r";"),  # Semicolon
        (TokenType.LBRACKET, r"\["),  # Left square bracket
        (TokenType.RBRACKET, r"\]"),  # Right square bracket
        (TokenType.LPAREN, r"\("),  # Left parenthesis
        (TokenType.RPAREN, r"\)"),  # Right parenthesis
        (TokenType.LBRACE, r"\{"),  # Left curly brace
        (TokenType.RBRACE, r"\}"),  # Right curly brace
        (TokenType.PIPE, r"\|"),  # Pipe symbol
        (TokenType.COMMA, r","),  # Comma
        (TokenType.WHITESPACE, r"[ \t]+"),  # Whitespace
        (TokenType.NEWLINE, r"\n"),  # Newline
    ]
    tokens = []
    line = 1
    column = 1
    position = 0
    while position < len(input_text):
        match = None
        for token_type, pattern in token_specification:
            regex = re.compile(pattern)
            match = regex.match(input_text, position)
            if match:
                value = match.group(0)
                if token_type == TokenType.NEWLINE:
                    line += 1
                    column = 1
                elif token_type == TokenType.WHITESPACE:
                    column += len(value)
                elif token_type != TokenType.COMMENT:
                    tokens.append(Token(token_type, value, line, column))
                    column += len(value)
                position = match.end()
                break
        if not match:
            raise SyntaxError(
                f"Illegal character: {input_text[position]} at line {line}, column {column}"
            )
        # Handle unmatched characters or skip whitespace
        while position < len(input_text) and input_text[position].isspace():
            if input_text[position] == "\n":
                line += 1
                column = 1
            else:
                column += 1
            position += 1

    return tokens


