class EbnfError(Exception):
    pass

class LeftRecursionError(EbnfError):
    def __init__(self, rule_name : str, *args: object) -> None:
        self.rule_name = rule_name
        super().__init__(*args)

class FirstFirstConflictError(EbnfError):
    def __init__(self, rule_name : str, *args: object) -> None:
        self.rule_name = rule_name
        super().__init__(*args)