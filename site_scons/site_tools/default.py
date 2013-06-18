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

__revision__ = "src/engine/SCons/Tool/default.py issue-2856:2676:d23b7a2f45e8 2012/08/05 15:38:28 garyo"

import SCons.Tool

def tool_list(env):

    other_plat_tools=[]
    linkers = ['gnulink', 'ilink', 'linkloc', 'ilink32' ]
    c_compilers = ['mingw', 'gcc', 'intelc', 'icl', 'icc', 'cc', 'bcc32' ]
    cxx_compilers = ['intelc', 'icc', 'g++', 'c++', 'bcc32' ]
    assemblers = ['nasm', 'gas', '386asm' ]
    fortran_compilers = ['gfortran', 'g77', 'ifl', 'cvf', 'f95', 'f90', 'fortran']
    ars = ['ar', 'tlib']
    other_plat_tools=['msvs','midl']

    c_compiler = SCons.Tool.FindTool(c_compilers, env) or c_compilers[0]

    # XXX this logic about what tool provides what should somehow be
    #     moved into the tool files themselves.
    if c_compiler and c_compiler == 'mingw':
        # MinGW contains a linker, C compiler, C++ compiler,
        # Fortran compiler, archiver and assembler:
        cxx_compiler = None
        linker = None
        assembler = None
        fortran_compiler = None
        ar = None
    else:
        # Don't use g++ if the C compiler has built-in C++ support:
        if c_compiler in ('intelc', 'icc'):
            cxx_compiler = None
        else:
            cxx_compiler = SCons.Tool.FindTool(cxx_compilers, env) or cxx_compilers[0]
        linker = SCons.Tool.FindTool(linkers, env) or linkers[0]
        assembler = SCons.Tool.FindTool(assemblers, env) or assemblers[0]
        fortran_compiler = SCons.Tool.FindTool(fortran_compilers, env) or fortran_compilers[0]
        ar = SCons.Tool.FindTool(ars, env) or ars[0]

    other_tools = SCons.Tool.FindAllTools(other_plat_tools + [
                               'dmd',
                               #TODO: merge 'install' into 'filesystem' and
                               # make 'filesystem' the default
                               'filesystem',
                               'm4',
                               'wix', #'midl', 'msvs',
                               # Parser generators
                               'lex', 'yacc',
                               # Foreign function interface
                               'rpcgen', 'swig',
                               # Java
                               'jar', 'javac', 'javah', 'rmic',
                               # TeX
                               'dvipdf', 'dvips', 'gs',
                               'tex', 'latex', 'pdflatex', 'pdftex',
                               # Archivers
                               'tar', 'zip', 'rpm',
                               # SourceCode factories
                               'BitKeeper', 'CVS', 'Perforce',
                               'RCS', 'SCCS', # 'Subversion',
                               ], env)

    tools = ([linker, c_compiler, cxx_compiler,
              fortran_compiler, assembler, ar]
             + other_tools)

    return [x for x in tools if x]

def generate(env):
    """Add default tools."""
    if env['PLATFORM'] == "win32":
        for t in tool_list(env):
            SCons.Tool.Tool(t)(env)
    else:
        for t in SCons.Tool.tool_list(env['PLATFORM'], env):
            SCons.Tool.Tool(t)(env)

def exists(env):
    return 1

# Local Variables:
# tab-width:4
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=4 shiftwidth=4:
