/*
    discipline -- a Python extension module demonstrating the
    structured technique for correctly managing memory allocations to
    guard against memory leaks and double-frees.

    Copyright 2020 by Lawrence D'Oliveiro <ldo@geek-central.gen.nz>.
    This code is licensed CC0
    <https://creativecommons.org/publicdomain/zero/1.0/>; do with it
    what you will.
*/

#include <iso646.h>
#include <stdio.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>

/*
    Useful stuff
*/

#define ARRAY_END {NULL}
  /* marks end of variable-length array of structs. All of these have
    a name field as the first field. */

/*
    Types
*/

static PyTypeObject ExceptMe_type = /* really just a dummy */
    {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "ExceptMe",
        .tp_basicsize = /*sizeof(something)*/ 0,
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_doc = "sentinel used to trigger exception in makedict",
    };

/*
    Methods
*/

static PyObject * discipline_makedict
  (
    PyObject * self,
    PyObject * args
  )
  {
    PyObject * result = NULL;
    PyObject * tempresult = NULL;
    PyObject * items = NULL;
    const char * msg = NULL;
    do /*once*/
      {
        const bool parsed_ok = PyArg_ParseTuple(args, "Os", &items, &msg);
      /* Grab references to possible partial results from PyArg_Parse routine
        before checking for parse error.
        Strictly, I don’t need to keep my own references to individual arguments,
        since they are referenced by the argument list itself. But this way I have
        fewer exceptions to the convention that every object pointer I maintain
        should be reference-counted. */
        Py_XINCREF(items);
        if (not parsed_ok)
            break;
        fputs("makedict says: ", stdout);
        fputs(msg, stdout);
        fputs("\n", stdout);
        if (not PyTuple_Check(items))
          {
            PyErr_SetString(PyExc_TypeError, "expecting a tuple");
            break;
          } /*if*/
        const ssize_t nr_items = PyTuple_Size(items);
        if (PyErr_Occurred())
            break;
        tempresult = PyDict_New();
        if (tempresult == NULL)
            break;
        for (ssize_t i = 0;;)
          {
            if (i == nr_items)
                break;
            PyObject * const item = PyTuple_GetItem(items, i);
            if (item == NULL)
                break;
            if (not PyTuple_Check(item) or PyTuple_Size(item) != 2)
              {
                PyErr_SetString(PyExc_TypeError, "expecting a 2-tuple");
                break;
              } /*if*/
            PyObject * const first = PyTuple_GetItem(item, 0);
            if (first == NULL)
                break;
            PyObject * const second = PyTuple_GetItem(item, 1);
            if (second == NULL)
                break;
            if (first == (PyObject *)&ExceptMe_type or second == (PyObject *)&ExceptMe_type)
              {
                PyErr_SetString(PyExc_ValueError, "ExceptMe object found");
                break;
              } /*if*/
            if (PyDict_SetItem(tempresult, first, second) < 0)
                break;
            ++i;
          } /*for*/
        if (PyErr_Occurred())
            break;
      /* all done */
        result = tempresult;
        tempresult = NULL; /* so I don’t dispose of it yet */
      }
    while (false);
    Py_XDECREF(items);
    Py_XDECREF(tempresult);
    return
        result;
  } /*discipline_makedict*/

/*
    Top level
*/

struct type_entry
    {
	    const char * name;
	    PyTypeObject * typeobj;
    };
static const struct type_entry types[] =
  {
    {"ExceptMe", &ExceptMe_type},
    ARRAY_END
  };

struct string_constant_entry
    {
        const char * name;
        const char * value;
    };
static const struct string_constant_entry string_constants[] =
  {
    {"ONE", "one"},
    {"TWO", "two"},
    ARRAY_END
  };

static PyMethodDef discipline_methods[] =
  {
    {"makedict", discipline_makedict, METH_VARARGS,
        "makes a dictionary from a tuple of (key, value) pairs."
        " Raises an exception if any key or value is ExceptMe."
    },
    ARRAY_END
  };

static PyModuleDef discipline_module =
  {
    PyModuleDef_HEAD_INIT,
    "discipline", /* module name */
    "demonstration of structured discipline", /* docstring */
    -1, /* size of per-interpreter state, -1 if entirely global */
    discipline_methods,
  };

PyMODINIT_FUNC PyInit_discipline(void)
  {
    PyObject * result = NULL;
    PyObject * modu = NULL;
    do /*once*/
      {
        modu = PyModule_Create(&discipline_module);
        if (PyErr_Occurred())
            break;
        for (const struct type_entry *e = types;;)
          {
            if (e->name == NULL)
                break;
            if (PyType_Ready(e->typeobj) < 0)
                break;
            if (PyModule_AddObject(modu, e->name, (PyObject *)e->typeobj) < 0)
                break;
            ++e;
          } /*for*/
        if (PyErr_Occurred())
            break;
        for (const struct string_constant_entry *e = string_constants;;)
          {
            if (e->name == NULL)
                break;
            if (PyModule_AddStringConstant(modu, e->name, e->value) < 0)
                break;
            ++e;
          } /*for*/
        if (PyErr_Occurred())
            break;
      /* all done */
        result = modu;
        modu = NULL; /* so I don’t dispose of it yet */
      }
    while (false);
    Py_XDECREF(modu);
    return
        result;
  } /*PyInit_discipline*/
