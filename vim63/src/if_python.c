/* vi:set ts=8 sts=4 sw=4:
 *
 * VIM - Vi IMproved	by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */
/*
 * Python extensions by Paul Moore.
 * Changes for Unix by David Leonard.
 *
 * This consists of four parts:
 * 1. Python interpreter main program
 * 2. Python output stream: writes output via [e]msg().
 * 3. Implementation of the Vim module for Python
 * 4. Utility functions for handling the interface between Vim and Python.
 */

#include "vim.h"

#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

/* Python.h defines _POSIX_THREADS itself (if needed) */
#ifdef _POSIX_THREADS
# undef _POSIX_THREADS
#endif

#if defined(_WIN32) && defined (HAVE_FCNTL_H)
# undef HAVE_FCNTL_H
#endif

#ifdef _DEBUG
# undef _DEBUG
#endif

#ifdef HAVE_STDARG_H
# undef HAVE_STDARG_H	/* Python's config.h defines it as well. */
#endif

#include <Python.h>
#if defined(MACOS) && !defined(MACOS_X_UNIX)
# include "macglue.h"
# include <CodeFragments.h>
#endif
#undef main /* Defined in python.h - aargh */
#undef HAVE_FCNTL_H /* Clash with os_win32.h */

#if !defined(FEAT_PYTHON) && defined(PROTO)
/* Use this to be able to generate prototypes without python being used. */
# define PyObject int
# define PyThreadState int
# define PyTypeObject int
struct PyMethodDef { int a; };
# define PySequenceMethods int
#endif

/* Parser flags */
#define single_input	256
#define file_input	257
#define eval_input	258

#if defined(PY_VERSION_HEX) && PY_VERSION_HEX >= 0x020300F0
  /* Python 2.3: can invoke ":python" recursively. */
# define PY_CAN_RECURSE
#endif

#if defined(DYNAMIC_PYTHON) || defined(PROTO)
# ifndef DYNAMIC_PYTHON
#  define HINSTANCE int		/* for generating prototypes */
# endif

/*
 * Wrapper defines
 */
# define PyArg_Parse dll_PyArg_Parse
# define PyArg_ParseTuple dll_PyArg_ParseTuple
# define PyDict_SetItemString dll_PyDict_SetItemString
# define PyErr_BadArgument dll_PyErr_BadArgument
# define PyErr_Clear dll_PyErr_Clear
# define PyErr_NoMemory dll_PyErr_NoMemory
# define PyErr_Occurred dll_PyErr_Occurred
# define PyErr_SetNone dll_PyErr_SetNone
# define PyErr_SetString dll_PyErr_SetString
# define PyEval_InitThreads dll_PyEval_InitThreads
# define PyEval_RestoreThread dll_PyEval_RestoreThread
# define PyEval_SaveThread dll_PyEval_SaveThread
# ifdef PY_CAN_RECURSE
#  define PyGILState_Ensure dll_PyGILState_Ensure
#  define PyGILState_Release dll_PyGILState_Release
# endif
# define PyInt_AsLong dll_PyInt_AsLong
# define PyInt_FromLong dll_PyInt_FromLong
# define PyInt_Type (*dll_PyInt_Type)
# define PyList_GetItem dll_PyList_GetItem
# define PyList_New dll_PyList_New
# define PyList_SetItem dll_PyList_SetItem
# define PyList_Size dll_PyList_Size
# define PyList_Type (*dll_PyList_Type)
# define PyImport_ImportModule dll_PyImport_ImportModule
# define PyDict_GetItemString dll_PyDict_GetItemString
# define PyModule_GetDict dll_PyModule_GetDict
# define PyRun_SimpleString dll_PyRun_SimpleString
# define PyString_AsString dll_PyString_AsString
# define PyString_FromString dll_PyString_FromString
# define PyString_FromStringAndSize dll_PyString_FromStringAndSize
# define PyString_Size dll_PyString_Size
# define PyString_Type (*dll_PyString_Type)
# define PySys_SetObject dll_PySys_SetObject
# define PySys_SetArgv dll_PySys_SetArgv
# define PyType_Type (*dll_PyType_Type)
# define Py_BuildValue dll_Py_BuildValue
# define Py_FindMethod dll_Py_FindMethod
# define Py_InitModule4 dll_Py_InitModule4
# define Py_Initialize dll_Py_Initialize
# define _PyObject_New dll__PyObject_New
# define _Py_NoneStruct (*dll__Py_NoneStruct)
# define PyObject_Init dll__PyObject_Init
# if defined(PY_VERSION_HEX) && PY_VERSION_HEX >= 0x02020000
#  define PyType_IsSubtype dll_PyType_IsSubtype
# endif
# if defined(PY_VERSION_HEX) && PY_VERSION_HEX >= 0x02030000
#  define PyObject_Malloc dll_PyObject_Malloc
#  define PyObject_Free dll_PyObject_Free
# endif

/*
 * Pointers for dynamic link
 */
static int(*dll_PyArg_Parse)(PyObject *, char *, ...);
static int(*dll_PyArg_ParseTuple)(PyObject *, char *, ...);
static int(*dll_PyDict_SetItemString)(PyObject *dp, char *key, PyObject *item);
static int(*dll_PyErr_BadArgument)(void);
static void(*dll_PyErr_Clear)(void);
static PyObject*(*dll_PyErr_NoMemory)(void);
static PyObject*(*dll_PyErr_Occurred)(void);
static void(*dll_PyErr_SetNone)(PyObject *);
static void(*dll_PyErr_SetString)(PyObject *, const char *);
static void(*dll_PyEval_InitThreads)(void);
static void(*dll_PyEval_RestoreThread)(PyThreadState *);
static PyThreadState*(*dll_PyEval_SaveThread)(void);
# ifdef PY_CAN_RECURSE
static PyGILState_STATE	(*dll_PyGILState_Ensure)(void);
static void (*dll_PyGILState_Release)(PyGILState_STATE);
#endif
static long(*dll_PyInt_AsLong)(PyObject *);
static PyObject*(*dll_PyInt_FromLong)(long);
static PyTypeObject* dll_PyInt_Type;
static PyObject*(*dll_PyList_GetItem)(PyObject *, int);
static PyObject*(*dll_PyList_New)(int size);
static int(*dll_PyList_SetItem)(PyObject *, int, PyObject *);
static int(*dll_PyList_Size)(PyObject *);
static PyTypeObject* dll_PyList_Type;
static PyObject*(*dll_PyImport_ImportModule)(const char *);
static PyObject*(*dll_PyDict_GetItemString)(PyObject *, const char *);
static PyObject*(*dll_PyModule_GetDict)(PyObject *);
static int(*dll_PyRun_SimpleString)(char *);
static char*(*dll_PyString_AsString)(PyObject *);
static PyObject*(*dll_PyString_FromString)(const char *);
static PyObject*(*dll_PyString_FromStringAndSize)(const char *, int);
static int(*dll_PyString_Size)(PyObject *);
static PyTypeObject* dll_PyString_Type;
static int(*dll_PySys_SetObject)(char *, PyObject *);
static int(*dll_PySys_SetArgv)(int, char **);
static PyTypeObject* dll_PyType_Type;
static PyObject*(*dll_Py_BuildValue)(char *, ...);
static PyObject*(*dll_Py_FindMethod)(struct PyMethodDef[], PyObject *, char *);
static PyObject*(*dll_Py_InitModule4)(char *, struct PyMethodDef *, char *, PyObject *, int);
static void(*dll_Py_Initialize)(void);
static PyObject*(*dll__PyObject_New)(PyTypeObject *, PyObject *);
static PyObject*(*dll__PyObject_Init)(PyObject *, PyTypeObject *);
static PyObject* dll__Py_NoneStruct;
# if defined(PY_VERSION_HEX) && PY_VERSION_HEX >= 0x02020000
static int (*dll_PyType_IsSubtype)(PyTypeObject *, PyTypeObject *);
# endif
# if defined(PY_VERSION_HEX) && PY_VERSION_HEX >= 0x02030000
static void* (*dll_PyObject_Malloc)(size_t);
static void (*dll_PyObject_Free)(void*);
# endif

static HINSTANCE hinstPython = 0; /* Instance of python.dll */

/* Imported exception objects */
static PyObject *imp_PyExc_AttributeError;
static PyObject *imp_PyExc_IndexError;
static PyObject *imp_PyExc_KeyboardInterrupt;
static PyObject *imp_PyExc_TypeError;
static PyObject *imp_PyExc_ValueError;

# define PyExc_AttributeError imp_PyExc_AttributeError
# define PyExc_IndexError imp_PyExc_IndexError
# define PyExc_KeyboardInterrupt imp_PyExc_KeyboardInterrupt
# define PyExc_TypeError imp_PyExc_TypeError
# define PyExc_ValueError imp_PyExc_ValueError

/*
 * Table of name to function pointer of python.
 */
# define PYTHON_PROC FARPROC
static struct
{
    char *name;
    PYTHON_PROC *ptr;
} python_funcname_table[] =
{
    {"PyArg_Parse", (PYTHON_PROC*)&dll_PyArg_Parse},
    {"PyArg_ParseTuple", (PYTHON_PROC*)&dll_PyArg_ParseTuple},
    {"PyDict_SetItemString", (PYTHON_PROC*)&dll_PyDict_SetItemString},
    {"PyErr_BadArgument", (PYTHON_PROC*)&dll_PyErr_BadArgument},
    {"PyErr_Clear", (PYTHON_PROC*)&dll_PyErr_Clear},
    {"PyErr_NoMemory", (PYTHON_PROC*)&dll_PyErr_NoMemory},
    {"PyErr_Occurred", (PYTHON_PROC*)&dll_PyErr_Occurred},
    {"PyErr_SetNone", (PYTHON_PROC*)&dll_PyErr_SetNone},
    {"PyErr_SetString", (PYTHON_PROC*)&dll_PyErr_SetString},
    {"PyEval_InitThreads", (PYTHON_PROC*)&dll_PyEval_InitThreads},
    {"PyEval_RestoreThread", (PYTHON_PROC*)&dll_PyEval_RestoreThread},
    {"PyEval_SaveThread", (PYTHON_PROC*)&dll_PyEval_SaveThread},
# ifdef PY_CAN_RECURSE
    {"PyGILState_Ensure", (PYTHON_PROC*)&dll_PyGILState_Ensure},
    {"PyGILState_Release", (PYTHON_PROC*)&dll_PyGILState_Release},
# endif
    {"PyInt_AsLong", (PYTHON_PROC*)&dll_PyInt_AsLong},
    {"PyInt_FromLong", (PYTHON_PROC*)&dll_PyInt_FromLong},
    {"PyInt_Type", (PYTHON_PROC*)&dll_PyInt_Type},
    {"PyList_GetItem", (PYTHON_PROC*)&dll_PyList_GetItem},
    {"PyList_New", (PYTHON_PROC*)&dll_PyList_New},
    {"PyList_SetItem", (PYTHON_PROC*)&dll_PyList_SetItem},
    {"PyList_Size", (PYTHON_PROC*)&dll_PyList_Size},
    {"PyList_Type", (PYTHON_PROC*)&dll_PyList_Type},
    {"PyImport_ImportModule", (PYTHON_PROC*)&dll_PyImport_ImportModule},
    {"PyDict_GetItemString", (PYTHON_PROC*)&dll_PyDict_GetItemString},
    {"PyModule_GetDict", (PYTHON_PROC*)&dll_PyModule_GetDict},
    {"PyRun_SimpleString", (PYTHON_PROC*)&dll_PyRun_SimpleString},
    {"PyString_AsString", (PYTHON_PROC*)&dll_PyString_AsString},
    {"PyString_FromString", (PYTHON_PROC*)&dll_PyString_FromString},
    {"PyString_FromStringAndSize", (PYTHON_PROC*)&dll_PyString_FromStringAndSize},
    {"PyString_Size", (PYTHON_PROC*)&dll_PyString_Size},
    {"PyString_Type", (PYTHON_PROC*)&dll_PyString_Type},
    {"PySys_SetObject", (PYTHON_PROC*)&dll_PySys_SetObject},
    {"PySys_SetArgv", (PYTHON_PROC*)&dll_PySys_SetArgv},
    {"PyType_Type", (PYTHON_PROC*)&dll_PyType_Type},
    {"Py_BuildValue", (PYTHON_PROC*)&dll_Py_BuildValue},
    {"Py_FindMethod", (PYTHON_PROC*)&dll_Py_FindMethod},
    {"Py_InitModule4", (PYTHON_PROC*)&dll_Py_InitModule4},
    {"Py_Initialize", (PYTHON_PROC*)&dll_Py_Initialize},
    {"_PyObject_New", (PYTHON_PROC*)&dll__PyObject_New},
    {"PyObject_Init", (PYTHON_PROC*)&dll__PyObject_Init},
    {"_Py_NoneStruct", (PYTHON_PROC*)&dll__Py_NoneStruct},
# if defined(PY_VERSION_HEX) && PY_VERSION_HEX >= 0x02020000
    {"PyType_IsSubtype", (PYTHON_PROC*)&dll_PyType_IsSubtype},
# endif
# if defined(PY_VERSION_HEX) && PY_VERSION_HEX >= 0x02030000
    {"PyObject_Malloc", (PYTHON_PROC*)&dll_PyObject_Malloc},
    {"PyObject_Free", (PYTHON_PROC*)&dll_PyObject_Free},
# endif
    {"", NULL},
};

/*
 * Free python.dll
 */
    static void
end_dynamic_python(void)
{
    if (hinstPython)
    {
	FreeLibrary(hinstPython);
	hinstPython = 0;
    }
}

/*
 * Load library and get all pointers.
 * Parameter 'libname' provides name of DLL.
 * Return OK or FAIL.
 */
    static int
python_runtime_link_init(char *libname, int verbose)
{
    int i;

    if (hinstPython)
	return OK;
    hinstPython = LoadLibrary(libname);
    if (!hinstPython)
    {
	if (verbose)
	    EMSG2(_(e_loadlib), libname);
	return FAIL;
    }

    for (i = 0; python_funcname_table[i].ptr; ++i)
    {
	if ((*python_funcname_table[i].ptr = GetProcAddress(hinstPython,
			python_funcname_table[i].name)) == NULL)
	{
	    FreeLibrary(hinstPython);
	    hinstPython = 0;
	    if (verbose)
		EMSG2(_(e_loadfunc), python_funcname_table[i].name);
	    return FAIL;
	}
    }
    return OK;
}

/*
 * If python is enabled (there is installed python on Windows system) return
 * TRUE, else FALSE.
 */
    int
python_enabled(verbose)
    int		verbose;
{
    return python_runtime_link_init(DYNAMIC_PYTHON_DLL, verbose) == OK;
}

/* Load the standard Python exceptions - don't import the symbols from the
 * DLL, as this can cause errors (importing data symbols is not reliable).
 */
static void get_exceptions __ARGS((void));

    static void
get_exceptions()
{
    PyObject *exmod = PyImport_ImportModule("exceptions");
    PyObject *exdict = PyModule_GetDict(exmod);
    imp_PyExc_AttributeError = PyDict_GetItemString(exdict, "AttributeError");
    imp_PyExc_IndexError = PyDict_GetItemString(exdict, "IndexError");
    imp_PyExc_KeyboardInterrupt = PyDict_GetItemString(exdict, "KeyboardInterrupt");
    imp_PyExc_TypeError = PyDict_GetItemString(exdict, "TypeError");
    imp_PyExc_ValueError = PyDict_GetItemString(exdict, "ValueError");
    Py_XINCREF(imp_PyExc_AttributeError);
    Py_XINCREF(imp_PyExc_IndexError);
    Py_XINCREF(imp_PyExc_KeyboardInterrupt);
    Py_XINCREF(imp_PyExc_TypeError);
    Py_XINCREF(imp_PyExc_ValueError);
    Py_XDECREF(exmod);
}
#endif /* DYNAMIC_PYTHON */

/******************************************************
 * Internal function prototypes.
 */

static void DoPythonCommand(exarg_T *, const char *);
static int RangeStart;
static int RangeEnd;

static void PythonIO_Flush(void);
static int PythonIO_Init(void);
static int PythonMod_Init(void);

/* Utility functions for the vim/python interface
 * ----------------------------------------------
 */
static PyObject *GetBufferLine(buf_T *, int);
static PyObject *GetBufferLineList(buf_T *, int, int);

static int SetBufferLine(buf_T *, int, PyObject *, int *);
static int SetBufferLineList(buf_T *, int, int, PyObject *, int *);
static int InsertBufferLines(buf_T *, int, PyObject *, int *);

static PyObject *LineToString(const char *);
static char *StringToLine(PyObject *);

static int VimErrorCheck(void);

#define PyErr_SetVim(str) PyErr_SetString(VimError, str)

/******************************************************
 * 1. Python interpreter main program.
 */

static int initialised = 0;

#if PYTHON_API_VERSION < 1007 /* Python 1.4 */
typedef PyObject PyThreadState;
#endif /* Python 1.4 */

#ifndef PY_CAN_RECURSE
static PyThreadState* saved_python_thread = NULL;

/*
 * Suspend a thread of the Python interpreter, other threads are allowed to
 * run.
 */
static void Python_SaveThread(void)
{
    saved_python_thread = PyEval_SaveThread();
}

/*
 * Restore a thread of the Python interpreter, waits for other threads to
 * block.
 */
static void Python_RestoreThread(void)
{
    PyEval_RestoreThread(saved_python_thread);
    saved_python_thread = NULL;
}
#endif

/*
 * obtain a lock on the Vim data structures
 */
static void Python_Lock_Vim(void)
{
}

/*
 * release a lock on the Vim data structures
 */
static void Python_Release_Vim(void)
{
}

    void
python_end()
{
#ifdef DYNAMIC_PYTHON
    end_dynamic_python();
#endif
}

    static int
Python_Init(void)
{
    if (!initialised)
    {
#ifdef DYNAMIC_PYTHON
	if (!python_enabled(TRUE))
	{
	    EMSG(_("E263: Sorry, this command is disabled, the Python library could not be loaded."));
	    goto fail;
	}
#endif

#if !defined(MACOS) || defined(MACOS_X_UNIX)
	Py_Initialize();
#else
	PyMac_Initialize();
#endif
	/* initialise threads */
	PyEval_InitThreads();

#ifdef DYNAMIC_PYTHON
	get_exceptions();
#endif

	if (PythonIO_Init())
	    goto fail;

	if (PythonMod_Init())
	    goto fail;

#ifndef PY_CAN_RECURSE
	/* the first python thread is vim's */
	Python_SaveThread();
#endif

	initialised = 1;
    }

    return 0;

fail:
    /* We call PythonIO_Flush() here to print any Python errors.
     * This is OK, as it is possible to call this function even
     * if PythonIO_Init() has not completed successfully (it will
     * not do anything in this case).
     */
    PythonIO_Flush();
    return -1;
}

/*
 * External interface
 */
    static void
DoPythonCommand(exarg_T *eap, const char *cmd)
{
#ifdef PY_CAN_RECURSE
    PyGILState_STATE	pygilstate;
#else
    static int		recursive = 0;
#endif
#if defined(MACOS) && !defined(MACOS_X_UNIX)
    GrafPtr		oldPort;
#endif
#if defined(HAVE_LOCALE_H) || defined(X_LOCALE)
    char		*saved_locale;
#endif

#ifndef PY_CAN_RECURSE
    if (recursive)
    {
	EMSG(_("E659: Cannot invoke Python recursively"));
	return;
    }
    ++recursive;
#endif

#if defined(MACOS) && !defined(MACOS_X_UNIX)
    GetPort(&oldPort);
    /* Check if the Python library is available */
    if ((Ptr)PyMac_Initialize == (Ptr)kUnresolvedCFragSymbolAddress)
	goto theend;
#endif
    if (Python_Init())
	goto theend;

    RangeStart = eap->line1;
    RangeEnd = eap->line2;
    Python_Release_Vim();	    /* leave vim */

#if defined(HAVE_LOCALE_H) || defined(X_LOCALE)
    /* Python only works properly when the LC_NUMERIC locale is "C". */
    saved_locale = setlocale(LC_NUMERIC, NULL);
    if (saved_locale == NULL || STRCMP(saved_locale, "C") == 0)
	saved_locale = NULL;
    else
    {
	/* Need to make a copy, value may change when setting new locale. */
	saved_locale = (char *)vim_strsave((char_u *)saved_locale);
	(void)setlocale(LC_NUMERIC, "C");
    }
#endif

#ifdef PY_CAN_RECURSE
    pygilstate = PyGILState_Ensure();
#else
    Python_RestoreThread();	    /* enter python */
#endif

    PyRun_SimpleString((char *)(cmd));

#ifdef PY_CAN_RECURSE
    PyGILState_Release(pygilstate);
#else
    Python_SaveThread();	    /* leave python */
#endif

#if defined(HAVE_LOCALE_H) || defined(X_LOCALE)
    if (saved_locale != NULL)
    {
	(void)setlocale(LC_NUMERIC, saved_locale);
	vim_free(saved_locale);
    }
#endif

    Python_Lock_Vim();		    /* enter vim */
    PythonIO_Flush();
#if defined(MACOS) && !defined(MACOS_X_UNIX)
    SetPort(oldPort);
#endif

theend:
#ifndef PY_CAN_RECURSE
    --recursive;
#endif
    return;	    /* keeps lint happy */
}

/*
 * ":python"
 */
    void
ex_python(exarg_T *eap)
{
    char_u *script;

    script = script_get(eap, eap->arg);
    if (!eap->skip)
    {
	if (script == NULL)
	    DoPythonCommand(eap, (char *)eap->arg);
	else
	    DoPythonCommand(eap, (char *)script);
    }
    vim_free(script);
}

#define BUFFER_SIZE 1024

/*
 * ":pyfile"
 */
    void
ex_pyfile(exarg_T *eap)
{
    static char buffer[BUFFER_SIZE];
    const char *file = (char *)eap->arg;
    char *p;

    /* Have to do it like this. PyRun_SimpleFile requires you to pass a
     * stdio file pointer, but Vim and the Python DLL are compiled with
     * different options under Windows, meaning that stdio pointers aren't
     * compatible between the two. Yuk.
     *
     * Put the string "execfile('file')" into buffer. But, we need to
     * escape any backslashes or single quotes in the file name, so that
     * Python won't mangle the file name.
     */
    strcpy(buffer, "execfile('");
    p = buffer + 10; /* size of "execfile('" */

    while (*file && p < buffer + (BUFFER_SIZE - 3))
    {
	if (*file == '\\' || *file == '\'')
	    *p++ = '\\';
	*p++ = *file++;
    }

    /* If we didn't finish the file name, we hit a buffer overflow */
    if (*file != '\0')
	return;

    /* Put in the terminating "')" and a null */
    *p++ = '\'';
    *p++ = ')';
    *p++ = '\0';

    /* Execute the file */
    DoPythonCommand(eap, buffer);
}

/******************************************************
 * 2. Python output stream: writes output via [e]msg().
 */

/* Implementation functions
 */

static PyObject *OutputGetattr(PyObject *, char *);
static int OutputSetattr(PyObject *, char *, PyObject *);

static PyObject *OutputWrite(PyObject *, PyObject *);
static PyObject *OutputWritelines(PyObject *, PyObject *);

typedef void (*writefn)(char_u *);
static void writer(writefn fn, char_u *str, int n);

/* Output object definition
 */

typedef struct
{
    PyObject_HEAD
    long softspace;
    long error;
} OutputObject;

static struct PyMethodDef OutputMethods[] = {
    /* name,	    function,		calling,    documentation */
    {"write",	    OutputWrite,	1,	    "" },
    {"writelines",  OutputWritelines,	1,	    "" },
    { NULL,	    NULL,		0,	    NULL }
};

static PyTypeObject OutputType = {
	PyObject_HEAD_INIT(0)
	0,
	"message",
	sizeof(OutputObject),
	0,

	(destructor) 0,
	(printfunc) 0,
	(getattrfunc) OutputGetattr,
	(setattrfunc) OutputSetattr,
	(cmpfunc) 0,
	(reprfunc) 0,

	0, /* as number */
	0, /* as sequence */
	0, /* as mapping */

	(hashfunc) 0,
	(ternaryfunc) 0,
	(reprfunc) 0
};

/*************/

    static PyObject *
OutputGetattr(PyObject *self, char *name)
{
    if (strcmp(name, "softspace") == 0)
	return PyInt_FromLong(((OutputObject *)(self))->softspace);

    return Py_FindMethod(OutputMethods, self, name);
}

    static int
OutputSetattr(PyObject *self, char *name, PyObject *val)
{
    if (val == NULL) {
	PyErr_SetString(PyExc_AttributeError, _("can't delete OutputObject attributes"));
	return -1;
    }

    if (strcmp(name, "softspace") == 0)
    {
	if (!PyInt_Check(val)) {
	    PyErr_SetString(PyExc_TypeError, _("softspace must be an integer"));
	    return -1;
	}

	((OutputObject *)(self))->softspace = PyInt_AsLong(val);
	return 0;
    }

    PyErr_SetString(PyExc_AttributeError, _("invalid attribute"));
    return -1;
}

/*************/

    static PyObject *
OutputWrite(PyObject *self, PyObject *args)
{
    int len;
    char *str;
    int error = ((OutputObject *)(self))->error;

    if (!PyArg_ParseTuple(args, "s#", &str, &len))
	return NULL;

    Py_BEGIN_ALLOW_THREADS
    Python_Lock_Vim();
    writer((writefn)(error ? emsg : msg), (char_u *)str, len);
    Python_Release_Vim();
    Py_END_ALLOW_THREADS

    Py_INCREF(Py_None);
    return Py_None;
}

    static PyObject *
OutputWritelines(PyObject *self, PyObject *args)
{
    int n;
    int i;
    PyObject *list;
    int error = ((OutputObject *)(self))->error;

    if (!PyArg_ParseTuple(args, "O", &list))
	return NULL;
    Py_INCREF(list);

    if (!PyList_Check(list)) {
	PyErr_SetString(PyExc_TypeError, _("writelines() requires list of strings"));
	Py_DECREF(list);
	return NULL;
    }

    n = PyList_Size(list);

    for (i = 0; i < n; ++i)
    {
	PyObject *line = PyList_GetItem(list, i);
	char *str;
	int len;

	if (!PyArg_Parse(line, "s#", &str, &len)) {
	    PyErr_SetString(PyExc_TypeError, _("writelines() requires list of strings"));
	    Py_DECREF(list);
	    return NULL;
	}

	Py_BEGIN_ALLOW_THREADS
	Python_Lock_Vim();
	writer((writefn)(error ? emsg : msg), (char_u *)str, len);
	Python_Release_Vim();
	Py_END_ALLOW_THREADS
    }

    Py_DECREF(list);
    Py_INCREF(Py_None);
    return Py_None;
}

/* Output buffer management
 */

static char_u *buffer = NULL;
static int buffer_len = 0;
static int buffer_size = 0;

static writefn old_fn = NULL;

    static void
buffer_ensure(int n)
{
    int new_size;
    char_u *new_buffer;

    if (n < buffer_size)
	return;

    new_size = buffer_size;
    while (new_size < n)
	new_size += 80;

    if (new_size != buffer_size)
    {
	new_buffer = alloc((unsigned)new_size);
	if (new_buffer == NULL)
	    return;

	if (buffer)
	{
	    memcpy(new_buffer, buffer, buffer_len);
	    vim_free(buffer);
	}

	buffer = new_buffer;
	buffer_size = new_size;
    }
}

    static void
PythonIO_Flush(void)
{
    if (old_fn && buffer_len)
    {
	buffer[buffer_len] = 0;
	old_fn(buffer);
    }

    buffer_len = 0;
}

    static void
writer(writefn fn, char_u *str, int n)
{
    char_u *ptr;

    if (fn != old_fn && old_fn != NULL)
	PythonIO_Flush();

    old_fn = fn;

    while (n > 0 && (ptr = memchr(str, '\n', n)) != NULL)
    {
	int len = ptr - str;

	buffer_ensure(buffer_len + len + 1);

	memcpy(buffer + buffer_len, str, len);
	buffer_len += len;
	buffer[buffer_len] = 0;
	fn(buffer);
	str = ptr + 1;
	n -= len + 1;
	buffer_len = 0;
    }

    /* Put the remaining text into the buffer for later printing */
    buffer_ensure(buffer_len + n + 1);
    memcpy(buffer + buffer_len, str, n);
    buffer_len += n;
}

/***************/

static OutputObject Output =
{
    PyObject_HEAD_INIT(&OutputType)
    0,
    0
};

static OutputObject Error =
{
    PyObject_HEAD_INIT(&OutputType)
    0,
    1
};

    static int
PythonIO_Init(void)
{
    /* Fixups... */
    OutputType.ob_type = &PyType_Type;

    PySys_SetObject("stdout", (PyObject *)(&Output));
    PySys_SetObject("stderr", (PyObject *)(&Error));

    if (PyErr_Occurred())
    {
	EMSG(_("E264: Python: Error initialising I/O objects"));
	return -1;
    }

    return 0;
}

/******************************************************
 * 3. Implementation of the Vim module for Python
 */

/* Vim module - Implementation functions
 * -------------------------------------
 */

static PyObject *VimError;

static PyObject *VimCommand(PyObject *, PyObject *);
static PyObject *VimEval(PyObject *, PyObject *);

/* Window type - Implementation functions
 * --------------------------------------
 */

typedef struct
{
    PyObject_HEAD
    win_T	*win;
}
WindowObject;

#define INVALID_WINDOW_VALUE ((win_T *)(-1))

#define WindowType_Check(obj) ((obj)->ob_type == &WindowType)

static PyObject *WindowNew(win_T *);

static void WindowDestructor(PyObject *);
static PyObject *WindowGetattr(PyObject *, char *);
static int WindowSetattr(PyObject *, char *, PyObject *);
static PyObject *WindowRepr(PyObject *);

/* Buffer type - Implementation functions
 * --------------------------------------
 */

typedef struct
{
    PyObject_HEAD
    buf_T *buf;
}
BufferObject;

#define INVALID_BUFFER_VALUE ((buf_T *)(-1))

#define BufferType_Check(obj) ((obj)->ob_type == &BufferType)

static PyObject *BufferNew (buf_T *);

static void BufferDestructor(PyObject *);
static PyObject *BufferGetattr(PyObject *, char *);
static PyObject *BufferRepr(PyObject *);

static int BufferLength(PyObject *);
static PyObject *BufferItem(PyObject *, int);
static PyObject *BufferSlice(PyObject *, int, int);
static int BufferAssItem(PyObject *, int, PyObject *);
static int BufferAssSlice(PyObject *, int, int, PyObject *);

static PyObject *BufferAppend(PyObject *, PyObject *);
static PyObject *BufferMark(PyObject *, PyObject *);
static PyObject *BufferRange(PyObject *, PyObject *);

/* Line range type - Implementation functions
 * --------------------------------------
 */

typedef struct
{
    PyObject_HEAD
    BufferObject *buf;
    int start;
    int end;
}
RangeObject;

#define RangeType_Check(obj) ((obj)->ob_type == &RangeType)

static PyObject *RangeNew(buf_T *, int, int);

static void RangeDestructor(PyObject *);
static PyObject *RangeGetattr(PyObject *, char *);
static PyObject *RangeRepr(PyObject *);

static int RangeLength(PyObject *);
static PyObject *RangeItem(PyObject *, int);
static PyObject *RangeSlice(PyObject *, int, int);
static int RangeAssItem(PyObject *, int, PyObject *);
static int RangeAssSlice(PyObject *, int, int, PyObject *);

static PyObject *RangeAppend(PyObject *, PyObject *);

/* Window list type - Implementation functions
 * -------------------------------------------
 */

static int WinListLength(PyObject *);
static PyObject *WinListItem(PyObject *, int);

/* Buffer list type - Implementation functions
 * -------------------------------------------
 */

static int BufListLength(PyObject *);
static PyObject *BufListItem(PyObject *, int);

/* Current objects type - Implementation functions
 * -----------------------------------------------
 */

static PyObject *CurrentGetattr(PyObject *, char *);
static int CurrentSetattr(PyObject *, char *, PyObject *);

/* Vim module - Definitions
 */

static struct PyMethodDef VimMethods[] = {
    /* name,	     function,		calling,    documentation */
    {"command",	     VimCommand,	1,	    "" },
    {"eval",	     VimEval,		1,	    "" },
    { NULL,	     NULL,		0,	    NULL }
};

/* Vim module - Implementation
 */
/*ARGSUSED*/
    static PyObject *
VimCommand(PyObject *self, PyObject *args)
{
    char *cmd;
    PyObject *result;

    if (!PyArg_ParseTuple(args, "s", &cmd))
	return NULL;

    PyErr_Clear();

    Py_BEGIN_ALLOW_THREADS
    Python_Lock_Vim();

    do_cmdline_cmd((char_u *)cmd);
    update_screen(VALID);

    Python_Release_Vim();
    Py_END_ALLOW_THREADS

    if (VimErrorCheck())
	result = NULL;
    else
	result = Py_None;

    Py_XINCREF(result);
    return result;
}

/*ARGSUSED*/
    static PyObject *
VimEval(PyObject *self, PyObject *args)
{
#ifdef FEAT_EVAL
    char	*expr;
    char	*str;
    PyObject	*result;

    if (!PyArg_ParseTuple(args, "s", &expr))
	return NULL;

    Py_BEGIN_ALLOW_THREADS
    Python_Lock_Vim();
    str = (char *)eval_to_string((char_u *)expr, NULL);
    Python_Release_Vim();
    Py_END_ALLOW_THREADS

    if (str == NULL)
    {
	PyErr_SetVim(_("invalid expression"));
	return NULL;
    }

    result = Py_BuildValue("s", str);

    Py_BEGIN_ALLOW_THREADS
    Python_Lock_Vim();
    vim_free(str);
    Python_Release_Vim();
    Py_END_ALLOW_THREADS

    return result;
#else
    PyErr_SetVim(_("expressions disabled at compile time"));
    return NULL;
#endif
}

/* Common routines for buffers and line ranges
 * -------------------------------------------
 */
    static int
CheckBuffer(BufferObject *this)
{
    if (this->buf == INVALID_BUFFER_VALUE)
    {
	PyErr_SetVim(_("attempt to refer to deleted buffer"));
	return -1;
    }

    return 0;
}

    static PyObject *
RBItem(BufferObject *self, int n, int start, int end)
{
    if (CheckBuffer(self))
	return NULL;

    if (n < 0 || n > end - start)
    {
	PyErr_SetString(PyExc_IndexError, _("line number out of range"));
	return NULL;
    }

    return GetBufferLine(self->buf, n+start);
}

    static PyObject *
RBSlice(BufferObject *self, int lo, int hi, int start, int end)
{
    int size;

    if (CheckBuffer(self))
	return NULL;

    size = end - start + 1;

    if (lo < 0)
	lo = 0;
    else if (lo > size)
	lo = size;
    if (hi < 0)
	hi = 0;
    if (hi < lo)
	hi = lo;
    else if (hi > size)
	hi = size;

    return GetBufferLineList(self->buf, lo+start, hi+start);
}

    static int
RBAssItem(BufferObject *self, int n, PyObject *val, int start, int end, int *new_end)
{
    int len_change;

    if (CheckBuffer(self))
	return -1;

    if (n < 0 || n > end - start)
    {
	PyErr_SetString(PyExc_IndexError, _("line number out of range"));
	return -1;
    }

    if (SetBufferLine(self->buf, n+start, val, &len_change) == FAIL)
	return -1;

    if (new_end)
	*new_end = end + len_change;

    return 0;
}

    static int
RBAssSlice(BufferObject *self, int lo, int hi, PyObject *val, int start, int end, int *new_end)
{
    int size;
    int len_change;

    /* Self must be a valid buffer */
    if (CheckBuffer(self))
	return -1;

    /* Sort out the slice range */
    size = end - start + 1;

    if (lo < 0)
	lo = 0;
    else if (lo > size)
	lo = size;
    if (hi < 0)
	hi = 0;
    if (hi < lo)
	hi = lo;
    else if (hi > size)
	hi = size;

    if (SetBufferLineList(self->buf, lo+start, hi+start, val, &len_change) == FAIL)
	return -1;

    if (new_end)
	*new_end = end + len_change;

    return 0;
}

    static PyObject *
RBAppend(BufferObject *self, PyObject *args, int start, int end, int *new_end)
{
    PyObject *lines;
    int len_change;
    int max;
    int n;

    if (CheckBuffer(self))
	return NULL;

    max = n = end - start + 1;

    if (!PyArg_ParseTuple(args, "O|i", &lines, &n))
	return NULL;

    if (n < 0 || n > max)
    {
	PyErr_SetString(PyExc_ValueError, _("line number out of range"));
	return NULL;
    }

    if (InsertBufferLines(self->buf, n + start - 1, lines, &len_change) == FAIL)
	return NULL;

    if (new_end)
	*new_end = end + len_change;

    Py_INCREF(Py_None);
    return Py_None;
}


/* Buffer object - Definitions
 */

static struct PyMethodDef BufferMethods[] = {
    /* name,	    function,		calling,    documentation */
    {"append",	    BufferAppend,	1,	    "" },
    {"mark",	    BufferMark,		1,	    "" },
    {"range",	    BufferRange,	1,	    "" },
    { NULL,	    NULL,		0,	    NULL }
};

static PySequenceMethods BufferAsSeq = {
    (inquiry)		BufferLength,	    /* sq_length,    len(x)   */
    (binaryfunc)	0, /* BufferConcat, */	     /* sq_concat,    x+y      */
    (intargfunc)	0, /* BufferRepeat, */	     /* sq_repeat,    x*n      */
    (intargfunc)	BufferItem,	    /* sq_item,      x[i]     */
    (intintargfunc)	BufferSlice,	    /* sq_slice,     x[i:j]   */
    (intobjargproc)	BufferAssItem,	    /* sq_ass_item,  x[i]=v   */
    (intintobjargproc)	BufferAssSlice,     /* sq_ass_slice, x[i:j]=v */
};

static PyTypeObject BufferType = {
    PyObject_HEAD_INIT(0)
    0,
    "buffer",
    sizeof(BufferObject),
    0,

    (destructor)    BufferDestructor,	/* tp_dealloc,	refcount==0  */
    (printfunc)     0,			/* tp_print,	print x      */
    (getattrfunc)   BufferGetattr,	/* tp_getattr,	x.attr	     */
    (setattrfunc)   0,			/* tp_setattr,	x.attr=v     */
    (cmpfunc)	    0,			/* tp_compare,	x>y	     */
    (reprfunc)	    BufferRepr,		/* tp_repr,	`x`, print x */

    0,		    /* as number */
    &BufferAsSeq,   /* as sequence */
    0,		    /* as mapping */

    (hashfunc) 0,			/* tp_hash, dict(x) */
    (ternaryfunc) 0,			/* tp_call, x()     */
    (reprfunc) 0,			/* tp_str,  str(x)  */
};

/* Buffer object - Implementation
 */

    static PyObject *
BufferNew(buf_T *buf)
{
    /* We need to handle deletion of buffers underneath us.
     * If we add a "python_ref" field to the buf_T structure,
     * then we can get at it in buf_freeall() in vim. We then
     * need to create only ONE Python object per buffer - if
     * we try to create a second, just INCREF the existing one
     * and return it. The (single) Python object referring to
     * the buffer is stored in "python_ref".
     * Question: what to do on a buf_freeall(). We'll probably
     * have to either delete the Python object (DECREF it to
     * zero - a bad idea, as it leaves dangling refs!) or
     * set the buf_T * value to an invalid value (-1?), which
     * means we need checks in all access functions... Bah.
     */

    BufferObject *self;

    if (buf->python_ref)
    {
	self = buf->python_ref;
	Py_INCREF(self);
    }
    else
    {
	self = PyObject_NEW(BufferObject, &BufferType);
	if (self == NULL)
	    return NULL;
	self->buf = buf;
	buf->python_ref = self;
    }

    return (PyObject *)(self);
}

    static void
BufferDestructor(PyObject *self)
{
    BufferObject *this = (BufferObject *)(self);

    if (this->buf && this->buf != INVALID_BUFFER_VALUE)
	this->buf->python_ref = NULL;

    PyMem_DEL(self);
}

    static PyObject *
BufferGetattr(PyObject *self, char *name)
{
    BufferObject *this = (BufferObject *)(self);

    if (CheckBuffer(this))
	return NULL;

    if (strcmp(name, "name") == 0)
	return Py_BuildValue("s",this->buf->b_ffname);
    else if (strcmp(name, "number") == 0)
	return Py_BuildValue("i",this->buf->b_fnum);
    else if (strcmp(name,"__members__") == 0)
	return Py_BuildValue("[ss]", "name", "number");
    else
	return Py_FindMethod(BufferMethods, self, name);
}

    static PyObject *
BufferRepr(PyObject *self)
{
    static char repr[50];
    BufferObject *this = (BufferObject *)(self);

    if (this->buf == INVALID_BUFFER_VALUE)
    {
	sprintf(repr, _("<buffer object (deleted) at %8lX>"), (long)(self));
	return PyString_FromString(repr);
    }
    else
    {
	char *name = (char *)this->buf->b_fname;
	int len;

	if (name == NULL)
	    name = "";
	len = strlen(name);

	if (len > 35)
	    name = name + (35 - len);

	sprintf(repr, "<buffer %s%s>", len > 35 ? "..." : "", name);

	return PyString_FromString(repr);
    }
}

/******************/

    static int
BufferLength(PyObject *self)
{
    /* HOW DO WE SIGNAL AN ERROR FROM THIS FUNCTION? */
    if (CheckBuffer((BufferObject *)(self)))
	return -1; /* ??? */

    return (((BufferObject *)(self))->buf->b_ml.ml_line_count);
}

    static PyObject *
BufferItem(PyObject *self, int n)
{
    return RBItem((BufferObject *)(self), n, 1,
		  (int)((BufferObject *)(self))->buf->b_ml.ml_line_count);
}

    static PyObject *
BufferSlice(PyObject *self, int lo, int hi)
{
    return RBSlice((BufferObject *)(self), lo, hi, 1,
		   (int)((BufferObject *)(self))->buf->b_ml.ml_line_count);
}

    static int
BufferAssItem(PyObject *self, int n, PyObject *val)
{
    return RBAssItem((BufferObject *)(self), n, val, 1,
		     (int)((BufferObject *)(self))->buf->b_ml.ml_line_count,
		     NULL);
}

    static int
BufferAssSlice(PyObject *self, int lo, int hi, PyObject *val)
{
    return RBAssSlice((BufferObject *)(self), lo, hi, val, 1,
		      (int)((BufferObject *)(self))->buf->b_ml.ml_line_count,
		      NULL);
}

    static PyObject *
BufferAppend(PyObject *self, PyObject *args)
{
    return RBAppend((BufferObject *)(self), args, 1,
		    (int)((BufferObject *)(self))->buf->b_ml.ml_line_count,
		    NULL);
}

    static PyObject *
BufferMark(PyObject *self, PyObject *args)
{
    pos_T	*posp;
    char	mark;
    buf_T	*curbuf_save;

    if (CheckBuffer((BufferObject *)(self)))
	return NULL;

    if (!PyArg_ParseTuple(args, "c", &mark))
	return NULL;

    curbuf_save = curbuf;
    curbuf = ((BufferObject *)(self))->buf;
    posp = getmark(mark, FALSE);
    curbuf = curbuf_save;

    if (posp == NULL)
    {
	PyErr_SetVim(_("invalid mark name"));
	return NULL;
    }

    /* Ckeck for keyboard interrupt */
    if (VimErrorCheck())
	return NULL;

    if (posp->lnum <= 0)
    {
	/* Or raise an error? */
	Py_INCREF(Py_None);
	return Py_None;
    }

    return Py_BuildValue("(ll)", (long)(posp->lnum), (long)(posp->col));
}

    static PyObject *
BufferRange(PyObject *self, PyObject *args)
{
    int start;
    int end;

    if (CheckBuffer((BufferObject *)(self)))
	return NULL;

    if (!PyArg_ParseTuple(args, "ii", &start, &end))
	return NULL;

    return RangeNew(((BufferObject *)(self))->buf, start, end);
}

/* Line range object - Definitions
 */

static struct PyMethodDef RangeMethods[] = {
    /* name,	    function,		calling,    documentation */
    {"append",	    RangeAppend,	1,	    "" },
    { NULL,	    NULL,		0,	    NULL }
};

static PySequenceMethods RangeAsSeq = {
    (inquiry)		RangeLength,	    /* sq_length,    len(x)   */
    (binaryfunc)	0, /* RangeConcat, */	     /* sq_concat,    x+y      */
    (intargfunc)	0, /* RangeRepeat, */	     /* sq_repeat,    x*n      */
    (intargfunc)	RangeItem,	    /* sq_item,      x[i]     */
    (intintargfunc)	RangeSlice,	    /* sq_slice,     x[i:j]   */
    (intobjargproc)	RangeAssItem,	    /* sq_ass_item,  x[i]=v   */
    (intintobjargproc)	RangeAssSlice,	    /* sq_ass_slice, x[i:j]=v */
};

static PyTypeObject RangeType = {
    PyObject_HEAD_INIT(0)
    0,
    "range",
    sizeof(RangeObject),
    0,

    (destructor)    RangeDestructor,	/* tp_dealloc,	refcount==0  */
    (printfunc)     0,			/* tp_print,	print x      */
    (getattrfunc)   RangeGetattr,	/* tp_getattr,	x.attr	     */
    (setattrfunc)   0,			/* tp_setattr,	x.attr=v     */
    (cmpfunc)	    0,			/* tp_compare,	x>y	     */
    (reprfunc)	    RangeRepr,		/* tp_repr,	`x`, print x */

    0,		    /* as number */
    &RangeAsSeq,    /* as sequence */
    0,		    /* as mapping */

    (hashfunc) 0,			/* tp_hash, dict(x) */
    (ternaryfunc) 0,			/* tp_call, x()     */
    (reprfunc) 0,			/* tp_str,  str(x)  */
};

/* Line range object - Implementation
 */

    static PyObject *
RangeNew(buf_T *buf, int start, int end)
{
    BufferObject *bufr;
    RangeObject *self;
    self = PyObject_NEW(RangeObject, &RangeType);
    if (self == NULL)
	return NULL;

    bufr = (BufferObject *)BufferNew(buf);
    if (bufr == NULL)
    {
	PyMem_DEL(self);
	return NULL;
    }
    Py_INCREF(bufr);

    self->buf = bufr;
    self->start = start;
    self->end = end;

    return (PyObject *)(self);
}

    static void
RangeDestructor(PyObject *self)
{
    Py_DECREF(((RangeObject *)(self))->buf);
    PyMem_DEL(self);
}

    static PyObject *
RangeGetattr(PyObject *self, char *name)
{
    if (strcmp(name, "start") == 0)
	return Py_BuildValue("i",((RangeObject *)(self))->start - 1);
    else if (strcmp(name, "end") == 0)
	return Py_BuildValue("i",((RangeObject *)(self))->end - 1);
    else
	return Py_FindMethod(RangeMethods, self, name);
}

    static PyObject *
RangeRepr(PyObject *self)
{
    static char repr[75];
    RangeObject *this = (RangeObject *)(self);

    if (this->buf->buf == INVALID_BUFFER_VALUE)
    {
	sprintf(repr, "<range object (for deleted buffer) at %8lX>",
								(long)(self));
	return PyString_FromString(repr);
    }
    else
    {
	char *name = (char *)this->buf->buf->b_fname;
	int len;

	if (name == NULL)
	    name = "";
	len = strlen(name);

	if (len > 45)
	    name = name + (45 - len);

	sprintf(repr, "<range %s%s (%d:%d)>",
		len > 45 ? "..." : "", name,
		this->start, this->end);

	return PyString_FromString(repr);
    }
}

/****************/

    static int
RangeLength(PyObject *self)
{
    /* HOW DO WE SIGNAL AN ERROR FROM THIS FUNCTION? */
    if (CheckBuffer(((RangeObject *)(self))->buf))
	return -1; /* ??? */

    return (((RangeObject *)(self))->end - ((RangeObject *)(self))->start + 1);
}

    static PyObject *
RangeItem(PyObject *self, int n)
{
    return RBItem(((RangeObject *)(self))->buf, n,
		  ((RangeObject *)(self))->start,
		  ((RangeObject *)(self))->end);
}

    static PyObject *
RangeSlice(PyObject *self, int lo, int hi)
{
    return RBSlice(((RangeObject *)(self))->buf, lo, hi,
		   ((RangeObject *)(self))->start,
		   ((RangeObject *)(self))->end);
}

    static int
RangeAssItem(PyObject *self, int n, PyObject *val)
{
    return RBAssItem(((RangeObject *)(self))->buf, n, val,
		     ((RangeObject *)(self))->start,
		     ((RangeObject *)(self))->end,
		     &((RangeObject *)(self))->end);
}

    static int
RangeAssSlice(PyObject *self, int lo, int hi, PyObject *val)
{
    return RBAssSlice(((RangeObject *)(self))->buf, lo, hi, val,
		      ((RangeObject *)(self))->start,
		      ((RangeObject *)(self))->end,
		      &((RangeObject *)(self))->end);
}

    static PyObject *
RangeAppend(PyObject *self, PyObject *args)
{
    return RBAppend(((RangeObject *)(self))->buf, args,
		    ((RangeObject *)(self))->start,
		    ((RangeObject *)(self))->end,
		    &((RangeObject *)(self))->end);
}

/* Buffer list object - Definitions
 */

typedef struct
{
    PyObject_HEAD
}
BufListObject;

static PySequenceMethods BufListAsSeq = {
    (inquiry)		BufListLength,	    /* sq_length,    len(x)   */
    (binaryfunc)	0,		    /* sq_concat,    x+y      */
    (intargfunc)	0,		    /* sq_repeat,    x*n      */
    (intargfunc)	BufListItem,	    /* sq_item,      x[i]     */
    (intintargfunc)	0,		    /* sq_slice,     x[i:j]   */
    (intobjargproc)	0,		    /* sq_ass_item,  x[i]=v   */
    (intintobjargproc)	0,		    /* sq_ass_slice, x[i:j]=v */
};

static PyTypeObject BufListType = {
    PyObject_HEAD_INIT(0)
    0,
    "buffer list",
    sizeof(BufListObject),
    0,

    (destructor)    0,			/* tp_dealloc,	refcount==0  */
    (printfunc)     0,			/* tp_print,	print x      */
    (getattrfunc)   0,			/* tp_getattr,	x.attr	     */
    (setattrfunc)   0,			/* tp_setattr,	x.attr=v     */
    (cmpfunc)	    0,			/* tp_compare,	x>y	     */
    (reprfunc)	    0,			/* tp_repr,	`x`, print x */

    0,		    /* as number */
    &BufListAsSeq,  /* as sequence */
    0,		    /* as mapping */

    (hashfunc) 0,			/* tp_hash, dict(x) */
    (ternaryfunc) 0,			/* tp_call, x()     */
    (reprfunc) 0,			/* tp_str,  str(x)  */
};

/* Buffer list object - Implementation
 */

/*ARGSUSED*/
    static int
BufListLength(PyObject *self)
{
    buf_T	*b = firstbuf;
    int		n = 0;

    while (b)
    {
	++n;
	b = b->b_next;
    }

    return n;
}

/*ARGSUSED*/
    static PyObject *
BufListItem(PyObject *self, int n)
{
    buf_T *b;

    for (b = firstbuf; b; b = b->b_next, --n)
    {
	if (n == 0)
	    return BufferNew(b);
    }

    PyErr_SetString(PyExc_IndexError, _("no such buffer"));
    return NULL;
}

/* Window object - Definitions
 */

static struct PyMethodDef WindowMethods[] = {
    /* name,	    function,		calling,    documentation */
    { NULL,	    NULL,		0,	    NULL }
};

static PyTypeObject WindowType = {
    PyObject_HEAD_INIT(0)
    0,
    "window",
    sizeof(WindowObject),
    0,

    (destructor)    WindowDestructor,	/* tp_dealloc,	refcount==0  */
    (printfunc)     0,			/* tp_print,	print x      */
    (getattrfunc)   WindowGetattr,	/* tp_getattr,	x.attr	     */
    (setattrfunc)   WindowSetattr,	/* tp_setattr,	x.attr=v     */
    (cmpfunc)	    0,			/* tp_compare,	x>y	     */
    (reprfunc)	    WindowRepr,		/* tp_repr,	`x`, print x */

    0,		    /* as number */
    0,		    /* as sequence */
    0,		    /* as mapping */

    (hashfunc) 0,			/* tp_hash, dict(x) */
    (ternaryfunc) 0,			/* tp_call, x()     */
    (reprfunc) 0,			/* tp_str,  str(x)  */
};

/* Window object - Implementation
 */

    static PyObject *
WindowNew(win_T *win)
{
    /* We need to handle deletion of windows underneath us.
     * If we add a "python_ref" field to the win_T structure,
     * then we can get at it in win_free() in vim. We then
     * need to create only ONE Python object per window - if
     * we try to create a second, just INCREF the existing one
     * and return it. The (single) Python object referring to
     * the window is stored in "python_ref".
     * On a win_free() we set the Python object's win_T* field
     * to an invalid value. We trap all uses of a window
     * object, and reject them if the win_T* field is invalid.
     */

    WindowObject *self;

    if (win->python_ref)
    {
	self = win->python_ref;
	Py_INCREF(self);
    }
    else
    {
	self = PyObject_NEW(WindowObject, &WindowType);
	if (self == NULL)
	    return NULL;
	self->win = win;
	win->python_ref = self;
    }

    return (PyObject *)(self);
}

    static void
WindowDestructor(PyObject *self)
{
    WindowObject *this = (WindowObject *)(self);

    if (this->win && this->win != INVALID_WINDOW_VALUE)
	this->win->python_ref = NULL;

    PyMem_DEL(self);
}

    static int
CheckWindow(WindowObject *this)
{
    if (this->win == INVALID_WINDOW_VALUE)
    {
	PyErr_SetVim(_("attempt to refer to deleted window"));
	return -1;
    }

    return 0;
}

    static PyObject *
WindowGetattr(PyObject *self, char *name)
{
    WindowObject *this = (WindowObject *)(self);

    if (CheckWindow(this))
	return NULL;

    if (strcmp(name, "buffer") == 0)
	return (PyObject *)BufferNew(this->win->w_buffer);
    else if (strcmp(name, "cursor") == 0)
    {
	pos_T *pos = &this->win->w_cursor;

	return Py_BuildValue("(ll)", (long)(pos->lnum), (long)(pos->col));
    }
    else if (strcmp(name, "height") == 0)
	return Py_BuildValue("l", (long)(this->win->w_height));
#ifdef FEAT_VERTSPLIT
    else if (strcmp(name, "width") == 0)
	return Py_BuildValue("l", (long)(W_WIDTH(this->win)));
#endif
    else if (strcmp(name,"__members__") == 0)
	return Py_BuildValue("[sss]", "buffer", "cursor", "height");
    else
	return Py_FindMethod(WindowMethods, self, name);
}

    static int
WindowSetattr(PyObject *self, char *name, PyObject *val)
{
    WindowObject *this = (WindowObject *)(self);

    if (CheckWindow(this))
	return -1;

    if (strcmp(name, "buffer") == 0)
    {
	PyErr_SetString(PyExc_TypeError, _("readonly attribute"));
	return -1;
    }
    else if (strcmp(name, "cursor") == 0)
    {
	long lnum;
	long col;

	if (!PyArg_Parse(val, "(ll)", &lnum, &col))
	    return -1;

	if (lnum <= 0 || lnum > this->win->w_buffer->b_ml.ml_line_count)
	{
	    PyErr_SetVim(_("cursor position outside buffer"));
	    return -1;
	}

	/* Check for keyboard interrupts */
	if (VimErrorCheck())
	    return -1;

	/* NO CHECK ON COLUMN - SEEMS NOT TO MATTER */

	this->win->w_cursor.lnum = lnum;
	this->win->w_cursor.col = col;
	update_screen(VALID);

	return 0;
    }
    else if (strcmp(name, "height") == 0)
    {
	int	height;
	win_T	*savewin;

	if (!PyArg_Parse(val, "i", &height))
	    return -1;

#ifdef FEAT_GUI
	need_mouse_correct = TRUE;
#endif
	savewin = curwin;
	curwin = this->win;
	win_setheight(height);
	curwin = savewin;

	/* Check for keyboard interrupts */
	if (VimErrorCheck())
	    return -1;

	return 0;
    }
#ifdef FEAT_VERTSPLIT
    else if (strcmp(name, "width") == 0)
    {
	int	width;
	win_T	*savewin;

	if (!PyArg_Parse(val, "i", &width))
	    return -1;

#ifdef FEAT_GUI
	need_mouse_correct = TRUE;
#endif
	savewin = curwin;
	curwin = this->win;
	win_setwidth(width);
	curwin = savewin;

	/* Check for keyboard interrupts */
	if (VimErrorCheck())
	    return -1;

	return 0;
    }
#endif
    else
    {
	PyErr_SetString(PyExc_AttributeError, name);
	return -1;
    }
}

    static PyObject *
WindowRepr(PyObject *self)
{
    static char repr[50];
    WindowObject *this = (WindowObject *)(self);

    if (this->win == INVALID_WINDOW_VALUE)
    {
	sprintf(repr, _("<window object (deleted) at %.8lX>"), (long)(self));
	return PyString_FromString(repr);
    }
    else
    {
	int	i = 0;
	win_T	*w;

	for (w = firstwin; w != NULL && w != this->win; w = W_NEXT(w))
	    ++i;

	if (w == NULL)
	    sprintf(repr, _("<window object (unknown) at %.8lX>"), (long)(self));
	else
	    sprintf(repr, _("<window %d>"), i);

	return PyString_FromString(repr);
    }
}

/* Window list object - Definitions
 */

typedef struct
{
    PyObject_HEAD
}
WinListObject;

static PySequenceMethods WinListAsSeq = {
    (inquiry)		WinListLength,	    /* sq_length,    len(x)   */
    (binaryfunc)	0,		    /* sq_concat,    x+y      */
    (intargfunc)	0,		    /* sq_repeat,    x*n      */
    (intargfunc)	WinListItem,	    /* sq_item,      x[i]     */
    (intintargfunc)	0,		    /* sq_slice,     x[i:j]   */
    (intobjargproc)	0,		    /* sq_ass_item,  x[i]=v   */
    (intintobjargproc)	0,		    /* sq_ass_slice, x[i:j]=v */
};

static PyTypeObject WinListType = {
    PyObject_HEAD_INIT(0)
    0,
    "window list",
    sizeof(WinListObject),
    0,

    (destructor)    0,			/* tp_dealloc,	refcount==0  */
    (printfunc)     0,			/* tp_print,	print x      */
    (getattrfunc)   0,			/* tp_getattr,	x.attr	     */
    (setattrfunc)   0,			/* tp_setattr,	x.attr=v     */
    (cmpfunc)	    0,			/* tp_compare,	x>y	     */
    (reprfunc)	    0,			/* tp_repr,	`x`, print x */

    0,		    /* as number */
    &WinListAsSeq,  /* as sequence */
    0,		    /* as mapping */

    (hashfunc) 0,			/* tp_hash, dict(x) */
    (ternaryfunc) 0,			/* tp_call, x()     */
    (reprfunc) 0,			/* tp_str,  str(x)  */
};

/* Window list object - Implementation
 */
/*ARGSUSED*/
    static int
WinListLength(PyObject *self)
{
    win_T	*w = firstwin;
    int		n = 0;

    while (w)
    {
	++n;
	w = W_NEXT(w);
    }

    return n;
}

/*ARGSUSED*/
    static PyObject *
WinListItem(PyObject *self, int n)
{
    win_T *w;

    for (w = firstwin; w; w = W_NEXT(w), --n)
	if (n == 0)
	    return WindowNew(w);

    PyErr_SetString(PyExc_IndexError, _("no such window"));
    return NULL;
}

/* Current items object - Definitions
 */

typedef struct
{
    PyObject_HEAD
}
CurrentObject;

static PyTypeObject CurrentType = {
    PyObject_HEAD_INIT(0)
    0,
    "current data",
    sizeof(CurrentObject),
    0,

    (destructor)    0,			/* tp_dealloc,	refcount==0  */
    (printfunc)     0,			/* tp_print,	print x      */
    (getattrfunc)   CurrentGetattr,	/* tp_getattr,	x.attr	     */
    (setattrfunc)   CurrentSetattr,	/* tp_setattr,	x.attr=v     */
    (cmpfunc)	    0,			/* tp_compare,	x>y	     */
    (reprfunc)	    0,			/* tp_repr,	`x`, print x */

    0,		    /* as number */
    0,		    /* as sequence */
    0,		    /* as mapping */

    (hashfunc) 0,			/* tp_hash, dict(x) */
    (ternaryfunc) 0,			/* tp_call, x()     */
    (reprfunc) 0,			/* tp_str,  str(x)  */
};

/* Current items object - Implementation
 */
/*ARGSUSED*/
    static PyObject *
CurrentGetattr(PyObject *self, char *name)
{
    if (strcmp(name, "buffer") == 0)
	return (PyObject *)BufferNew(curbuf);
    else if (strcmp(name, "window") == 0)
	return (PyObject *)WindowNew(curwin);
    else if (strcmp(name, "line") == 0)
	return GetBufferLine(curbuf, (int)curwin->w_cursor.lnum);
    else if (strcmp(name, "range") == 0)
	return RangeNew(curbuf, RangeStart, RangeEnd);
    else if (strcmp(name,"__members__") == 0)
	return Py_BuildValue("[ssss]", "buffer", "window", "line", "range");
    else
    {
	PyErr_SetString(PyExc_AttributeError, name);
	return NULL;
    }
}

/*ARGSUSED*/
    static int
CurrentSetattr(PyObject *self, char *name, PyObject *value)
{
    if (strcmp(name, "line") == 0)
    {
	if (SetBufferLine(curbuf, (int)curwin->w_cursor.lnum, value, NULL) == FAIL)
	    return -1;

	return 0;
    }
    else
    {
	PyErr_SetString(PyExc_AttributeError, name);
	return -1;
    }
}

/* External interface
 */

    void
python_buffer_free(buf_T *buf)
{
    if (buf->python_ref)
    {
	BufferObject *bp = buf->python_ref;
	bp->buf = INVALID_BUFFER_VALUE;
	buf->python_ref = NULL;
    }
}

#if defined(FEAT_WINDOWS) || defined(PROTO)
    void
python_window_free(win_T *win)
{
    if (win->python_ref)
    {
	WindowObject *wp = win->python_ref;
	wp->win = INVALID_WINDOW_VALUE;
	win->python_ref = NULL;
    }
}
#endif

static BufListObject TheBufferList =
{
    PyObject_HEAD_INIT(&BufListType)
};

static WinListObject TheWindowList =
{
    PyObject_HEAD_INIT(&WinListType)
};

static CurrentObject TheCurrent =
{
    PyObject_HEAD_INIT(&CurrentType)
};

    static int
PythonMod_Init(void)
{
    PyObject *mod;
    PyObject *dict;
    static char *(argv[2]) = {"", NULL};

    /* Fixups... */
    BufferType.ob_type = &PyType_Type;
    RangeType.ob_type = &PyType_Type;
    WindowType.ob_type = &PyType_Type;
    BufListType.ob_type = &PyType_Type;
    WinListType.ob_type = &PyType_Type;
    CurrentType.ob_type = &PyType_Type;

    /* Set sys.argv[] to avoid a crash in warn(). */
    PySys_SetArgv(1, argv);

    mod = Py_InitModule("vim", VimMethods);
    dict = PyModule_GetDict(mod);

    VimError = Py_BuildValue("s", "vim.error");

    PyDict_SetItemString(dict, "error", VimError);
    PyDict_SetItemString(dict, "buffers", (PyObject *)(&TheBufferList));
    PyDict_SetItemString(dict, "current", (PyObject *)(&TheCurrent));
    PyDict_SetItemString(dict, "windows", (PyObject *)(&TheWindowList));

    if (PyErr_Occurred())
	return -1;

    return 0;
}

/*************************************************************************
 * 4. Utility functions for handling the interface between Vim and Python.
 */

/* Get a line from the specified buffer. The line number is
 * in Vim format (1-based). The line is returned as a Python
 * string object.
 */
    static PyObject *
GetBufferLine(buf_T *buf, int n)
{
    return LineToString((char *)ml_get_buf(buf, (linenr_T)n, FALSE));
}

/* Get a list of lines from the specified buffer. The line numbers
 * are in Vim format (1-based). The range is from lo up to, but not
 * including, hi. The list is returned as a Python list of string objects.
 */
    static PyObject *
GetBufferLineList(buf_T *buf, int lo, int hi)
{
    int i;
    int n = hi - lo;
    PyObject *list = PyList_New(n);

    if (list == NULL)
	return NULL;

    for (i = 0; i < n; ++i)
    {
	PyObject *str = LineToString((char *)ml_get_buf(buf, (linenr_T)(lo+i), FALSE));

	/* Error check - was the Python string creation OK? */
	if (str == NULL)
	{
	    Py_DECREF(list);
	    return NULL;
	}

	/* Set the list item */
	if (PyList_SetItem(list, i, str))
	{
	    Py_DECREF(str);
	    Py_DECREF(list);
	    return NULL;
	}
    }

    /* The ownership of the Python list is passed to the caller (ie,
     * the caller should Py_DECREF() the object when it is finished
     * with it).
     */

    return list;
}

/*
 * Check if deleting lines made the cursor position invalid.
 * Changed the lines from "lo" to "hi" and added "extra" lines (negative if
 * deleted).
 */
    static void
py_fix_cursor(int lo, int hi, int extra)
{
    if (curwin->w_cursor.lnum >= lo)
    {
	/* Adjust the cursor position if it's in/after the changed
	 * lines. */
	if (curwin->w_cursor.lnum >= hi)
	{
	    curwin->w_cursor.lnum += extra;
	    check_cursor_col();
	}
	else if (extra < 0)
	{
	    curwin->w_cursor.lnum = lo;
	    check_cursor();
	}
	changed_cline_bef_curs();
    }
    invalidate_botline();
}

/* Replace a line in the specified buffer. The line number is
 * in Vim format (1-based). The replacement line is given as
 * a Python string object. The object is checked for validity
 * and correct format. Errors are returned as a value of FAIL.
 * The return value is OK on success.
 * If OK is returned and len_change is not NULL, *len_change
 * is set to the change in the buffer length.
 */
    static int
SetBufferLine(buf_T *buf, int n, PyObject *line, int *len_change)
{
    /* First of all, we check the thpe of the supplied Python object.
     * There are three cases:
     *	  1. NULL, or None - this is a deletion.
     *	  2. A string	   - this is a replacement.
     *	  3. Anything else - this is an error.
     */
    if (line == Py_None || line == NULL)
    {
	buf_T *savebuf = curbuf;

	PyErr_Clear();
	curbuf = buf;

	if (u_savedel((linenr_T)n, 1L) == FAIL)
	    PyErr_SetVim(_("cannot save undo information"));
	else if (ml_delete((linenr_T)n, FALSE) == FAIL)
	    PyErr_SetVim(_("cannot delete line"));
	else
	{
	    deleted_lines_mark((linenr_T)n, 1L);
	    if (buf == curwin->w_buffer)
		py_fix_cursor(n, n + 1, -1);
	}

	curbuf = savebuf;

	if (PyErr_Occurred() || VimErrorCheck())
	    return FAIL;

	if (len_change)
	    *len_change = -1;

	return OK;
    }
    else if (PyString_Check(line))
    {
	char *save = StringToLine(line);
	buf_T *savebuf = curbuf;

	if (save == NULL)
	    return FAIL;

	/* We do not need to free "save" if ml_replace() consumes it. */
	PyErr_Clear();
	curbuf = buf;

	if (u_savesub((linenr_T)n) == FAIL)
	{
	    PyErr_SetVim(_("cannot save undo information"));
	    vim_free(save);
	}
	else if (ml_replace((linenr_T)n, (char_u *)save, FALSE) == FAIL)
	{
	    PyErr_SetVim(_("cannot replace line"));
	    vim_free(save);
	}
	else
	    changed_bytes((linenr_T)n, 0);

	curbuf = savebuf;

	if (PyErr_Occurred() || VimErrorCheck())
	    return FAIL;

	if (len_change)
	    *len_change = 0;

	return OK;
    }
    else
    {
	PyErr_BadArgument();
	return FAIL;
    }
}

/* Replace a range of lines in the specified buffer. The line numbers are in
 * Vim format (1-based). The range is from lo up to, but not including, hi.
 * The replacement lines are given as a Python list of string objects. The
 * list is checked for validity and correct format. Errors are returned as a
 * value of FAIL.  The return value is OK on success.
 * If OK is returned and len_change is not NULL, *len_change
 * is set to the change in the buffer length.
 */
    static int
SetBufferLineList(buf_T *buf, int lo, int hi, PyObject *list, int *len_change)
{
    /* First of all, we check the thpe of the supplied Python object.
     * There are three cases:
     *	  1. NULL, or None - this is a deletion.
     *	  2. A list	   - this is a replacement.
     *	  3. Anything else - this is an error.
     */
    if (list == Py_None || list == NULL)
    {
	int	i;
	int	n = hi - lo;
	buf_T	*savebuf = curbuf;

	PyErr_Clear();
	curbuf = buf;

	if (u_savedel((linenr_T)lo, (long)n) == FAIL)
	    PyErr_SetVim(_("cannot save undo information"));
	else
	{
	    for (i = 0; i < n; ++i)
	    {
		if (ml_delete((linenr_T)lo, FALSE) == FAIL)
		{
		    PyErr_SetVim(_("cannot delete line"));
		    break;
		}
	    }
	    deleted_lines_mark((linenr_T)lo, (long)i);

	    if (buf == curwin->w_buffer)
		py_fix_cursor(lo, hi, -n);
	}

	curbuf = savebuf;

	if (PyErr_Occurred() || VimErrorCheck())
	    return FAIL;

	if (len_change)
	    *len_change = -n;

	return OK;
    }
    else if (PyList_Check(list))
    {
	int	i;
	int	new_len = PyList_Size(list);
	int	old_len = hi - lo;
	int	extra = 0;	/* lines added to text, can be negative */
	char	**array;
	buf_T	*savebuf;

	if (new_len == 0)	/* avoid allocating zero bytes */
	    array = NULL;
	else
	{
	    array = (char **)alloc((unsigned)(new_len * sizeof(char *)));
	    if (array == NULL)
	    {
		PyErr_NoMemory();
		return FAIL;
	    }
	}

	for (i = 0; i < new_len; ++i)
	{
	    PyObject *line = PyList_GetItem(list, i);

	    array[i] = StringToLine(line);
	    if (array[i] == NULL)
	    {
		while (i)
		    vim_free(array[--i]);
		vim_free(array);
		return FAIL;
	    }
	}

	savebuf = curbuf;

	PyErr_Clear();
	curbuf = buf;

	if (u_save((linenr_T)(lo-1), (linenr_T)hi) == FAIL)
	    PyErr_SetVim(_("cannot save undo information"));

	/* If the size of the range is reducing (ie, new_len < old_len) we
	 * need to delete some old_len. We do this at the start, by
	 * repeatedly deleting line "lo".
	 */
	if (!PyErr_Occurred())
	{
	    for (i = 0; i < old_len - new_len; ++i)
		if (ml_delete((linenr_T)lo, FALSE) == FAIL)
		{
		    PyErr_SetVim(_("cannot delete line"));
		    break;
		}
	    extra -= i;
	}

	/* For as long as possible, replace the existing old_len with the
	 * new old_len. This is a more efficient operation, as it requires
	 * less memory allocation and freeing.
	 */
	if (!PyErr_Occurred())
	{
	    for (i = 0; i < old_len && i < new_len; ++i)
		if (ml_replace((linenr_T)(lo+i), (char_u *)array[i], FALSE)
								      == FAIL)
		{
		    PyErr_SetVim(_("cannot replace line"));
		    break;
		}
	}
	else
	    i = 0;

	/* Now we may need to insert the remaining new old_len. If we do, we
	 * must free the strings as we finish with them (we can't pass the
	 * responsibility to vim in this case).
	 */
	if (!PyErr_Occurred())
	{
	    while (i < new_len)
	    {
		if (ml_append((linenr_T)(lo + i - 1),
					(char_u *)array[i], 0, FALSE) == FAIL)
		{
		    PyErr_SetVim(_("cannot insert line"));
		    break;
		}
		vim_free(array[i]);
		++i;
		++extra;
	    }
	}

	/* Free any left-over old_len, as a result of an error */
	while (i < new_len)
	{
	    vim_free(array[i]);
	    ++i;
	}

	/* Free the array of old_len. All of its contents have now
	 * been dealt with (either freed, or the responsibility passed
	 * to vim.
	 */
	vim_free(array);

	/* Adjust marks. Invalidate any which lie in the
	 * changed range, and move any in the remainder of the buffer.
	 */
	mark_adjust((linenr_T)lo, (linenr_T)(hi - 1),
						  (long)MAXLNUM, (long)extra);
	changed_lines((linenr_T)lo, 0, (linenr_T)hi, (long)extra);

	if (buf == curwin->w_buffer)
	    py_fix_cursor(lo, hi, extra);

	curbuf = savebuf;

	if (PyErr_Occurred() || VimErrorCheck())
	    return FAIL;

	if (len_change)
	    *len_change = new_len - old_len;

	return OK;
    }
    else
    {
	PyErr_BadArgument();
	return FAIL;
    }
}

/* Insert a number of lines into the specified buffer after the specifed line.
 * The line number is in Vim format (1-based). The lines to be inserted are
 * given as a Python list of string objects or as a single string. The lines
 * to be added are checked for validity and correct format. Errors are
 * returned as a value of FAIL.  The return value is OK on success.
 * If OK is returned and len_change is not NULL, *len_change
 * is set to the change in the buffer length.
 */
    static int
InsertBufferLines(buf_T *buf, int n, PyObject *lines, int *len_change)
{
    /* First of all, we check the type of the supplied Python object.
     * It must be a string or a list, or the call is in error.
     */
    if (PyString_Check(lines))
    {
	char	*str = StringToLine(lines);
	buf_T	*savebuf;

	if (str == NULL)
	    return FAIL;

	savebuf = curbuf;

	PyErr_Clear();
	curbuf = buf;

	if (u_save((linenr_T)n, (linenr_T)(n+1)) == FAIL)
	    PyErr_SetVim(_("cannot save undo information"));
	else if (ml_append((linenr_T)n, (char_u *)str, 0, FALSE) == FAIL)
	    PyErr_SetVim(_("cannot insert line"));
	else
	    appended_lines_mark((linenr_T)n, 1L);

	vim_free(str);
	curbuf = savebuf;
	update_screen(VALID);

	if (PyErr_Occurred() || VimErrorCheck())
	    return FAIL;

	if (len_change)
	    *len_change = 1;

	return OK;
    }
    else if (PyList_Check(lines))
    {
	int	i;
	int	size = PyList_Size(lines);
	char	**array;
	buf_T	*savebuf;

	array = (char **)alloc((unsigned)(size * sizeof(char *)));
	if (array == NULL)
	{
	    PyErr_NoMemory();
	    return FAIL;
	}

	for (i = 0; i < size; ++i)
	{
	    PyObject *line = PyList_GetItem(lines, i);
	    array[i] = StringToLine(line);

	    if (array[i] == NULL)
	    {
		while (i)
		    vim_free(array[--i]);
		vim_free(array);
		return FAIL;
	    }
	}

	savebuf = curbuf;

	PyErr_Clear();
	curbuf = buf;

	if (u_save((linenr_T)n, (linenr_T)(n + 1)) == FAIL)
	    PyErr_SetVim(_("cannot save undo information"));
	else
	{
	    for (i = 0; i < size; ++i)
	    {
		if (ml_append((linenr_T)(n + i),
					(char_u *)array[i], 0, FALSE) == FAIL)
		{
		    PyErr_SetVim(_("cannot insert line"));

		    /* Free the rest of the lines */
		    while (i < size)
			vim_free(array[i++]);

		    break;
		}
		vim_free(array[i]);
	    }
	    if (i > 0)
		appended_lines_mark((linenr_T)n, (long)i);
	}

	/* Free the array of lines. All of its contents have now
	 * been freed.
	 */
	vim_free(array);

	curbuf = savebuf;
	update_screen(VALID);

	if (PyErr_Occurred() || VimErrorCheck())
	    return FAIL;

	if (len_change)
	    *len_change = size;

	return OK;
    }
    else
    {
	PyErr_BadArgument();
	return FAIL;
    }
}

/* Convert a Vim line into a Python string.
 * All internal newlines are replaced by null characters.
 *
 * On errors, the Python exception data is set, and NULL is returned.
 */
    static PyObject *
LineToString(const char *str)
{
    PyObject *result;
    int len = strlen(str);
    char *p;

    /* Allocate an Python string object, with uninitialised contents. We
     * must do it this way, so that we can modify the string in place
     * later. See the Python source, Objects/stringobject.c for details.
     */
    result = PyString_FromStringAndSize(NULL, len);
    if (result == NULL)
	return NULL;

    p = PyString_AsString(result);

    while (*str)
    {
	if (*str == '\n')
	    *p = '\0';
	else
	    *p = *str;

	++p;
	++str;
    }

    return result;
}

/* Convert a Python string into a Vim line.
 *
 * The result is in allocated memory. All internal nulls are replaced by
 * newline characters. It is an error for the string to contain newline
 * characters.
 *
 * On errors, the Python exception data is set, and NULL is returned.
 */
    static char *
StringToLine(PyObject *obj)
{
    const char *str;
    char *save;
    int len;
    int i;

    if (obj == NULL || !PyString_Check(obj))
    {
	PyErr_BadArgument();
	return NULL;
    }

    str = PyString_AsString(obj);
    len = PyString_Size(obj);

    /* Error checking: String must not contain newlines, as we
     * are replacing a single line, and we must replace it with
     * a single line.
     */
    if (memchr(str, '\n', len))
    {
	PyErr_SetVim(_("string cannot contain newlines"));
	return NULL;
    }

    /* Create a copy of the string, with internal nulls replaced by
     * newline characters, as is the vim convention.
     */
    save = (char *)alloc((unsigned)(len+1));
    if (save == NULL)
    {
	PyErr_NoMemory();
	return NULL;
    }

    for (i = 0; i < len; ++i)
    {
	if (str[i] == '\0')
	    save[i] = '\n';
	else
	    save[i] = str[i];
    }

    save[i] = '\0';

    return save;
}

/* Check to see whether a Vim error has been reported, or a keyboard
 * interrupt has been detected.
 */
    static int
VimErrorCheck(void)
{
    if (got_int)
    {
	PyErr_SetNone(PyExc_KeyboardInterrupt);
	return 1;
    }
    else if (did_emsg && !PyErr_Occurred())
    {
	PyErr_SetNone(VimError);
	return 1;
    }

    return 0;
}


/* Don't generate a prototype for the next function, it generates an error on
 * newer Python versions. */
#if PYTHON_API_VERSION < 1007 /* Python 1.4 */ && !defined(PROTO)

    char *
Py_GetProgramName(void)
{
    return "vim";
}
#endif /* Python 1.4 */
