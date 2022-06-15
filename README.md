# jthreads

A threading implementation for Python written in C.

> NOTE: Work in progress - for fun, not production.

**WARNING**: *Currently a broken impl use at your own risk*.

## Trying it out

```bash
$ git clone https://github.com/Jonxslays/jthreads.git
$ cd jthreads

# Create a venv and install the module
$ python -m venv .venv
$ source .venv/bin/activate
$ python setup.py install
```

```py
# main.py
from jthreads import Thread

def add_and_print(a: int, b: int, *, message: str) -> None:
    print(f"{message}{a + b}")

t = Thread(
    add_and_print,
    name="Add",
    positional=(1, 2),
    keyword={"message": "The result is: "},
)

t.start()
# Outputs `The result is: 3`
```

## License

jthreads is licensed under the
[MIT License](https://github.com/Jonxslays/jthreads/blob/master/LICENSE).
