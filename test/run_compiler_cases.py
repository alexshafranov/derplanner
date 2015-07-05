# Copyright (c) 2015 Alexander Shafranov shafranov@gmail.com
# 
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
# claim that you wrote the original software. If you use this software
# in a product, an acknowledgment in the product documentation would be
# appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
# misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

import os
import glob
import types
import unittest
import functools
import subprocess
import tempfile
import shutil

class Main(unittest.TestCase):
    pass

def read_expected(domain_path):
    result = []
    for line in open(domain_path, 'rb'):
        if line.startswith('//>'):
            result += [line.lstrip('//>').strip()]
    return result

def make_test(domain_path):

    def test(self):
        expected = read_expected(domain_path)
        exe = os.path.join('bin', 'x64', 'debug', 'derplannerc')
        temp_dir = tempfile.mkdtemp()
        p = subprocess.Popen(args=[exe, '-o', temp_dir, domain_path], stderr=subprocess.PIPE)
        (stdoutdata, stderrdata) = p.communicate()
        shutil.rmtree(temp_dir)
        actual = [line.strip() for line in stderrdata.splitlines()]
        self.assertEqual(expected, actual)

    return test

def generate(base_path):
    diagnostic_domains = glob.glob(os.path.join(base_path, 'sema-*.domain')) + glob.glob(os.path.join(base_path, 'syntax-*.domain'))
    for domain_path in diagnostic_domains:
        setattr(Main, 'test#' + domain_path, make_test(domain_path))


if __name__ == '__main__':
    generate(base_path=os.path.join(os.path.dirname(__file__), 'cases'))
    unittest.main()
