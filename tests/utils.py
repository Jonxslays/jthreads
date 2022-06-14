import inspect
import typing
from pathlib import Path


def _strip_test_prefix(string: str) -> str:
    return string.replace("test_", "")


def setup(vars: typing.Dict[str, typing.Any]) -> None:
    functions = tuple(filter(lambda f: f.startswith("test_"), vars))

    for function in functions:
        vars[function]()


def do_assert(module: str, condition: bool, message: str = "") -> None:
    clean_module = _strip_test_prefix(Path(module).stem)
    function = _strip_test_prefix(inspect.stack()[2][3])
    assert condition, f"[{clean_module}-{function}] Test fail. {message}"


def do_success(module: str, message: str = "") -> None:
    clean_module = _strip_test_prefix(Path(module).stem)
    function = _strip_test_prefix(inspect.stack()[2][3])
    print(f"[{clean_module}-{function}] Test success. {message}")
