import pyparsing as pp

# Parse c++ enum from string of code
def enum_parser(code_string: str) -> dict[str, dict[str, int]]:
    # syntax we don't want to see in the final parse tree
    LBRACE, RBRACE, EQ, COMMA = pp.Suppress.using_each("{}=,")
    _enum = pp.Suppress("enum")
    identifier = pp.Word(pp.alphas + "_", pp.alphanums + "_")
    integer = pp.Word(pp.nums)
    enumValue = pp.Group(identifier("name") + pp.Optional(EQ + integer("value")))
    enumList = pp.Group(enumValue + (COMMA + enumValue)[...] + pp.Optional(COMMA))
    enum = _enum + identifier("enum") + pp.Optional(":" + identifier) + LBRACE + enumList("names") + RBRACE

    # find instances of enums ignoring other syntax
    enums: dict[str, dict[str, int]] = {}
    for item, start, stop in enum.scan_string(code_string):
        idx = 0
        for entry in item.names:
            if(entry.value != ""):
                idx = int(entry.value)
            if(not item.enum in enums):
                enums[item.enum] = {}
            #...
            enums[item.enum][entry.name] = idx
            idx += 1
    return(enums)
