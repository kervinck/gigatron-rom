import pathlib

from setuptools import setup

here = pathlib.Path(__file__).parent.resolve()

long_description = (here / "README.md").read_text(encoding="utf-8")

REPO_ROOT = (pathlib.Path(__file__).parent / ".." / ".." / "..").resolve()

setup(
    name="py-gtemu",
    version="0.1a20201211",
    setup_requires=["cffi>=1.0.0"],
    cffi_modules=["gtemu_extension_build.py:ffibuilder"],
    install_requires=["cffi>=1.0.0"],
    python_requires=">=3.6, <4",
    py_modules=["gtemu"],
)
