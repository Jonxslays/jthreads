import utils

_assert = lambda c, m="": utils.do_assert(__file__, c, m)
_success = lambda m="": utils.do_success(__file__, m)


def test_test() -> None:
    _assert(True, "Tests are broken...")
    _success()


if __name__ == "__main__":
    utils.setup(globals())
