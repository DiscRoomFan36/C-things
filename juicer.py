#!/bin/python3

#
# squeeze the header files together
#
# Fletcher M - 07/02/2025
#


DEBUG = False

def read_entire_file(filepath: str) -> str:
    with open(filepath, "r") as f:
        return f.read()

SOURCES_FOLDER = "./src/"
MYTB_SOURCE    = SOURCES_FOLDER+"mytb_unsqueezed.h"


def get_all_includes(file: str) -> list[str]:
    return [line for line in file.split("\n") if line.startswith("#include")]


from enum import Enum
class Lib_Type(Enum):
    SYSTEM = 0
    OTHER  = 1

def get_include_name(line: str) -> tuple[str, Lib_Type]:
    if "<" in line:
        _, rest = line.split("<")
        name, _ = rest.split(">")
        return name, Lib_Type.SYSTEM

    if "\"" in line:
        _, name, _ = line.split("\"")
        return name, Lib_Type.OTHER

    assert False

def separate_includes(includes: list[str]) -> tuple[list[str], list[str]]:
    sys   = []
    other = []
    for include in includes:
        name, lib_type = get_include_name(include)
        match lib_type:
            case Lib_Type.SYSTEM: sys  .append(name)
            case Lib_Type.OTHER:  other.append(name)
    return sys, other

def get_current_day_string() -> str:
    from datetime import datetime
    date = datetime.today()
    return f"{date.day:02}/{date.month:02}/{date.year}"


if __name__ == "__main__":
    mytb_unsqueezed = read_entire_file(MYTB_SOURCE)

    # find imports
    includes = get_all_includes(mytb_unsqueezed)
    if DEBUG: print(includes)

    sys_includes, other_includes = separate_includes(includes)
    if DEBUG: print(sys_includes)
    if DEBUG: print(other_includes)


    # search all sub imports for other includes
    dependency_map: dict[str, tuple[list[str], list[str]]] = {}
    for inc in other_includes:
        new_file = read_entire_file(SOURCES_FOLDER + inc)

        other_file_includes = get_all_includes(new_file)
        dependency_map[inc] = separate_includes(other_file_includes)
    if DEBUG: print(dependency_map)


    # de-dup all system includes
    all_system_includes = set(sys_includes)
    for sys, _ in dependency_map.values():
        for inc in sys:
            all_system_includes.add(inc)
    sorted_system = sorted(all_system_includes)


    # start constructing the output
    mytb_output = [
        "//",
        "// mytb.h - my stb clone",
        "//",
        "// Fletcher M",
        "// Compiled (squeezed) - " + get_current_day_string(),
        "//",
        "",
        "#ifndef MYTB_H_",
        "#define MYTB_H_",
        "",
    ]

    # now we add the std libs
    mytb_output.append("// All used standard libs")
    for lib in sorted_system: mytb_output.append(f"#include <{lib}>")


    # now the rest of the file. in the order that it came in. hopefully this is the right order
    lines = mytb_unsqueezed.splitlines()

    # skip the header, we already got one
    while True:
        lines = lines[1:]
        if lines[0].startswith("#define MYTB_H_"):
            break
    lines = lines[1:]


    # TODO figure out some way to add prefix's
    # I could do it here... might be fun

    for line in lines:
        if not line.startswith("#include"):
            # bit of a hack...
            if line == "#endif // MYTB_H_": continue

            mytb_output.append(line)
            continue

        name, type = get_include_name(line)
        if type == Lib_Type.SYSTEM:
            # we added all the system libs
            continue

        assert type == Lib_Type.OTHER

        mytb_output.append("//")
        mytb_output.append(f"// ---------- {name} ----------")
        mytb_output.append("//")
        mytb_output.append("")

        # we need to add the whole file.
        the_file = read_entire_file(SOURCES_FOLDER + name)

        for line in the_file.splitlines():
            if line.startswith("#include"): continue
            mytb_output.append(line)



    mytb_output.append("#endif // MYTB_H_")
    mytb_output.append("")

    final_output = "\n".join(mytb_output)

    # now save it to the file
    import sys

    if DEBUG: print(final_output)

    assert sys.argv[0] == "juicer.py"
    assert len(sys.argv) == 2

    with open(sys.argv[1], "w") as f:
        f.write(final_output)




