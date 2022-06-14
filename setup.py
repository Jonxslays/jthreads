from setuptools import setup, Extension

def main():
    setup(
        name="jthreads",
        version="0.1.0",
        description="A threading implementation for Python written in C.",
        author="Jonxslays",
        ext_modules=[Extension("jthreads", ["src/jthreads.c"])],
    )

if __name__ == "__main__":
    main()
