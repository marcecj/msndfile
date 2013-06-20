"""SCons.Tool.default

Initialization with a default tool list.

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
selection method.

"""

#
# Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 The SCons Foundation
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

import SCons.Tool

def tool_list(platform, env):

    if platform != "win32":
        return SCons.Tool.tool_list(platform, env)

    linkers     = ['gnulink', 'ilink', 'linkloc', 'ilink32' ]
    c_compilers = ['mingw', 'gcc', 'intelc', 'icl', 'icc', 'cc', 'bcc32']
    ars         = ['ar', 'tlib']

    c_compiler = SCons.Tool.FindTool(c_compilers, env) or c_compilers[0]

    if c_compiler and c_compiler == 'mingw':
        # MinGW contains a linker, C compiler, C++ compiler, Fortran compiler,
        # archiver and assembler:
        linker = None
        ar = None
    else:
        linker = SCons.Tool.FindTool(linkers, env) or linkers[0]
        ar     = SCons.Tool.FindTool(ars, env)     or ars[0]

    other_tools = SCons.Tool.FindAllTools(['msvs', 'filesystem', 'zip'], env)

    tools = ([linker, c_compiler, ar] + other_tools)

    return [x for x in tools if x]

def generate(env):
    """Add default tools."""
    for t in tool_list(env['PLATFORM'], env):
        SCons.Tool.Tool(t)(env)

def exists(env):
    return 1

# Local Variables:
# tab-width:4
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=4 shiftwidth=4:
