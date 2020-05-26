/*
    discipline -- a Python extension module demonstrating the
    structured technique for correctly managing memory allocations to
    guard against memory leaks and double-frees.

    Copyright 2020 by Lawrence D'Oliveiro <ldo@geek-central.gen.nz>.
    This code is licensed CC0
    <https://creativecommons.org/publicdomain/zero/1.0/>; do with it
    what you will.
*/

#include <stdbool.h>
#include <iso646.h>
#include <stdio.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>

/*
    Useful stuff
*/

#define END_STRUCT_LIST {0}
  /* put at end of variable-length C arrays of structs; any new elements must
    be added before this. */
#define END_PTR_LIST 0
  /* put at end of variable-length C arrays of pointers; any new elements must
    be added before this. */

/* informational types to indicate that a pointer is “borrowing” its
  reference and doesn’t need to be disposed: */
typedef char
    br_char;
typedef PyObject
    br_PyObject;

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
    br_PyObject * items;
    const br_char * msg = NULL;
    do /*once*/
      {
        const bool parsed_ok = PyArg_ParseTuple(args, "Os", &items, &msg);
        if (not parsed_ok)
            break;
        fprintf(stdout, "makedict says: “%s”\n", msg);
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
            br_PyObject * const item = PyTuple_GetItem(items, i);
            if (item == NULL)
                break;
            if (not PyTuple_Check(item) or PyTuple_Size(item) != 2)
              {
                PyErr_SetString(PyExc_TypeError, "expecting a 2-tuple");
                break;
              } /*if*/
            br_PyObject * const first = PyTuple_GetItem(item, 0);
            if (first == NULL)
                break;
            br_PyObject * const second = PyTuple_GetItem(item, 1);
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
    Py_XDECREF(tempresult);
    return
        result;
  } /*discipline_makedict*/

/*
    Top level
*/

/* If your module defines custom objects like types, constants, exceptions etc,
  it is convenient to collect them in tables so they can be added to the module
  in a loop in the init routine (below). This reduces the repetitiveness of
  the init code, including the error recovery. */

static PyTypeObject * types[] = /* all types defined in this module */
  {
    &ExceptMe_type,
    END_PTR_LIST
  };

struct string_constant_entry
    {
        const char * name;
        const char * value;
    };
static const struct string_constant_entry string_constants[] =
  /* all string constants defined in this module (all purely gratuitous) */
  {
    {"ONE", "one"},
    {"TWO", "two"},
    END_STRUCT_LIST
  };

static PyMethodDef discipline_methods[] =
  {
    {"makedict", discipline_makedict, METH_VARARGS,
        "makedict(«tuple of pairs», «message»)\n"
        "displays a message and makes a dictionary from a tuple"
        " of (key, value) pairs. Raises an exception if any key"
        " or value is ExceptMe."
    },
    END_STRUCT_LIST
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
        for (PyTypeObject ** e = types;;)
          {
            if (*e == NULL)
                break;
            if (PyType_Ready(*e) < 0)
                break;
            if (PyModule_AddObject(modu, (**e).tp_name, (PyObject *)*e) < 0)
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
