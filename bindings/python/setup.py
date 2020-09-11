from pprint import pprint
import os
from pathlib import Path
import re

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext as build_ext_orig


def _float(x):
    x = x.replace('f', '')
    try:
        return float(x)
    except ValueError:
        return


def _python(x):
    try:
        return eval(x)
    except Exception:
        return


def parse_constants():
    d = {}
    r1 = re.compile(r'(VKY_[A-Za-z0-9\_]+)\s+=\s+([^\n\,]+)')
    r2 = re.compile(
        r'#define\s(VKY_[A-Za-z0-9_]+)[ ]+([*A-Za-z0-9\.,_ +-/]+)\n')
    r3 = re.compile(
        r'#define\s(VKY_[^ ]+)\s+VKY_CONST[_INT]*\([A-Za-z0-9_]+\s*,\s*([*A-Za-z0-9\.,_ +-/]+)\)')

    fns = ('constants.h', 'scene.h', 'app.h')
    for fn in fns:
        path = Path(__file__).parent / '../../include/visky' / fn
        constants = path.read_text()
        for r in (r1, r2, r3):
            for m in r.finditer(constants):
                k, v = m.group(1), m.group(2)
                if v.isdigit():
                    v = int(v)
                elif _float(v) is not None:
                    v = _float(v)
                elif _python(v) is not None:
                    v = _python(v)
                else:
                    v = f"'{v}'  # TODO"
                # TODO: handle VK_* header #define's
                d[k] = v
    # Write the Python constants.py file
    output = '# automatically-generated by setup.py -- DO NOT EDIT\n\n'
    output += '\n'.join(f'{k.replace("VKY_", "")} = {d[k]}' for k in sorted(d))
    output += '\n'
    (Path(__file__).parent / 'visky/_constants.py').write_text(output)


# see https://stackoverflow.com/a/48015772/1595060

class CMakeExtension(Extension):
    def __init__(self, name):
        # don't invoke the original build_ext for this special extension
        super(CMakeExtension, self).__init__(name, sources=[])


class build_ext(build_ext_orig):
    def run(self):
        for ext in self.extensions:
            self.build_cmake(ext)
        super().run()

    def build_cmake(self, ext):

        # Create visky/_constants.py with the VKY constant values extracted from constants.h
        parse_constants()

        cwd = Path().absolute()
        root_dir = (Path(__file__).parent / '../../').resolve()

        # these dirs will be created in build_py, so if you don't have
        # any python sources to bundle, the dirs will be missing
        build_temp = Path(self.build_temp)
        build_temp.mkdir(parents=True, exist_ok=True)
        extdir = Path(self.get_ext_fullpath(ext.name))
        extdir.mkdir(parents=True, exist_ok=True)

        # example of cmake args
        config = 'Debug'  # if self.debug else 'Release'
        cmake_args = [
            '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' +
            str(extdir.absolute()),
            '-DCMAKE_BUILD_TYPE=' + config
        ]

        # example of build args
        build_args = [
            '--config', config,
            '--', '-j4'
        ]

        os.chdir(str(build_temp))
        self.spawn(['cmake', str(root_dir)] + cmake_args)
        if not self.dry_run:
            self.spawn(['cmake', '--build', '.'] + build_args)
        # Troubleshooting: if fail on line above then delete all possible
        # temporary CMake files including "CMakeCache.txt" in top level dir.
        os.chdir(str(cwd))


setup(
    name='visky',
    version='0.0.0a0',
    description='Scientific visualization',
    author='Cyrille Rossant',
    author_email='rossant@users.noreply.github.com',
    url='https://visky.dev',
    long_description='''Scientific visualization''',
    packages=['visky'],
    ext_modules=[CMakeExtension('visky')],
    cmdclass={
        'build_ext': build_ext,
    }
)
