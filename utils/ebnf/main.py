# Credit goes to ChatGPT. It wrotes the code with my instructions.

from tokenizer import tokenize
from parser import *
import argparse
import os.path
import sys
import json


def read_file_content(file_path: str) -> str:
    """
    Reads the content of the file specified by file_path.
    Raises appropriate errors if the file cannot be found or read.
    """
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"File not found: {file_path}")

    if not os.path.isfile(file_path):
        raise IsADirectoryError(f"Expected a file but found a directory: {file_path}")

    try:
        with open(file_path, "r") as file:
            return file.read()
    except Exception as e:
        raise IOError(f"An error occurred while reading the file: {e}")


def write_json_to_file(output, output_path: str):
    """
    Writes the given output string to the specified output file path.
    """
    try:
        with open(output_path, "w") as file:
            json.dump(output, file, indent=4)
    except Exception as e:
        raise IOError(f"An error occurred while writing to the file: {e}")


def print_info(parser: Parser):
    rules = parser.rules
    terminals = [t for t in parser.terminals]
    terminals.sort()
    print(f"EBNF has {len(rules)} rules:")
    for prod in rules:
        print(f"\t{prod}")
    print()

    print(f"EBNF has {len(terminals)} terminals:")
    print("\t", end="")
    print(terminals)
    print()

    # graphviz = ast.to_graphviz()
    # with open("ast_graph.dot", "w") as file:
    #     file.write(graphviz)


def print_first_set(parser: Parser, skip: set[str]) -> dict:
    print("FIRST set:")
    try:
        parser.compute_first_set(set() if skip == None else skip)

        # for those with skipped tags
        manual_parsing_required: set[str] = set()

        for rule, first_map in parser.first_set.items():
            print(f"\t{rule}:")
            for first in first_map:
                print(f"\t\t{first[0]} -> {first[1]}")
            if any([f[0].startswith(Parser.SKIPPED) for f in first_map]):
                manual_parsing_required.add(rule)
        print()

        if len(skip) > 0:
            print("Manual parsing is required due to skippiness:")
            for rule in manual_parsing_required:
                print(f"\t{rule}")

        output = {}
        output["first_set"] = parser.first_set
        output["first_set_skipped"] = [e for e in manual_parsing_required]
        return output

    except exceptions.LeftRecursionError as ex:
        print(
            f"\tFailed to compute FIRST due to left recursion of '{ex.rule_name}'",
            file=sys.stderr,
        )
    except exceptions.FirstFirstConflictError as ex:
        print(
            f"\tFailed to compute FIRST due to FIRST/FIRST of '{ex.rule_name}'",
            file=sys.stderr,
        )


def main():
    parser = argparse.ArgumentParser(description="Process an EBNF source file.")

    parser.add_argument("file_path", type=str, help="The path to the EBNF source file.")

    parser.add_argument(
        "-p",
        "--operation",
        nargs="+",
        choices=["info", "first"],
        required=True,
        help="Specify the operation.",
    )

    parser.add_argument(
        "--skip", nargs="*", help="Specify the rule to skip when computing FIRST set."
    )

    parser.add_argument(
        "-o", "--output", type=str, required=False, help="The path to the output file."
    )

    # Parse the arguments
    args = parser.parse_args()

    # Read the file content
    try:
        ebnf_text = read_file_content(args.file_path)
    except (FileNotFoundError, IsADirectoryError, IOError) as error:
        print(error)
        return

    tokens = tokenize(ebnf_text)
    parser = Parser(tokens)
    parser.parse()

    output = dict()
    for op in args.operation:
        if op == "info":
            print_info(parser)
        elif op == "first":
            output["first"] = print_first_set(parser, args.skip)
        else:
            print(f"Invalid operation: {op}. Use -o info or -o first.")

    if args.output:
        # Write the output to the specified file
        try:
            write_json_to_file(output, args.output)
        except IOError as error:
            print(error)


if __name__ == "__main__":
    main()
