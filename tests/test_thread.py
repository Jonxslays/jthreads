from jthreads import Thread

import utils

_assert = lambda c, m="": utils.do_assert(__file__, c, m)
_success = lambda m="": utils.do_success(__file__, m)


def _func(x: int, y: int, *, message: str) -> str:
    return f"{message}{x + y}"


def test_thread_init() -> None:
    t = Thread(
        _func,
        name=f"Thread",
        positional=(69000, 420),
        keyword={"message": "Poggers "},
    )

    _assert(t.name == f"Thread", "Thread name doesn't match")
    _assert(not t.started, "Not started, but started is true")
    _assert(not t.completed, "Not started, but completed is true")
    _assert(t.positional == (69000, 420), "Positional arg mismatch")
    _assert(t.keyword == {"message": "Poggers "}, "Keyword arg mismatch")

    # FIXME: This won't always return the value of the target func
    # t.start()
    _assert(t.started, "Complete, but started is false")
    _assert(t.completed, "Complete, but completed is false")
    _success()


def test_ok() -> None:
    _success()


if __name__ == "__main__":
    utils.setup(globals())
