
/* tgvoip.cpp - Python wrapper for libtgvoip
  version 0.1 August 8th, 2018

  Copyright (C) 2018 Gabriel Jacobo https://mdqinc.com

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.
  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:
  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <Python.h>
#include <iostream>
#include <string>
#include <map>

#include "tgvoip/VoIPController.h"
#include "tgvoip/VoIPServerConfig.h"


/* Available functions */
static PyObject *tgvoip_call_start(PyObject *self, PyObject *args);
static PyObject *tgvoip_call_stop(PyObject *self, PyObject *args);

/* Module specification */
static PyMethodDef module_methods[] = {
    {"call_start", (PyCFunction) tgvoip_call_start, METH_VARARGS, "Establish a call"},
    {"call_stop", (PyCFunction) tgvoip_call_stop, METH_NOARGS, "Terminate a call"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef tgvoip_module =
{
    PyModuleDef_HEAD_INIT,
    "tgvoip", /* name of module */
    "libtgvoip wrapper for Python 3",
    -1,
    module_methods
};

/* Initialize the module */
PyMODINIT_FUNC PyInit_tgvoip(void)
{
    return PyModule_Create(&tgvoip_module);
}


/* Telegram Voip Calling Implementation */
using namespace std;
using namespace tgvoip;

static VoIPController::Config config {
    /*init_timeout*/30.0,
    /*recv_timeout*/30.0,
    /*data_saving*/DATA_SAVING_NEVER,
    /*enableAEC*/true,
    /*enableNS*/true,
    /*enableAGC*/true,
    /*enableCallUpgrade*/false,
};

static VoIPController *cnt = NULL;
static bool call_active = false;

static bool check_type(PyObject *o, const char *type)
{
    PyObject *t = PyDict_GetItemString(o, "@type");
    if (t == NULL) {
        return false;
    }

    t = PyObject_Str(t);
    bool ret = strcmp(PyUnicode_AsUTF8(t), type) == 0;
    Py_DECREF(t);
    return ret;
}

static PyObject *tgvoip_call_start(PyObject *self, PyObject *args)
{
    PyObject *call = NULL;
    if (!PyArg_UnpackTuple(args, "tgvoip_call", 1, 1, &call)) {
        return NULL;
    }

    if (!PyDict_Check(call)) {
        PyErr_SetString(PyExc_RuntimeError, "Parameter has to be a dict with call information");
        return NULL;
    }

    // Verify the "@type" property
    if (!check_type(call, "call")) {
        PyErr_SetString(PyExc_RuntimeError, "Parameter does not have @type == call");
        return NULL;
    }

    // Retrieve the state dict
    PyObject *state = PyDict_GetItemString(call, "state");
    if (state == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "No state entry found");
        return NULL;
    }

    if (!check_type(state, "callStateReady")) {
        PyErr_SetString(PyExc_RuntimeError, "state does not have @type == callStateReady");
        return NULL;
    }

    // Build the endpoints list
    PyObject *connections = PyDict_GetItemString(state, "connections");
    if( !PyList_Check(connections)) {
        PyErr_SetString(PyExc_RuntimeError, "Connections element is not a list");
        return NULL;
    }

    vector<Endpoint> endpoints;
    uint32_t num_connections = PyList_Size(connections);
    for(uint32_t i = 0; i < num_connections; i++) {
        PyObject *conn = PyList_GetItem(connections, i);

        if (!PyDict_Check(conn)) {
            continue;
        }

        if (!check_type(conn, "callConnection")) {
            continue;
        }


        PyObject *conn_type = PyDict_GetItemString(conn, "@type");
        if (conn_type == NULL) {
            continue;
        }

        conn_type = PyObject_Str(conn_type);
        if (strcmp(PyUnicode_AsUTF8(conn_type), "callConnection") != 0) {
            Py_DECREF(conn_type);
            continue;
        }
        Py_DECREF(conn_type);

        PyObject *conn_id = PyDict_GetItemString(conn, "id");
        if (conn_id == NULL) {
            continue;
        }
        conn_id = PyLong_FromUnicodeObject(conn_id, 10);
        int64_t id = (int64_t) PyLong_AsLongLong(conn_id);
        Py_DECREF(conn_id);

        PyObject *conn_port = PyDict_GetItemString(conn, "port");
        if (conn_port == NULL) {
            continue;
        }
        uint16_t port = (uint16_t) PyLong_AsLong(conn_port);

        PyObject *conn_ip = PyDict_GetItemString(conn, "ip");
        if (conn_ip == NULL) {
            continue;
        }
        conn_ip = PyObject_Str(conn_ip);
        IPv4Address address = IPv4Address(string(PyUnicode_AsUTF8(conn_ip)));
        Py_DECREF(conn_ip);

        PyObject *conn_ipv6 = PyDict_GetItemString(conn, "ipv6");
        if (conn_ipv6 == NULL) {
            continue;
        }
        conn_ipv6 = PyObject_Str(conn_ipv6);
        IPv6Address v6address = IPv6Address(string(PyUnicode_AsUTF8(conn_ipv6)));
        Py_DECREF(conn_ipv6);

        PyObject *peer_tag = PyDict_GetItemString(conn, "peer_tag");
        if (peer_tag == NULL) {
            continue;
        }

        endpoints.push_back(Endpoint(id, port, address, v6address, Endpoint::TYPE_UDP_RELAY, (unsigned char *) PyBytes_AsString(peer_tag)));
    }

    // Build the server config
    PyObject *state_config = PyDict_GetItemString(state, "config");
    if( !PyDict_Check(state_config)) {
        PyErr_SetString(PyExc_RuntimeError, "state[config] element is not a dict");
        return NULL;
    }
    map<string, string> server_conf;
    PyObject *key, *value;
    Py_ssize_t pos = 0;

    while (PyDict_Next(state_config, &pos, &key, &value)) {
        PyObject *k = PyObject_Str(key);
        PyObject *v = PyObject_Str(value);
        server_conf[string(PyUnicode_AsUTF8(k))] = string(PyUnicode_AsUTF8(v));
        Py_DECREF(k);
        Py_DECREF(v);
    }
    // Encryption key
    PyObject *encription_key = PyDict_GetItemString(state, "encryption_key");
    if (encription_key == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "No state[encryption_key] entry found");
        return NULL;
    }

    // Outgoing
    PyObject *is_outgoing = PyDict_GetItemString(call, "is_outgoing");
    if (is_outgoing == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "No call[is_outgoing] entry found");
        return NULL;
    }

    // Protocol
    PyObject *protocol = PyDict_GetItemString(state, "protocol");
    if( !PyDict_Check(protocol)) {
        PyErr_SetString(PyExc_RuntimeError, "No adequate state[protocol] entry found");
        return NULL;
    }

    PyObject *udp_p2p = PyDict_GetItemString(protocol, "udp_p2p");
    bool allow_p2p = false;
    if (udp_p2p != NULL) {
        allow_p2p = PyObject_IsTrue(udp_p2p);
    }

    PyObject *protocol_max_layer = PyDict_GetItemString(protocol, "max_layer");
    int32_t max_layer = 65;
    if (protocol_max_layer != NULL) {
        max_layer = PyLong_AsLong(protocol_max_layer);
    }

    ServerConfig::GetSharedInstance()->Update(server_conf);
    if (cnt == NULL) {
        cnt = new VoIPController();
    }
    cnt->SetConfig(config);
    cnt->SetEncryptionKey(PyBytes_AsString(encription_key), PyObject_IsTrue(is_outgoing));
    cnt->SetRemoteEndpoints(endpoints, allow_p2p, max_layer);
    cnt->Start();
    cnt->Connect();
    call_active = true;
    Py_INCREF(Py_True);
    return Py_True;
}


static PyObject *tgvoip_call_stop(PyObject *self, PyObject *args)
{
    if (!call_active) {
        Py_INCREF(Py_False);
        return Py_False;
    }
    cnt->Stop();
    // Deleting is not strictly required but it works around a bug when using the same UDP port multiple times
    delete cnt;
    cnt = NULL;
    call_active = false;
    Py_INCREF(Py_True);
    return Py_True;

}
