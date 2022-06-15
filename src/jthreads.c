#define PY_SSIZE_T_CLEAN
#include <pthread.h>
#include <Python.h>
#include <stdbool.h>
#include "structmember.h"

typedef struct {
    PyObject_HEAD;
    PyObject *target;
    PyObject *name;
    PyObject *positional;
    PyObject *keyword;
    pthread_t thread;
    bool started;
    bool completed;
} Thread;

// FIXME: Figure out how to hook into the GIL
// If we try to start more than 1 thread it dies :(

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
        self->thread = (pthread_t) NULL;
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

// FIXME: These members can be changed by end users, not great.
// This is C, not python!

static PyMemberDef thread_members[] = {
    {"target", T_OBJECT_EX, offsetof(Thread, target), 0, "Target"},
    {"name", T_OBJECT_EX, offsetof(Thread, name), 0, "Name"},
    {"positional", T_OBJECT_EX, offsetof(Thread, positional), 0, "Positional Args"},
    {"keyword", T_OBJECT_EX, offsetof(Thread, keyword), 0, "Keyword Args"},
    {"thread", T_ULONG, offsetof(Thread, thread), 0, "Posix Thread"},
    {"started", T_BOOL, offsetof(Thread, started), 0, "Started"},
    {"completed", T_BOOL, offsetof(Thread, completed), 0, "Completed"},
    {NULL},
};

void *handle_thread_target(void *thread_p) {
    Thread *thread = (Thread *) thread_p;

    PyObject *result = PyObject_Call(thread->target, thread->positional, thread->keyword);
    if (result == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Thread target function call failed");
        return NULL;
    }

    thread->completed = true;
    return (void *) result;
}

static PyObject *thread_start(Thread *self) {
    if (self->started) {
        // Imagine
        PyErr_SetString(PyExc_RuntimeError, "You can only start a thread once");
        return NULL;
    }

    // TODO: Get the resulting PyObject *value from the thread

    self->started = true;
    int success = pthread_create(&self->thread, NULL, handle_thread_target, (void *) self);
    if (success) {
        // Thread creation failed
        PyErr_SetString(PyExc_RuntimeError, "Failed to create new thread");
        return NULL;
    }

    return Py_None;
}

static PyObject *thread_join(Thread *self) {
    if (!self->started) {
        // Cant join a party that doesn't exist
        PyErr_SetString(PyExc_RuntimeError, "You must start the thread to join it");
        return NULL;
    }

    int success = pthread_join(self->thread, NULL);
    if (success) {
        // Thread creation failed
        PyErr_SetString(PyExc_RuntimeError, "Failed to join thread");
        return NULL;
    }

    return Py_None;
}

static PyMethodDef thread_methods[] = {
    {"start", (PyCFunction) thread_start, METH_NOARGS, "Start the thread."},
    {"join", (PyCFunction) thread_join, METH_NOARGS, "Join the thread."},
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
