#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include <stdbool.h>

typedef struct {
    PyObject_HEAD;
    PyObject *target;
    PyObject *name;
    PyObject *positional;
    PyObject *keyword;
    bool started;
    bool completed;
} Thread;

static void thread_dealloc(Thread *self) {
    Py_XDECREF(self->target);
    Py_XDECREF(self->name);
    Py_XDECREF(self->positional);
    Py_XDECREF(self->keyword);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *thread_alloc(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
    Thread *self;

    self = (Thread *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->name = PyUnicode_FromString("");
        if (self->name == NULL) {
            Py_XDECREF(self);
            return NULL;
        }

        self->positional = PyTuple_New(0);
        if (self->positional == NULL) {
            Py_DECREF(self);
            return NULL;
        }

        self->keyword = PyDict_New();
        if (self->keyword == NULL) {
            Py_DECREF(self);
            return NULL;
        }

        self->started = false;
        self->completed = false;
        self->target = NULL;
    }

    return (PyObject *) self;
}

static PyObject *swap_and_gc(PyObject *old, PyObject *new) {
    PyObject *tmp = old;
    Py_INCREF(new);
    old = new;
    Py_XDECREF(tmp);
    return old;
}

static int thread_init(Thread *self, PyObject *args, PyObject *kwargs) {
    static char *keywords[] = {"target", "name", "positional", "keyword", NULL};
    PyObject *target = NULL, *name = NULL, *positional = NULL, *keyword = NULL;

    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "O|$O!O!O!:Thread.__init__", keywords,
        &target, &PyUnicode_Type, &name,
        &PyTuple_Type, &positional,
        &PyDict_Type, &keyword
    )) {
        return -1;
    }

    if (!PyCallable_Check(target)) {
        PyErr_SetString(PyExc_TypeError, "'target' must be a callable.");
        return -1;
    }

    self->target = swap_and_gc(self->target, target);
    if (name) self->name = swap_and_gc(self->name, name);
    if (positional) self->positional = swap_and_gc(self->positional, positional);
    if (keyword) self->keyword = swap_and_gc(self->keyword, keyword);

    return 0;
}

static PyMemberDef thread_members[] = {
    {"target", T_OBJECT_EX, offsetof(Thread, target), 0, "Target"},
    {"name", T_OBJECT_EX, offsetof(Thread, name), 0, "Name"},
    {"positional", T_OBJECT_EX, offsetof(Thread, positional), 0, "Positional Args"},
    {"keyword", T_OBJECT_EX, offsetof(Thread, keyword), 0, "Keyword Args"},
    {"started", T_BOOL, offsetof(Thread, started), 0, "Started"},
    {"completed", T_BOOL, offsetof(Thread, completed), 0, "Completed"},
    {NULL},
};

static PyObject *start(Thread *self) {
    self->started = true;

    // TODO: Use pthreads here

    PyObject *result = PyObject_Call(self->target, self->positional, self->keyword);
    if (result == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Function call failed");
        return NULL;
    }

    self->completed = true;
    return result;
}

static PyMethodDef thread_methods[] = {
    {"start", (PyCFunction) start, METH_NOARGS, "Start the thread."},
    {NULL},
};

static PyTypeObject ThreadType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "jthreads.Thread",
    .tp_doc = PyDoc_STR("A Thread!"),
    .tp_basicsize = sizeof(Thread),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = thread_alloc,
    .tp_init = (initproc) thread_init,
    .tp_dealloc = (destructor) thread_dealloc,
    .tp_members = thread_members,
    .tp_methods = thread_methods,
};

static PyModuleDef jthreadsModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "jthreads",
    .m_doc = "A threading implementation for Python written in C.",
    .m_size = -1,
};

PyMODINIT_FUNC PyInit_jthreads(void) {
    PyObject *module = PyModule_Create(&jthreadsModule);

    if (PyType_Ready(&ThreadType) < 0) return NULL;
    if (module == NULL) return NULL;

    Py_INCREF(&ThreadType);
    if (PyModule_AddObject(module, "Thread", (PyObject *) &ThreadType) < 0) {
        Py_DECREF(&ThreadType);
        Py_DECREF(module);
        return NULL;
    }

    return module;
}
